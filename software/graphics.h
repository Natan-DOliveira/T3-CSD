#ifndef GRAPHICS_H
#define GRAPHICS_H

// Desenha pixels (suporte a NULL para retângulos, color <0 usa sprite multi-cor como no exemplo)
void draw_pixels(int x, int y, char *sprite, int w, int h, int color) {
    int i, j;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            int pixel = (sprite == NULL) ? 1 : sprite[i * w + j];
            if (pixel) {
                int pix_color = (color < 0) ? pixel : color;  // Usa valor do sprite se <0
                display_pixel(x + j, y + i, pix_color);
            }
        }
    }
}

// Desenha objeto (altera frame se chg=1, usa explosão se dying, como no exemplo adaptado)
void draw_object(struct object_s *obj, int chg, int color) {
    if (obj->active) {
        if (chg) {
            obj->cursprite = (obj->cursprite + 1) % obj->num_frames;  // Cicla frames
        }
        char *spr = obj->sprite_frame[obj->cursprite];
        int ww = obj->w, hh = obj->h;
        if (obj->dying_timer > 0) {
            spr = (obj->type == 0) ? (char *)explosion_player_spr : (char *)explosion_inv_spr;
            ww = (obj->type == 0) ? 15 : 13;
            hh = (obj->type == 0) ? 8 : 7;
        }
        draw_pixels(obj->x, obj->y, spr, ww, hh, color);
    }
}

#endif
