#include <hf-risc.h>
#include "vga_drv.h"
#include "game.h"
#include "keyboard.h"
#include "graphics.h" 

// Função manual para converter int para string
void int_to_str_main(int value, char *str) {
    char temp[16];
    int i = 0;
    
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    while (value > 0) {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }
    int j = 0;
    while (i > 0) {
        str[j++] = temp[--i];
    }
    str[j] = '\0';
}

int main(void) {
    init_display(); 

    // ==========================================
    // CAMADA 1: MENU INICIAL (Só roda 1 vez)
    // ==========================================
    int blink_timer = 0;
    int show_text = 1;

    while (1) {
        display_print("SPACE INVADERS", 100, 80, 1, GREEN);

        blink_timer++;
        if (blink_timer > 300) { 
            show_text = !show_text;
            blink_timer = 0;
            if (!show_text) display_frectangle(40, 120, 240, 10, BLACK);
        }

        if (show_text) display_print("APERTE ESPACO SE VOCE AMA O MOLODOY. EH MOLODOY OU NAO EH?", 45, 120, 1, WHITE);

        check_keyboard();
        if (key_fire) {
            key_fire = 0; 
            break;
        }
        delay_ms(1);
    }

    // ==========================================
    // CAMADA 2: LOOP DE SESSÃO DO JOGO
    // (Daqui pra frente o jogo reinicia aqui)
    // ==========================================
    while (1) { 
        
        init_game(); // Reseta tudo (vidas, posições, score)
        draw_game();
        
        int last_score = -1;
        int restart_timer = 0; // Para o texto de reinício piscar

        // ==========================================
        // CAMADA 3: LOOP DO FRAME (Jogo rodando)
        // ==========================================
        while (1) {
            update_game();

            // 1. Verifica se o Player Morreu Definitivamente
            if (!player.active) {
                // Efeito pisca para o texto de reiniciar
                restart_timer++;
                if (restart_timer > 300) {
                     restart_timer = 0;
                     // Apaga e reescreve para piscar (opcional)
                     // display_frectangle(45, 120, 240, 10, BLACK); 
                }
                
                // Desenha a instrução
                display_print("APERTE ESPACO PARA REINICIAR", 45, 120, 1, WHITE);
                
                // Verifica se quer reiniciar
                check_keyboard(); // Importante ler o teclado aqui
                if (key_fire) {
                    key_fire = 0;
                    break; // SAI do Loop Camada 3 e volta pro Loop Camada 2 (init_game)
                }
                
                delay_ms(1);
                continue; // Pula o resto do frame
            }

            // 2. Atualiza Score
            if (score != last_score) {
                char score_str[32];
                display_frectangle(10, 5, 100, 10, BLACK);
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

            // 3. Verifica Vitória
            int aliens_alive = 0;
            for(int i=0; i<NUM_ALIENS; i++) {
                if(aliens[i].active) aliens_alive++;
            }
            if (aliens_alive == 0) {
                display_frectangle(110, 95, 80, 20, BLACK);
                display_print("YOU WIN", 120, 100, 1, GREEN);
                
                // Se ganhar, também trava e pede reinício (Opcional, mas boa prática)
                display_print("APERTE ESPACO PARA REINICIAR", 45, 120, 1, WHITE);
                check_keyboard();
                if (key_fire) {
                    key_fire = 0;
                    break;
                }
            }

            // 4. Delay e Input Normal
            for (int i = 0; i < 20; i++) {
                check_keyboard();
                delay_ms(1);     
            }
        } // Fim Loop Camada 3
    } // Fim Loop Camada 2

    return 0;
}
