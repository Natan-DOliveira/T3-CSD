#include <hf-risc.h>
#include "vga_drv.h"
#include "game.h"
#include "keyboard.h"

// Função manual para converter int para string (substitui sprintf)
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
    // 1. Inicializa o vídeo apenas para mostrar o menu
    init_display(); 

    // --- TELA DE MENU INICIAL ---
    int blink_timer = 0;
    int show_text = 1;

    // Loop do Menu (Fica aqui até apertar espaço)
    while (1) {
        // Desenha o Título (Fixo)
        display_print("MENU INICIAL", 100, 80, 1, GREEN);

        // Lógica para piscar o texto de baixo
        blink_timer++;
        if (blink_timer > 300) { // Velocidade do pisca
            show_text = !show_text;
            blink_timer = 0;
            
            // Se for para esconder, desenha um retângulo preto em cima
            if (!show_text) {
                display_frectangle(40, 120, 240, 10, BLACK);
            }
        }

        // Desenha a instrução se for o momento visível
        if (show_text) {
            display_print("APERTE ESPACO PARA COMECAR", 45, 120, 1, WHITE);
        }

        // Verifica teclado
        check_keyboard();
        
        // Se apertou ESPAÇO, sai do menu e começa o jogo
        if (key_fire) {
            key_fire = 0; // Reseta para não dar um tiro logo no começo
            break;
        }

        delay_ms(1);
    }
    // -----------------------------

    // 2. Agora sim inicia o jogo real (Isso vai limpar a tela do menu)
    init_game();
    draw_game();

    int last_score = -1;

    while (1) {
        update_game();

        // Lógica de Atualização do Placar
        if (score != last_score) {
            char score_str[32];
            
            // Limpa a área do score anterior
            display_frectangle(10, 5, 100, 10, BLACK);

            // Monta a string
            char *prefix = "Score: ";
            int k = 0;
            while(prefix[k] != '\0') {
                score_str[k] = prefix[k];
                k++;
            }
            int_to_str_main(score, &score_str[k]);

            // Escreve o novo score
            display_print(score_str, 10, 5, 1, WHITE);
            
            last_score = score;
        }

        // Lógica de Vitória
        int aliens_alive = 0;
        for(int i=0; i<NUM_ALIENS; i++) {
            if(aliens[i].active) aliens_alive++;
        }
        
        if (aliens_alive == 0) {
            display_frectangle(110, 95, 80, 20, BLACK);
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
