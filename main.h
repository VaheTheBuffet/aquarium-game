#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <raylib.h>
#include <stdlib.h>

#define SIGN(x) (x < 0 ? -1 : 1)

void init_game();
void update();
void draw();

typedef struct Fish {
    Vector2 pos, next_pos;
    int hunger;
} Fish;

#endif
