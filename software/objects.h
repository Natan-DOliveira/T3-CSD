#ifndef OBJECTS_H
#define OBJECTS_H

struct object_s {
    char *sprite_frame[3];  // Até 3 frames para animação (0 termina ciclo), como no exemplo
    int num_frames;         // Número de frames válidos
    int cursprite;          // Frame atual
    int w, h;               // Largura e altura
    int x, y;               // Posição atual
    int old_x, old_y;       // Posição anterior (para apagar apenas se mudou)
    int dx, dy;             // Direção de movimento
    int speedx, speedy;     // Velocidade (contadores para throttle)
    int speedxcnt, speedycnt;  // Contadores atuais
    int active;             // 1 = ativo, 0 = inativo
    int type;               // Tipo (aliens: 1-3, UFO:4, outros:0)
    int points;             // Pontos ao destruir
    int dying_timer;        // Timer para animação de morte
};

// Inicializa objeto com frames (NULL para frames não usados)
void init_object(struct object_s *obj, char *frame1, char *frame2, char *frame3, int w, int h, int x, int y, int dx, int dy, int spx, int spy, int type, int pts) {
    obj->sprite_frame[0] = frame1;
    obj->sprite_frame[1] = frame2;
    obj->sprite_frame[2] = frame3;
    obj->num_frames = (frame3 ? 3 : (frame2 ? 2 : 1));
    obj->cursprite = 0;
    obj->w = w;
    obj->h = h;
    obj->x = x;
    obj->y = y;
    obj->old_x = x;
    obj->old_y = y;
    obj->dx = dx;
    obj->dy = dy;
    obj->speedx = spx;
    obj->speedy = spy;
    obj->speedxcnt = spx;
    obj->speedycnt = spy;
    obj->active = 1;
    obj->type = type;
    obj->points = pts;
    obj->dying_timer = 0;
}

// Verifica colisão (bounding box, como no exemplo)
int check_coll(struct object_s *a, struct object_s *b) {
    if (!a->active || !b->active) return 0;
    return (a->x < b->x + b->w && a->x + a->w > b->x && a->y < b->y + b->h && a->y + a->h > b->y);
}

#endif
