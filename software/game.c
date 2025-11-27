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

// Desenha objeto com animação e tratamento de explosão
void draw_object(struct object_s *obj, int chg, int color) {
    if (obj->active) {
        char *spr;
        int ww = obj->w;
        int hh = obj->h;

        // Lógica de Explosão (Se dying_timer > 0, troca o sprite)
        if (obj->dying_timer > 0) {
            if (obj->type == 0) { // Player
                spr = (char *)explosion_player_spr;
                ww = 15; hh = 8;
            } else { // Aliens e UFO
                spr = (char *)explosion_inv_spr;
                ww = 13; hh = 7;
            }
            // Força cor de explosão se não for comando de apagar (BLACK)
            if (color != BLACK) color = (obj->type == 0) ? GREEN : RED;
        } else {
            // Desenho normal
            if (chg) {
                obj->cursprite = (obj->cursprite + 1) % obj->num_frames;
            }
            spr = obj->sprite_frame[obj->cursprite];
        }
        
        draw_sprite(obj->x, obj->y, spr, ww, hh, color);
    }
}

// Move objeto, gerencia explosão e redesenha
void move_object(struct object_s *obj) {
    if (!obj->active) return;

    // Se estiver explodindo, não move, apenas gerencia o timer
    if (obj->dying_timer > 0) {
        obj->dying_timer--;
        if (obj->dying_timer == 0) {
            // O timer acabou: hack para draw_object saber que tem que apagar a explosão
            obj->dying_timer = 1; 
            draw_object(obj, 0, BLACK); // Apaga o sprite de explosão
            obj->dying_timer = 0;
            obj->active = 0; // Desativa totalmente
        } else {
            // Ainda explodindo: redesenha para garantir visibilidade (efeito piscante opcional)
            draw_object(obj, 0, -1);
        }
        return; // Retorna cedo para não processar movimento X/Y
    }

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

    if (moved) {
        draw_object(&oldobj, 0, BLACK); // Apaga anterior
        if (obj->active) {
            draw_object(obj, 1, -1); // Desenha novo
        }
    }
}

// Detecção de colisão AABB
int detect_collision(struct object_s *obj1, struct object_s *obj2) {
    // Se estiver morrendo (dying), não colide mais
    if (!obj1->active || obj1->dying_timer > 0 || !obj2->active || obj2->dying_timer > 0) return 0;
    
    // Ajuste fino: para explosões, a caixa de colisão pode mudar, mas usamos a original aqui
    return (obj1->x < obj2->x + obj2->w &&
            obj1->x + obj1->w > obj2->x &&
            obj1->y < obj2->y + obj2->h &&
            obj1->y + obj1->h > obj2->y);
}

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
            // Correção de pontos: Type 3 (Top) = 30, Type 2 = 20, Type 1 = 10
            int pts = (type == 3) ? 30 : (type == 2) ? 20 : 10;
            init_object(&aliens[idx], fra, frb, NULL, w, 8, 20 + c * 40, 25 + r * 12, 0, 0, 10, 10, type, pts);
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
    // Se o player morreu completamente (acabou a animação), Fim de Jogo
    if (!player.active) {
         display_print("GAME OVER", 110, 100, 1, RED);
         while(1); 
    }

    // Player Input (Só move se não estiver explodindo)
    player.dx = 0;
    if (player.dying_timer == 0) {
        if (key_left) player.dx = -2;
        if (key_right) player.dx = 2;
    }
    move_object(&player); 

    // Tiro
    if (key_fire && !bullet.active && player.dying_timer == 0) {
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
            if (aliens[i].active && aliens[i].dying_timer == 0) {
                if ((alien_dx > 0 && aliens[i].x > SCREEN_W - 20) || 
                    (alien_dx < 0 && aliens[i].x < 5)) drop = 1;
            }
        }
        
        // Redesenho em bloco (apaga apenas quem está vivo e não explodindo)
        for (int i = 0; i < NUM_ALIENS; i++) {
            if (aliens[i].active && aliens[i].dying_timer == 0) 
                draw_object(&aliens[i], 0, BLACK);
        }

        if (drop) {
            alien_dx = -alien_dx;
            for (int i = 0; i < NUM_ALIENS; i++) aliens[i].y += 6;
        } else {
            for (int i = 0; i < NUM_ALIENS; i++) aliens[i].x += alien_dx;
        }
        
        // Desenha (apenas vivos e não explodindo, pois move_object cuida das explosões)
        for (int i = 0; i < NUM_ALIENS; i++) {
            if (aliens[i].active && aliens[i].dying_timer == 0) 
                draw_object(&aliens[i], 1, -1);
        }

        // Tiro Alien
        for (int i = 0; i < NUM_ALIENS; i++) {
            if (aliens[i].active && aliens[i].dying_timer == 0 && (simple_rand() % 100 < 2)) {
                for (int b = 0; b < MAX_BOMBS; b++) {
                    if (!bombs[b].active) {
                        init_object(&bombs[b], NULL, NULL, NULL, 3, 5, aliens[i].x + 4, aliens[i].y + 8, 0, 4, 1, 1, 6, 0);
                        break;
                    }
                }
            }
        }
    }

    // Bombas e Explosões de Aliens (move_object agora trata o timer)
    for (int i = 0; i < NUM_ALIENS; i++) {
        if (aliens[i].dying_timer > 0) move_object(&aliens[i]);
    }
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
                
                // INICIO DA ANIMAÇÃO DE EXPLOSÃO
                draw_object(&bullet, 0, BLACK); // Apaga bala
                bullet.active = 0;
                
                aliens[i].dying_timer = 5; // Configura timer (aprox 100ms)
                // Não desativa alien ainda, deixa move_object tratar
            }
        }
        if (ufo.active && detect_collision(&bullet, &ufo)) {
             score += ufo.points;
             
             draw_object(&bullet, 0, BLACK);
             bullet.active = 0;

             ufo.dying_timer = 10; // UFO explode por mais tempo
        }
        for (int i=0; i<NUM_SHIELDS; i++) {
             if (shields[i].active && detect_collision(&bullet, &shields[i])) {
                 draw_object(&bullet, 0, BLACK);
                 bullet.active = 0;
             }
        }
    }

    // Game Over e Bombas
    for (int b = 0; b < MAX_BOMBS; b++) {
        if (bombs[b].active) {
            if (detect_collision(&bombs[b], &player)) {
                // INICIA EXPLOSÃO DO PLAYER
                draw_object(&bombs[b], 0, BLACK);
                bombs[b].active = 0;
                
                player.dying_timer = 50; // Animação longa de morte
            }
            for (int s = 0; s < NUM_SHIELDS; s++) {
                if (shields[s].active && detect_collision(&bombs[b], &shields[s])) {
                    draw_object(&shields[s], 0, BLACK);
                    draw_object(&bombs[b], 0, BLACK);
                    shields[s].active = 0;
                    bombs[b].active = 0;
                }
            }
        }
    }
}
