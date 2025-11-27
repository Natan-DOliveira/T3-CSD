/*
 * Arquivo: graphics.h
 * Descrição: Funções auxiliares para desenho de sprites sobre o driver VGA.
 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "vga_drv.h"

// Inicializa a tela com fundo preto
static inline void init_display() {
    display_background(BLACK);
}

// -------------------------------------------------------------------------
// Função: draw_sprite
// Descrição: Desenha uma matriz de bytes na tela (Sprite).
// Parâmetros:
//   x, y: Posição superior esquerda
//   sprite: Ponteiro para o array de pixels
//   w, h: Largura e Altura
//   color: Cor fixa para desenhar (se < 0, usa a cor definida na matriz do sprite)
// Nota: Pixels com valor 0 são considerados transparentes.
// -------------------------------------------------------------------------
static inline void draw_sprite(int x, int y, char *sprite, int w, int h, int color) {
    int i, j;
    // Se sprite nulo, desenha retângulo sólido (fallback/debug)
    if (sprite == NULL) { 
        display_frectangle(x, y, w, h, (color < 0) ? WHITE : color); 
        return;
    }

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            int pixel = sprite[i * w + j]; 
            if (pixel != 0) { // Ignora transparência (0)
                int final_color = (color < 0) ? pixel : color;
                display_pixel(x + j, y + i, final_color);
            }
        }
    }
}

#endif
