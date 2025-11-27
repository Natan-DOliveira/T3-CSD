#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "vga_drv.h"

// Inicializa a tela
static inline void init_display() {
    display_background(BLACK);
}

// Desenha um sprite genérico. Se color < 0, usa a cor definida na matriz do sprite.
// "static inline" evita erro de múltipla definição se incluído em vários lugares
static inline void draw_sprite(int x, int y, char *sprite, int w, int h, int color) {
    int i, j;
    if (sprite == NULL) { 
        display_frectangle(x, y, w, h, (color < 0) ? WHITE : color); 
        return;
    }

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            int pixel = sprite[i * w + j]; 
            if (pixel != 0) { 
                int final_color = (color < 0) ? pixel : color;
                display_pixel(x + j, y + i, final_color);
            }
        }
    }
}

#endif
