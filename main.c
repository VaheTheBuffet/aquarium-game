#include "main.h"
#include <raymath.h>

#define WIDTH 800
#define HEIGHT 600
#define TITLE "Game"

static Fish FISHES[100];

int main() 
{
    InitWindow(WIDTH, HEIGHT, TITLE);

    init_game();

    while(!WindowShouldClose()) {
        update();
        draw();
    }

    CloseWindow();
    return 0;
}

void init_game() 
{
    for(int i = 0; i < 100; i++) {
        FISHES[i] = (Fish){
            (Vector2){(float)rand() / RAND_MAX * WIDTH, (float)rand() / RAND_MAX * HEIGHT}, 
            (Vector2){(float)rand() / RAND_MAX * WIDTH, (float)rand() / RAND_MAX * HEIGHT}, 
            0
        };
    }
}

void update() 
{
    static float lerp_rate = 20.0;

    for(int i = 0; i < 100; i++) {
        Fish *cur_fish = FISHES + i;
        //pick a new point for the fish to track
        if(Vector2Distance(cur_fish->pos, cur_fish->next_pos) < 1) {
            cur_fish->next_pos.x = ((float)rand() / RAND_MAX) * WIDTH;
            cur_fish->next_pos.y = ((float)rand() / RAND_MAX) * HEIGHT;
        }

        //linear interpolate to tracking point
        float dx = SIGN(cur_fish->next_pos.x - cur_fish->pos.x) * lerp_rate * GetFrameTime();
        float dy = SIGN(cur_fish->next_pos.y - cur_fish->pos.y) * lerp_rate * GetFrameTime();

        cur_fish->pos.x += dx;
        cur_fish->pos.y += dy;
    }
}

void draw() 
{
    ClearBackground(BLACK);
    BeginDrawing();

    char debug_text[100];
    sprintf(debug_text, "Next Point: (%f, %f)", FISHES[0].next_pos.x, FISHES[0].next_pos.y);
    DrawText(debug_text, 0, 0, 20, RAYWHITE);

    for(int i = 0; i < 100; i++) {
        Fish *cur_fish = FISHES + i;
        DrawCircle((int)cur_fish->pos.x, (int)cur_fish->pos.y, 10, RAYWHITE);
    }

    EndDrawing();
}
