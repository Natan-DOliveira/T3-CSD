#include <hf-risc.h>
#include "vga_drv.h"
#include "game.h"
#include "graphics.h"  
#include "sprites.h"
#include "keyboard.h" 

struct object_s player;
struct object_s aliens[NUM_ALIENS];
struct object_s bullet;
struct object_s bombs[MAX_BOMBS];
struct object_s shields[NUM_SHIELDS];
struct object_s ufo;

int score = 0;
int alien_dx = 3;
int move_timer = 0;
int move_threshold = 15;
unsigned int rng_seed = 123;

#define ROWS 4
#define COLS 6

int simple_rand() {
    rng_seed = rng_seed * 1103515245 + 12345;
    return (unsigned int)(rng_seed / 65536) % 32768;
}

// Lógica de inicialização
void init_object(struct object_s *obj, char *frame1, char *frame2, char *frame3, int w, int h, int x, int y, int dx, int dy, int spx, int spy, int type, int pts) {
    obj->sprite_frame[0] = frame1;
    obj->sprite_frame[1] = frame2;
    obj->sprite_frame[2] = frame3;
    obj->num_frames = (frame3 ? 3 : (frame2 ? 2 : 1));
    obj->cursprite = 0;
    obj->w = w; obj->h = h;
    obj->x = x; obj->y = y;
    obj->dx = dx; obj->dy = dy;
    obj->speedx = spx; obj->speedy = spy;
    obj->speedxcnt = spx; obj->speedycnt = spy;
    obj->active = 1;
    obj->type = type;
    obj->points = pts;
    obj->dying_timer = 0;
}

// Desenha objeto com animação
void draw_object(struct object_s *obj, int chg, int color) {
    if (obj->active) {
        if (chg) {
            obj->cursprite = (obj->cursprite + 1) % obj->num_frames;
        }
        char *spr = obj->sprite_frame[obj->cursprite];
        int ww = obj->w, hh = obj->h;

        if (obj->dying_timer > 0) {
            // Se estiver morrendo, usa cor sólida ou sprite de explosão se tiver
            color = (color == BLACK) ? BLACK : RED; 
        }
        
        draw_sprite(obj->x, obj->y, spr, ww, hh, color);
    }
}

// Move objeto, limpa rastro e redesenha
void move_object(struct object_s *obj) {
    if (!obj->active) return;

    struct object_s oldobj = *obj; 
    int moved = 0;

    if (--obj->speedxcnt <= 0) {
        obj->speedxcnt = obj->speedx;
        obj->x += obj->dx;
        moved = 1;
    }
    if (--obj->speedycnt <= 0) {
        obj->speedycnt = obj->speedy;
        obj->y += obj->dy;
        moved = 1;
    }

    // Limites de tela
    if (obj->type == 0) { // Player
        if (obj->x < 5) obj->x = 5;
        if (obj->x > 300 - 15) obj->x = 300 - 15;
    } else if (obj->type == 4) { // UFO
         if (obj->x > 300) obj->active = 0;
    } else if (obj->type == 5) { // Tiro
        if (obj->y < 0) obj->active = 0;
    } else if (obj->type == 6) { // Bomba
        if (obj->y > 218) obj->active = 0;
    }

    if (moved || obj->dying_timer > 0) {
        draw_object(&oldobj, 0, BLACK); // Apaga
        if (obj->active) {
            draw_object(obj, 1, -1); // Desenha novo (Multicolorido)
        }
    }
}

// Detecção de colisão AABB
int detect_collision(struct object_s *obj1, struct object_s *obj2) {
    if (!obj1->active || !obj2->active) return 0;
    return (obj1->x < obj2->x + obj2->w &&
            obj1->x + obj1->w > obj2->x &&
            obj1->y < obj2->y + obj2->h &&
            obj1->y + obj1->h > obj2->y);
}

// Função que desenha o estado inicial completo
void draw_game(void) {
    draw_object(&player, 0, -1);
    for (int i = 0; i < NUM_ALIENS; i++) draw_object(&aliens[i], 0, -1);
    for (int i = 0; i < NUM_SHIELDS; i++) draw_object(&shields[i], 0, GREEN);
    draw_object(&bullet, 0, WHITE);
    for (int i = 0; i < MAX_BOMBS; i++) draw_object(&bombs[i], 0, WHITE);
    draw_object(&ufo, 0, RED);
}

void init_game(void) {
    init_display(); 

    // Init Player
    init_object(&player, (char *)player_spr, NULL, NULL, 13, 8, SCREEN_W / 2, SCREEN_H - 15, 0, 0, 1, 1, 0, 0);

    // Init Aliens
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int idx = r * COLS + c;
            int type = (r == 0) ? 3 : (r < 2) ? 2 : 1;
            char *fra = (type == 3) ? (char *)alien3a_spr : (type == 2) ? (char *)alien2a_spr : (char *)alien1a_spr;
            char *frb = (type == 3) ? (char *)alien3b_spr : (type == 2) ? (char *)alien2b_spr : (char *)alien1b_spr;
            int w = (type == 3) ? 12 : (type == 2) ? 11 : 8;
            init_object(&aliens[idx], fra, frb, NULL, w, 8, 20 + c * 40, 25 + r * 12, 0, 0, 10, 10, type, (4 - type) * 10);
        }
    }

    // Init Escudos
    for (int i = 0; i < NUM_SHIELDS; i++) {
        init_object(&shields[i], (char *)shield_spr, NULL, NULL, 22, 16, 35 + i * 90, player.y - 30, 0, 0, 0, 0, 0, 0);
    }

    // Init Projéteis
    init_object(&bullet, NULL, NULL, NULL, 2, 4, 0, 0, 0, -5, 1, 1, 5, 0);
    bullet.active = 0;
    for (int i = 0; i < MAX_BOMBS; i++) {
        init_object(&bombs[i], NULL, NULL, NULL, 3, 5, 0, 0, 0, 4, 1, 1, 6, 0);
        bombs[i].active = 0;
    }

    init_object(&ufo, (char *)ufo_spr, NULL, NULL, 16, 7, -16, 10, 2, 0, 1, 1, 4, 100);
    ufo.active = 0;
    score = 0;
}

void update_game(void) {
    // Player Input
    player.dx = 0;
    if (key_left) player.dx = -2;
    if (key_right) player.dx = 2;
    move_object(&player); 

    // Tiro
    if (key_fire && !bullet.active) {
        init_object(&bullet, NULL, NULL, NULL, 2, 4, player.x + 6, player.y - 4, 0, -5, 1, 1, 5, 0);
        key_fire = 0; 
    }
    move_object(&bullet);

    // Aliens
    move_timer++;
    if (move_timer > move_threshold) {
        move_timer = 0;
        int drop = 0;
        for (int i = 0; i < NUM_ALIENS; i++) {
            if (aliens[i].active) {
                if ((alien_dx > 0 && aliens[i].x > SCREEN_W - 20) || 
                    (alien_dx < 0 && aliens[i].x < 5)) drop = 1;
            }
        }
        // Apaga todos antes de mover em bloco
        for (int i = 0; i < NUM_ALIENS; i++) if (aliens[i].active) draw_object(&aliens[i], 0, BLACK);

        if (drop) {
            alien_dx = -alien_dx;
            for (int i = 0; i < NUM_ALIENS; i++) aliens[i].y += 6;
        } else {
            for (int i = 0; i < NUM_ALIENS; i++) aliens[i].x += alien_dx;
        }
        
        // Desenha
        for (int i = 0; i < NUM_ALIENS; i++) if (aliens[i].active) draw_object(&aliens[i], 1, -1);

        // Tiro Alien
        for (int i = 0; i < NUM_ALIENS; i++) {
            if (aliens[i].active && (simple_rand() % 100 < 2)) {
                for (int b = 0; b < MAX_BOMBS; b++) {
                    if (!bombs[b].active) {
                        init_object(&bombs[b], NULL, NULL, NULL, 3, 5, aliens[i].x + 4, aliens[i].y + 8, 0, 4, 1, 1, 6, 0);
                        break;
                    }
                }
            }
        }
    }

    // Bombas
    for (int i = 0; i < MAX_BOMBS; i++) move_object(&bombs[i]);

    // UFO
    if (!ufo.active && (simple_rand() % 500 == 0)) {
        init_object(&ufo, (char *)ufo_spr, NULL, NULL, 16, 7, -16, 10, 2, 0, 1, 1, 4, 100);
    }
    move_object(&ufo);

    // Colisões
    if (bullet.active) {
        for (int i = 0; i < NUM_ALIENS; i++) {
            if (detect_collision(&bullet, &aliens[i])) { 
                score += aliens[i].points;
                bullet.active = 0;
                aliens[i].active = 0; 
                draw_object(&aliens[i], 0, BLACK);
                draw_object(&bullet, 0, BLACK);
            }
        }
        if (ufo.active && detect_collision(&bullet, &ufo)) {
             score += ufo.points;
             bullet.active = 0;
             ufo.active = 0;
             draw_object(&ufo, 0, BLACK);
             draw_object(&bullet, 0, BLACK);
        }
        for (int i=0; i<NUM_SHIELDS; i++) {
             if (shields[i].active && detect_collision(&bullet, &shields[i])) {
                 bullet.active = 0;
                 draw_object(&bullet, 0, BLACK);
             }
        }
    }

    // Game Over
    for (int b = 0; b < MAX_BOMBS; b++) {
        if (bombs[b].active) {
            if (detect_collision(&bombs[b], &player)) {
                display_print("GAME OVER", 110, 100, 1, RED);
                while(1); 
            }
            for (int s = 0; s < NUM_SHIELDS; s++) {
                if (shields[s].active && detect_collision(&bombs[b], &shields[s])) {
                    shields[s].active = 0;
                    bombs[b].active = 0;
                    draw_object(&shields[s], 0, BLACK);
                    draw_object(&bombs[b], 0, BLACK);
                }
            }
        }
    }
}
