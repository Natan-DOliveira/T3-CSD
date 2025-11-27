/*
 * Arquivo: spaceinv.c
 * Feito por: Eduardo Pietro, Gabriel Woltmann e Natan Oliveira
 * Descrição: Loop Principal do Jogo.
 * Gerencia FSM: Menu -> Jogo -> Game Over/Win.
 */

#include <hf-risc.h>
#include "vga_drv.h"
#include "game.h"
#include "keyboard.h"
#include "graphics.h" 

// -------------------------------------------------------------------------
// Função Auxiliar: int_to_str_main
// Descrição: Converte um número inteiro para string manualmente.
// Motivo: Em ambientes bare-metal limitados, sprintf pode ser muito pesado.
// -------------------------------------------------------------------------
void int_to_str_main(int value, char *str) {
    char temp[16];
    int i = 0;
    
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    // Extrai os dígitos de trás para frente
    while (value > 0) {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }
    // Inverte para a ordem correta
    int j = 0;
    while (i > 0) {
        str[j++] = temp[--i];
    }
    str[j] = '\0';
}

int main(void) {
    init_display(); // Limpa a tela e define fundo preto

    // ==========================================
    // ESTADO 1: MENU INICIAL
    // Loop de espera até o jogador apertar ESPAÇO
    // ==========================================
    int blink_timer = 0;
    int show_text = 1;

    while (1) {
        // Título fixo
        display_print("SPACE INVADERS", 100, 80, 1, GREEN);

        // Lógica para piscar o texto "APERTE ESPACO"
        blink_timer++;
        if (blink_timer > 300) { 
            show_text = !show_text;
            blink_timer = 0;
            // Limpa a área do texto quando ele deve sumir
            if (!show_text) display_frectangle(40, 120, 240, 10, BLACK);
        }

        if (show_text) display_print("APERTE ESPACO PARA COMECAR", 45, 120, 1, WHITE);

        check_keyboard(); // Lê entrada do hardware
        if (key_fire) {
            key_fire = 0; // Consome o evento de tecla
            break;        // Sai do menu
        }
        delay_ms(1);
    }

    // ==========================================
    // ESTADO 2: LOOP DE SESSÃO
    // Reinicia as variáveis sempre que uma nova partida começa
    // ==========================================
    while (1) { 
        
        init_game(); // Reseta vidas, posições, score e aliens
        draw_game(); // Desenha o estado inicial
        
        int last_score = -1;
        int restart_timer = 0; 

        // ==========================================
        // ESTADO 3: LOOP DO FRAME (Gameplay)
        // O jogo roda continuamente aqui dentro
        // ==========================================
        while (1) {
            update_game(); // Atualiza física e IA

            // --- CASO 1: JOGADOR PERDEU ---
            if (!player.active) {
                restart_timer++;
                // Efeito visual opcional para piscar texto de reinício
                if (restart_timer > 300) {
                     restart_timer = 0;
                }
                
                display_print("APERTE ESPACO PARA REINICIAR", 45, 120, 1, WHITE);
                
                check_keyboard();
                if (key_fire) {
                    key_fire = 0;
                    break; // Sai do Loop de Frame -> Volta para Loop de Sessão (init_game)
                }
                
                delay_ms(1);
                continue; // Pula o resto da lógica de renderização se estiver morto
            }

            // --- CASO 2: ATUALIZAÇÃO DO PLACAR (HUD) ---
            // Só redesenha se o score mudou para economizar ciclos de CPU/VGA
            if (score != last_score) {
                char score_str[32];
                display_frectangle(10, 5, 100, 10, BLACK); // Limpa área do score
                
                // Constrói string "Score: " + valor
                char *prefix = "Score: ";
                int k = 0;
                while(prefix[k] != '\0') {
                    score_str[k] = prefix[k];
                    k++;
                }
                int_to_str_main(score, &score_str[k]);
                
                display_print(score_str, 10, 5, 1, WHITE);
                last_score = score;
            }

            // --- CASO 3: JOGADOR GANHOU (Vitória) ---
            int aliens_alive = 0;
            for(int i=0; i<NUM_ALIENS; i++) {
                if(aliens[i].active) aliens_alive++;
            }
            
            if (aliens_alive == 0) {
                display_frectangle(110, 95, 80, 20, BLACK);
                display_print("YOU WIN", 120, 100, 1, GREEN);
                
                display_print("APERTE ESPACO PARA REINICIAR", 45, 120, 1, WHITE);
                check_keyboard();
                if (key_fire) {
                    key_fire = 0;
                    break; // Reinicia o jogo
                }
            }

            // --- CONTROLE DE FPS (Timing) ---
            // Pequeno loop para reduzir a velocidade do jogo e ler inputs frequentemente
            for (int i = 0; i < 20; i++) {
                check_keyboard();
                delay_ms(1);     
            }
        } // Fim Loop Camada 3 (Frame)
    } // Fim Loop Camada 2 (Sessão)

    return 0;
}
