#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>

#define SIGN(x) (x < 0 ? -1 : 1)

typedef struct Fish {
    Vector2 pos, next_pos;

    int hunger;
    int tracking_food;

    bool ate_food;
} Fish;

typedef struct Node {
    struct Node *next;
} Node;

typedef struct Food {
    Vector2 pos;
    int id;
} Food;


void init_game();
void update();
void draw();
void place_food(Food food);
void destroy_food(int idx);
void update_tracking();
void update_next_pos();
void update_hunger();
void update_feeding();
void update_pos();

#endif
