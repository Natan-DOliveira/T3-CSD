#ifndef GAME_H
#define GAME_H

#include "objects.h" 

// Constantes globais
#define SCREEN_W 300
#define SCREEN_H 218
#define NUM_ALIENS 24 
#define NUM_SHIELDS 3 
#define MAX_BOMBS 2

// Variáveis globais compartilhadas
extern int score;
extern struct object_s player;
extern struct object_s aliens[];
extern struct object_s bullet;
extern struct object_s bombs[];
extern struct object_s shields[];
extern struct object_s ufo;

// Protótipos das funções (Implementação vai no game.c)
void init_game(void);
void update_game(void);
void draw_game(void); // Nova função simplificada para o main

// Funções de física/lógica expostas
void init_object(struct object_s *obj, char *frame1, char *frame2, char *frame3, int w, int h, int x, int y, int dx, int dy, int spx, int spy, int type, int pts);
void move_object(struct object_s *obj);
void draw_object(struct object_s *obj, int chg, int color);
int detect_collision(struct object_s *obj1, struct object_s *obj2);

#endif
