#include <hf-risc.h>
#include "vga_drv.h"
#include "game.h"
#include "keyboard.h"

int main(void) {
    init_game();
    draw_game();

    int last_score = -1;

    while (1) {
        update_game();

        if (score != last_score) {
            char score_str[16];
            sprintf(score_str, "Score: %d", score);
            display_print(score_str, 10, 5, 1, WHITE);
            last_score = score;
        }

        int aliens_alive = 0;
        for(int i=0; i<NUM_ALIENS; i++) if(aliens[i].active) aliens_alive++;
        
        if (aliens_alive == 0) {
            display_print("YOU WIN", 120, 100, 1, GREEN);
            while(1);
        }

        for (int i = 0; i < 20; i++) {
            check_keyboard();
            delay_ms(1);     
        }
    }

    return 0;
}
