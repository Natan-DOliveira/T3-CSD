#ifndef OBJECTS_H
#define OBJECTS_H

struct object_s {
    char *sprite_frame[3];
    int num_frames;
    int cursprite;
    int w, h;
    int x, y;
    int dx, dy;             
    int speedx, speedy;     
    int speedxcnt, speedycnt; 
    int active;
    int type;               
    int points;
    int dying_timer;
};

#endif
