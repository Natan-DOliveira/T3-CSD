/*
 * Arquivo: game.c
 * Descrição: Implementação da lógica de jogo (física, colisões, IA).
 * Responsável por mover objetos e atualizar estados.
 */

#include <hf-risc.h>
#include "vga_drv.h"
#include "game.h"
#include "graphics.h"  
#include "sprites.h"
#include "keyboard.h" 

// Instânciação dos objetos globais
struct object_s player;
struct object_s aliens[NUM_ALIENS];
struct object_s bullet;         // Apenas 1 tiro do player por vez
struct object_s bombs[MAX_BOMBS]; // Tiros dos inimigos
struct object_s shields[NUM_SHIELDS];
struct object_s ufo;            // Nave mistério

// Variáveis de estado do jogo
int score = 0;
int alien_dx = 3;       // Velocidade horizontal dos aliens (positivo=direita, negativo=esquerda)
int move_timer = 0;     // Contador para controlar a velocidade da animação dos aliens
int move_threshold = 15; // Quanto menor, mais rápido os aliens se movem
unsigned int rng_seed = 123;

#define ROWS 4
#define COLS 6

// Gerador de números pseudo-aleatórios simples (Linear Congruential Generator)
int simple_rand() {
    rng_seed = rng_seed * 1103515245 + 12345;
    return (unsigned int)(rng_seed / 65536) % 32768;
}

// Inicializa um objeto genérico com suas propriedades
void init_object(struct object_s *obj, char *frame1, char *frame2, char *frame3, int w, int h, int x, int y, int dx, int dy, int spx, int spy, int type, int pts) {
    obj->sprite_frame[0] = frame1;
    obj->sprite_frame[1] = frame2;
    obj->sprite_frame[2] = frame3;
    obj->num_frames = (frame3 ? 3 : (frame2 ? 2 : 1)); // Determina quantos frames de animação existem
    obj->cursprite = 0;
    obj->w = w; obj->h = h;
    obj->x = x; obj->y = y;
    obj->dx = dx; obj->dy = dy;
    obj->speedx = spx; obj->speedy = spy; // Controles de velocidade (skip frames)
    obj->speedxcnt = spx; obj->speedycnt = spy;
    obj->active = 1;
    obj->type = type;
    obj->points = pts;
    obj->dying_timer = 0; // Se > 0, está tocando animação de explosão
}

// Renderiza o objeto na tela
void draw_object(struct object_s *obj, int chg, int color) {
    if (obj->active) {
        char *spr;
        int ww = obj->w;
        int hh = obj->h;

        // Verifica se está explodindo
        if (obj->dying_timer > 0) {
            if (obj->type == 0) { // Tipo 0 = Player
                spr = (char *)explosion_player_spr;
                ww = 15; hh = 8;
            } else { // Aliens e UFO
                spr = (char *)explosion_inv_spr;
                ww = 13; hh = 7;
            }
        } else {
            // Alterna frame de animação (ex: pernas do alien mexendo)
            if (chg) {
                obj->cursprite = (obj->cursprite + 1) % obj->num_frames;
            }
            spr = obj->sprite_frame[obj->cursprite];
        }
        
        // Chama a rotina gráfica de baixo nível
        draw_sprite(obj->x, obj->y, spr, ww, hh, color);
    }
}

// Atualiza a posição (Física)
void move_object(struct object_s *obj) {
    if (!obj->active) return;

    // Lógica de explosão: apenas conta o tempo e desenha a explosão, não move
    if (obj->dying_timer > 0) {
        obj->dying_timer--;
        if (obj->dying_timer == 0) {
            // Apaga a explosão ao final
            char *exp_spr = (obj->type == 0) ? (char *)explosion_player_spr : (char *)explosion_inv_spr;
            int ew = (obj->type == 0) ? 15 : 13;
            int eh = (obj->type == 0) ? 8 : 7;
            draw_sprite(obj->x, obj->y, exp_spr, ew, eh, BLACK);
            obj->active = 0; // Marca como morto definitivamente
        } else {
            draw_object(obj, 0, -1);
        }
        return; 
    }

    struct object_s oldobj = *obj; // Salva posição antiga para apagar rastro
    int moved = 0;

    // Controla velocidade X (move apenas a cada N ciclos)
    if (--obj->speedxcnt <= 0) {
        obj->speedxcnt = obj->speedx;
        obj->x += obj->dx;
        moved = 1;
    }
    // Controla velocidade Y
    if (--obj->speedycnt <= 0) {
        obj->speedycnt = obj->speedy;
        obj->y += obj->dy;
        moved = 1;
    }

    // Limites de tela (Bounding Box)
    if (obj->type == 0) { // Player (não sai da tela)
        if (obj->x < 5) obj->x = 5;
        if (obj->x > 300 - 15) obj->x = 300 - 15;
    } else if (obj->type == 4) { // UFO (desativa ao sair da tela)
         if (obj->x > 300) obj->active = 0;
    } else if (obj->type == 5) { // Tiro Player (desativa no topo)
        if (obj->y < 0) obj->active = 0;
    } else if (obj->type == 6) { // Bomba Inimiga (desativa no chão)
        if (obj->y > 218) obj->active = 0;
    }

    // Se moveu, apaga a posição antiga e desenha a nova
    if (moved) {
        draw_object(&oldobj, 0, BLACK); // Apaga
        if (obj->active) {
            draw_object(obj, 1, -1);    // Desenha novo
        }
    }
}

// Detecção de Colisão AABB (Axis-Aligned Bounding Box)
int detect_collision(struct object_s *obj1, struct object_s *obj2) {
    if (!obj1->active || obj1->dying_timer > 0 || !obj2->active || obj2->dying_timer > 0) return 0;
    
    // Verifica sobreposição de retângulos
    return (obj1->x < obj2->x + obj2->w &&
            obj1->x + obj1->w > obj2->x &&
            obj1->y < obj2->y + obj2->h &&
            obj1->y + obj1->h > obj2->y);
}

// Renderiza todos os elementos do jogo (chamado no início)
void draw_game(void) {
    draw_object(&player, 0, -1);
    for (int i = 0; i < NUM_ALIENS; i++) draw_object(&aliens[i], 0, -1);
    for (int i = 0; i < NUM_SHIELDS; i++) draw_object(&shields[i], 0, -1);
    draw_object(&bullet, 0, WHITE);
    for (int i = 0; i < MAX_BOMBS; i++) draw_object(&bombs[i], 0, WHITE);
    draw_object(&ufo, 0, RED);
}

// Configura o estado inicial de uma partida
void init_game(void) {
    init_display(); 

    // Inicializa Player no centro inferior
    init_object(&player, (char *)player_spr, NULL, NULL, 13, 8, SCREEN_W / 2, SCREEN_H - 15, 0, 0, 1, 1, 0, 0);

    // Inicializa grade de Aliens
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int idx = r * COLS + c;
            // Define tipos diferentes por linha (3=cima, 2=meio, 1=baixo)
            int type = (r == 0) ? 3 : (r < 2) ? 2 : 1;
            char *fra = (type == 3) ? (char *)alien3a_spr : (type == 2) ? (char *)alien2a_spr : (char *)alien1a_spr;
            char *frb = (type == 3) ? (char *)alien3b_spr : (type == 2) ? (char *)alien2b_spr : (char *)alien1b_spr;
            int w = (type == 3) ? 12 : (type == 2) ? 11 : 8;
            int pts = (type == 3) ? 30 : (type == 2) ? 20 : 10; // Pontuação conforme dificuldade
            init_object(&aliens[idx], fra, frb, NULL, w, 8, 20 + c * 40, 25 + r * 12, 0, 0, 10, 10, type, pts);
        }
    }

    // Inicializa Escudos
    for (int i = 0; i < NUM_SHIELDS; i++) {
        init_object(&shields[i], (char *)shield_spr, NULL, NULL, 22, 16, 35 + i * 90, player.y - 30, 0, 0, 0, 0, 0, 0);
    }

    // Inicializa Projéteis (inativos)
    init_object(&bullet, NULL, NULL, NULL, 2, 4, 0, 0, 0, -5, 1, 1, 5, 0);
    bullet.active = 0;
    for (int i = 0; i < MAX_BOMBS; i++) {
        init_object(&bombs[i], NULL, NULL, NULL, 3, 5, 0, 0, 0, 4, 1, 1, 6, 0);
        bombs[i].active = 0;
    }

    // Inicializa UFO (fora da tela)
    init_object(&ufo, (char *)ufo_spr, NULL, NULL, 16, 7, -16, 10, 2, 0, 1, 1, 4, 100);
    ufo.active = 0;
    score = 0;
}

// Loop principal de lógica (chamado a cada frame)
void update_game(void) {
    if (!player.active) {
         return; // Jogo parado se player morreu
    }

    // 1. Movimento do Player (Baseado nas flags do teclado)
    player.dx = 0;
    if (player.dying_timer == 0) {
        if (key_left) player.dx = -2;
        if (key_right) player.dx = 2;
    }
    move_object(&player); 

    // 2. Disparo do Player
    if (key_fire && !bullet.active && player.dying_timer == 0) {
        // Spawna o tiro na ponta da nave
        init_object(&bullet, NULL, NULL, NULL, 2, 4, player.x + 6, player.y - 4, 0, -5, 1, 1, 5, 0);
        key_fire = 0; // Evita metralhadora automática
    }
    move_object(&bullet);

    // 3. IA dos Aliens (Movimento em bloco)
    if (player.dying_timer == 0) {
        move_timer++;
        if (move_timer > move_threshold) { // Controla velocidade do enxame
            move_timer = 0;
            int drop = 0; // Flag para descer linha
            
            // Verifica se algum alien tocou na borda
            int active_count = 0;
            for (int i = 0; i < NUM_ALIENS; i++) {
                if (aliens[i].active && aliens[i].dying_timer == 0) {
                    active_count++;
                    if ((alien_dx > 0 && aliens[i].x > SCREEN_W - 20) || 
                        (alien_dx < 0 && aliens[i].x < 5)) drop = 1;
                }
            }
            
            // Redesenha aliens (apaga posição atual)
            for (int i = 0; i < NUM_ALIENS; i++) {
                if (aliens[i].active && aliens[i].dying_timer == 0) 
                    draw_object(&aliens[i], 0, BLACK);
            }

            // Aplica movimento (Descer ou Lado)
            if (drop) {
                alien_dx = -alien_dx; // Inverte direção
                for (int i = 0; i < NUM_ALIENS; i++) {
                    if (aliens[i].active) aliens[i].y += 6; // Desce
                }
            } else {
                for (int i = 0; i < NUM_ALIENS; i++) {
                    if (aliens[i].active && aliens[i].dying_timer == 0)
                        aliens[i].x += alien_dx; // Move lado
                }
            }
            
            // Lógica de Tiro dos Inimigos (IA Agressiva)
            int fire_chance = 2; 
            if (active_count < 10) fire_chance = 8;  // Acelera tiros no fim
            if (active_count < 4)  fire_chance = 18; 

            for (int i = 0; i < NUM_ALIENS; i++) {
                if (aliens[i].active && aliens[i].dying_timer == 0) {
                    draw_object(&aliens[i], 1, -1); // Desenha nova posição
                    
                    // Tenta atirar aleatoriamente
                    if (simple_rand() % 100 < fire_chance) {
                        for (int b = 0; b < MAX_BOMBS; b++) {
                            if (!bombs[b].active) { // Procura slot de bomba livre
                                init_object(&bombs[b], NULL, NULL, NULL, 3, 5, aliens[i].x + 4, aliens[i].y + 8, 0, 4, 1, 1, 6, 0);
                                break;
                            }
                        }
                    }
                }
            }
        }
    } 

    // 4. Processa Animações de Explosão
    for (int i = 0; i < NUM_ALIENS; i++) {
        if (aliens[i].dying_timer > 0) move_object(&aliens[i]);
    }
    for (int i = 0; i < MAX_BOMBS; i++) move_object(&bombs[i]);

    // 5. UFO (Bônus)
    if (player.dying_timer == 0) {
        if (!ufo.active && (simple_rand() % 500 == 0)) {
            init_object(&ufo, (char *)ufo_spr, NULL, NULL, 16, 7, -16, 10, 2, 0, 1, 1, 4, 100);
        }
        move_object(&ufo);
    }

    // 6. Verificação de Colisões
    
    // Tiro do Player vs Inimigos/UFO/Escudos
    if (bullet.active) {
        for (int i = 0; i < NUM_ALIENS; i++) {
            if (detect_collision(&bullet, &aliens[i])) { 
                score += aliens[i].points;
                draw_object(&bullet, 0, BLACK); // Apaga tiro
                bullet.active = 0;
                draw_object(&aliens[i], 0, BLACK); // Apaga alien
                aliens[i].dying_timer = 5;         // Inicia explosão
            }
        }
        if (ufo.active && detect_collision(&bullet, &ufo)) {
             score += ufo.points;
             draw_object(&bullet, 0, BLACK);
             bullet.active = 0;
             draw_object(&ufo, 0, BLACK);
             ufo.dying_timer = 10; 
        }
        // Tiro do Player vs Escudo (destrói escudo também)
        for (int i=0; i<NUM_SHIELDS; i++) {
             if (shields[i].active && detect_collision(&bullet, &shields[i])) {
                 draw_object(&bullet, 0, BLACK);
                 bullet.active = 0;
             }
        }
    }

    // Bombas Inimigas vs Player e Escudos
    for (int b = 0; b < MAX_BOMBS; b++) {
        if (bombs[b].active) {
            // Bomba vs Player
            if (detect_collision(&bombs[b], &player)) {
                draw_object(&bombs[b], 0, BLACK);
                bombs[b].active = 0;
                draw_object(&player, 0, BLACK);
                player.dying_timer = 50; // Tempo da animação de morte longa
                
                // GAME OVER
                display_frectangle(110, 95, 80, 20, BLACK); 
                display_print("GAME OVER", 110, 100, 1, RED); 
            }
            // Bomba vs Escudo
            for (int s = 0; s < NUM_SHIELDS; s++) {
                if (shields[s].active && detect_collision(&bombs[b], &shields[s])) {
                    draw_object(&shields[s], 0, BLACK); // Destrói escudo
                    draw_object(&bombs[b], 0, BLACK);   // Destrói bomba
                    shields[s].active = 0;
                    bombs[b].active = 0;
                }
            }
        }
    }
}
