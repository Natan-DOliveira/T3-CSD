#include <hf-risc.h>
#include "vga_drv.h"
#include "sprites.h"
#include "objects.h"
#include "graphics.h"
#include "keyboard.h"

// Constantes
#define BLACK   0x0
#define GREEN   0x2
#define RED     0x4
#define WHITE   0xF
#define CYAN    0x3
#define YELLOW  0xE

#define SCREEN_W 300
#define SCREEN_H 218

#define ROWS 4
#define COLS 6
#define NUM_ALIENS (ROWS * COLS)
#define NUM_SHIELDS 3 
#define MAX_BOMBS 2

// Globais
int key_left = 0, key_right = 0, key_fire = 0, key_break = 0;
unsigned int rng_seed = 123;
int score = 0;

// RNG
int simple_rand() {
    rng_seed = rng_seed * 1103515245 + 12345;
    return (unsigned int)(rng_seed / 65536) % 32768;
}

int main(void) {
    struct object_s aliens[NUM_ALIENS];
    struct object_s player;
    struct object_s bullet;
    struct object_s shield[NUM_SHIELDS];
    struct object_s bombs[MAX_BOMBS];
    struct object_s ufo;

    display_background(BLACK);

    // Init player (sem frames extras)
    init_object(&player, (char *)player_spr, NULL, NULL, 13, 8, SCREEN_W / 2, SCREEN_H - 15, 0, 0, 1, 1, 0, 0);
    draw_object(&player, 0, GREEN);

    // Init aliens com frames A/B
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int idx = r * COLS + c;
            int type = (r == 0) ? 3 : (r < 2) ? 2 : 1;
            char *fra = (type == 3) ? (char *)alien3a_spr : (type == 2) ? (char *)alien2a_spr : (char *)alien1a_spr;
            char *frb = (type == 3) ? (char *)alien3b_spr : (type == 2) ? (char *)alien2b_spr : (char *)alien1b_spr;
            int w = (type == 3) ? 12 : (type == 2) ? 11 : 8;
            init_object(&aliens[idx], fra, frb, NULL, w, 8, 20 + c * 40, 25 + r * 12, 3, 0, 10, 10, type, (4 - type) * 10);
            int col = (type == 1) ? CYAN : (type == 2) ? YELLOW : RED;
            draw_object(&aliens[idx], 0, col);
        }
    }

    for (int i = 0; i < NUM_SHIELDS; i++) {
        init_object(&shield[i], (char *)shield_spr, NULL, NULL, 22, 16, 35 + i * 90, player.y - 30, 0, 0, 0, 0, 0, 0);
        draw_object(&shield[i], 0, GREEN);
    }

    // Init projéteis (retângulos, sem sprite)
    init_object(&bullet, NULL, NULL, NULL, 2, 4, 0, 0, 0, -5, 1, 1, 0, 0);
    bullet.active = 0;
    for (int i = 0; i < MAX_BOMBS; i++) {
        init_object(&bombs[i], NULL, NULL, NULL, 3, 5, 0, 0, 0, 4, 1, 1, 0, 0);
        bombs[i].active = 0;
    }

    // Init UFO (aparece random)
    init_object(&ufo, (char *)ufo_spr, NULL, NULL, 16, 7, -16, 10, 2, 0, 1, 1, 4, 100);  // Type 4, 100 pts
    ufo.active = 0;

    int alien_dx = 3;  // Direção grupo aliens
    int move_timer = 0;
    int move_threshold = 10;
    int ufo_timer = simple_rand() % 200 + 100;  // Aparece após 5-10s

    while (1) {
        check_keyboard();

        if (key_left) display_print("LEFT", 10, 20, 1, WHITE);
        if (key_right) display_print("RIGHT", 10, 30, 1, WHITE);
        if (key_fire) display_print("FIRE", 10, 40, 1, WHITE);

        // Lógica player (move como no exemplo: condicional)
        int old_player_x = player.x;
        if (key_left && player.x > 5) player.x -= 2;
        if (key_right && player.x < SCREEN_W - 15) player.x += 2;
        if (old_player_x != player.x) {
            draw_pixels(old_player_x, player.y, player.sprite_frame[0], player.w, player.h, BLACK);  // Apaga velho
            draw_object(&player, 0, GREEN);  // Desenha novo
        }

        // Ativa tiro
        if (key_fire && !bullet.active) {
            bullet.active = 1;
            bullet.x = player.x + 6;
            bullet.y = player.y - 4;
            draw_pixels(bullet.x, bullet.y, NULL, bullet.w, bullet.h, WHITE);
            key_fire = 0;
        }

        // Move bala (como move_object adaptado)
        if (bullet.active) {
            draw_pixels(bullet.x, bullet.y, NULL, bullet.w, bullet.h, BLACK);
            bullet.y += bullet.dy;
            if (bullet.y < 0) bullet.active = 0;
            else {
                // Colisões bala
                for (int i = 0; i < NUM_ALIENS; i++) {
                    if (aliens[i].active && aliens[i].dying_timer == 0 && check_coll(&bullet, &aliens[i])) {
                        score += aliens[i].points;
                        bullet.active = 0;
                        aliens[i].dying_timer = 5;
                        draw_object(&aliens[i], 0, BLACK);
                        draw_object(&aliens[i], 0, WHITE);  // Explosão
                        break;
                    }
                }
                if (ufo.active && check_coll(&bullet, &ufo)) {
                    score += ufo.points;
                    bullet.active = 0;
                    ufo.dying_timer = 5;
                    draw_object(&ufo, 0, BLACK);
                    draw_object(&ufo, 0, WHITE);
                }
                for (int i = 0; i < NUM_SHIELDS; i++) {
                    if (shield[i].active && check_coll(&bullet, &shield[i])) bullet.active = 0;
                }
                if (bullet.active) draw_pixels(bullet.x, bullet.y, NULL, bullet.w, bullet.h, WHITE);
            }
        }

        // Animação morte aliens/UFO/player
        int alive_count = 0;
        for (int i = 0; i < NUM_ALIENS; i++) {
            if (aliens[i].active) {
                alive_count++;
                if (aliens[i].dying_timer > 0) {
                    aliens[i].dying_timer--;
                    if (aliens[i].dying_timer == 0) {
                        draw_pixels(aliens[i].x, aliens[i].y, (char *)explosion_inv_spr, 13, 7, BLACK);
                        aliens[i].active = 0;
                    }
                }
            }
        }
        if (ufo.active && ufo.dying_timer > 0) {
            ufo.dying_timer--;
            if (ufo.dying_timer == 0) {
                draw_pixels(ufo.x, ufo.y, (char *)explosion_inv_spr, 13, 7, BLACK);
                ufo.active = 0;
            }
        }

        // Lógica aliens (move como no exemplo)
        move_timer++;
        if (move_timer > move_threshold) {
            move_timer = 0;
            int drop = 0;
            for (int i = 0; i < NUM_ALIENS; i++) {
                if (aliens[i].active && ((alien_dx > 0 && aliens[i].x > SCREEN_W - 15) || (alien_dx < 0 && aliens[i].x < 5))) drop = 1;
            }
            for (int i = 0; i < NUM_ALIENS; i++) {
                if (aliens[i].active) draw_object(&aliens[i], 0, BLACK);  // Apaga antes
            }
            if (drop) {
                alien_dx = -alien_dx;
                for (int i = 0; i < NUM_ALIENS; i++) aliens[i].y += 6;
            } else {
                for (int i = 0; i < NUM_ALIENS; i++) aliens[i].x += alien_dx;
            }
            for (int i = 0; i < NUM_ALIENS; i++) {
                if (aliens[i].active) {
                    int col = (aliens[i].type == 1) ? CYAN : (aliens[i].type == 2) ? YELLOW : RED;
                    draw_object(&aliens[i], 1, col);  // Desenha com animação
                }
            }

            // Aliens atiram
            for (int i = 0; i < NUM_ALIENS; i++) {
                if (aliens[i].active && (simple_rand() % 100 < 2)) {
                    for (int b = 0; b < MAX_BOMBS; b++) {
                        if (!bombs[b].active) {
                            bombs[b].active = 1;
                            bombs[b].x = aliens[i].x + 4;
                            bombs[b].y = aliens[i].y + 8;
                            draw_pixels(bombs[b].x, bombs[b].y, NULL, bombs[b].w, bombs[b].h, WHITE);
                            break;
                        }
                    }
                }
            }
        }

        // Lógica bombas
        for (int b = 0; b < MAX_BOMBS; b++) {
            if (bombs[b].active) {
                draw_pixels(bombs[b].x, bombs[b].y, NULL, bombs[b].w, bombs[b].h, BLACK);
                bombs[b].y += bombs[b].dy;
                if (bombs[b].y > SCREEN_H) bombs[b].active = 0;
                else {
                    if (check_coll(&bombs[b], &player)) {
                        player.dying_timer = 5;
                        draw_object(&player, 0, BLACK);
                        draw_object(&player, 0, RED);  // Explosão player
                        display_print("GAME OVER", 100, 100, 1, RED);
                        while (1);
                    }
                    for (int k = 0; k < NUM_SHIELDS; k++) {
                        if (shield[k].active && check_coll(&bombs[b], &shield[k])) {
                            shield[k].active = 0;
                            draw_object(&shield[k], 0, BLACK);
                            bombs[b].active = 0;
                            break;
                        }
                    }
                    if (bombs[b].active) draw_pixels(bombs[b].x, bombs[b].y, NULL, bombs[b].w, bombs[b].h, WHITE);
                }
            }
        }

        // Lógica UFO
        ufo_timer--;
        if (ufo_timer <= 0 && !ufo.active) {
            ufo.active = 1;
            ufo.x = -16;  // Começa fora esquerda
            ufo_timer = simple_rand() % 200 + 100;
            draw_object(&ufo, 0, RED);
        }
        if (ufo.active && ufo.dying_timer == 0) {
            draw_object(&ufo, 0, BLACK);
            ufo.x += ufo.dx;
            if (ufo.x > SCREEN_W) ufo.active = 0;
            else draw_object(&ufo, 1, RED);
        }

        // Pontuação
        char score_str[16];
        sprintf(score_str, "Score: %d", score);
        display_print(score_str, 10, 10, 1, WHITE);

        // Vitória
        if (alive_count == 0) {
            display_print("YOU WIN", 120, 100, 1, GREEN);
            while (1);
        }

        delay_ms(50);  // Delay aumentado para jogo mais lento
    }
    return 0;
}
