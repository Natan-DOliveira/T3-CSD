#include <hf-risc.h>
#include "vga_drv.h"
#include "game.h"
#include "keyboard.h"

// Função manual para converter int para string (substitui sprintf)
// Necessário pois stdio.h não está disponível
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
    init_game();
    draw_game();

    int last_score = -1;

    while (1) {
        update_game();

        // Lógica de Atualização do Placar
        if (score != last_score) {
            char score_str[32];
            
            // 1. LIMPEZA: Escreve "Score:       " em PRETO para apagar o número anterior
            // Isso evita que o novo número seja escrito por cima do velho
            display_print("Score:        ", 10, 5, 1, BLACK);

            // 2. FORMATAÇÃO MANUAL: Constrói a string "Score: <numero>"
            // Copia o prefixo manualmente
            char *prefix = "Score: ";
            int k = 0;
            while(prefix[k] != '\0') {
                score_str[k] = prefix[k];
                k++;
            }
            
            // Converte o número e anexa
            int_to_str_main(score, &score_str[k]);

            // 3. DESENHO: Escreve o novo score em BRANCO
            display_print(score_str, 10, 5, 1, WHITE);
            
            last_score = score;
        }

        // Lógica de Vitória
        int aliens_alive = 0;
        for(int i=0; i<NUM_ALIENS; i++) {
            if(aliens[i].active) aliens_alive++;
        }
        
        if (aliens_alive == 0) {
            display_print("YOU WIN", 120, 100, 1, GREEN);
            while(1);
        }

        // Delay e leitura de teclado
        for (int i = 0; i < 20; i++) {
            check_keyboard();
            delay_ms(1);     
        }
    }

    return 0;
}
