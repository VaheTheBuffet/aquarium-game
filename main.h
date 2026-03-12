#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>
#include <float.h>

#define SIGN(x) (x < 0 ? -1 : 1)

typedef enum {
    RANDOM = 0x1,
    TRACKING = 0x1 << 1,
    SUCCESS = 0x1 << 2,
    FAILED = 0x1 << 3,
    DEAD = 0x01 << 4,
} TrackingStatus;

typedef struct Fish {
    Vector2 pos, next_pos;

    float hunger;
    int tracking_food;

    TrackingStatus tracking_status;
} Fish;

typedef struct Node {
    struct Node *next;
} Node;

typedef struct Food {
    Vector2 pos;
    int id;
} Food;


void init_game();

void update_tracking();
void update_next_pos();
void update_hunger();
void update_feeding();
void update_pos();
void update();

void draw_debug();
void draw_food();
void draw_fish();
void draw_fish_range();

void place_food(Food food);
void destroy_food(int idx);
void clear_events();
void spawn_fish(Fish fish);
void destroy_fish(int idx);

#endif
