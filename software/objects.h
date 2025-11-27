/*
 * Arquivo: objects.h
 * Descrição: Definição da estrutura base para todos os objetos do jogo.
 */

#ifndef OBJECTS_H
#define OBJECTS_H

struct object_s {
    char *sprite_frame[3];  // Ponteiros para os frames de animação
    int num_frames;         // Quantidade total de frames
    int cursprite;          // Frame atual sendo exibido
    int w, h;               // Dimensões
    int x, y;               // Posição na tela
    int dx, dy;             // Vetores de movimento (direção)
    int speedx, speedy;     // Divisores de velocidade (para slow motion)
    int speedxcnt, speedycnt; // Contadores internos de velocidade
    int active;             // 1 = Vivo/Visível, 0 = Inativo/Morto
    int type;               // Identificador do tipo (Player, Alien, Tiro, etc)
    int points;             // Pontos que vale ao ser destruído
    int dying_timer;        // Timer para controlar duração da animação de explosão
};

#endif
