#include "main.h"

#define WIDTH 800
#define HEIGHT 600
#define TITLE "Game"

#define RANDOM_TRACKING -1
#define MAX_FISH 10
#define FISH_SPEED 50.0f
#define FISH_RANGE 200.0f
static Fish FISHES[MAX_FISH];

#define MAX_FOOD 10
#define FOOD_SPEED 30.0f
static Food FOOD[MAX_FOOD];
static Node FOOD_POOL;

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
    // Pool allocator for food for some reason
    // Since sysetms will iterate over the pool regardless of the memory being allocated or not
    // we have to set a flag for them to check
    Node *head = &FOOD_POOL;
    for(int i = 0; i < MAX_FOOD; i++) {
        head->next = (Node *)(FOOD+i);
        head = head->next;
        FOOD[i].id = -1;
    }

    for(int i = 0; i < MAX_FISH; i++) {
        FISHES[i] = (Fish){
            (Vector2){(float)rand() / RAND_MAX * WIDTH, (float)rand() / RAND_MAX * HEIGHT}, 
            (Vector2){(float)rand() / RAND_MAX * WIDTH, (float)rand() / RAND_MAX * HEIGHT}, 
            0,
            RANDOM_TRACKING,
        };
    }
}

void update() 
{
    update_tracking();
    update_pos();
    update_hunger();

    if(IsMouseButtonPressed(0)) {
        place_food((Food){GetMousePosition(), 0});
    }
}


void update_hunger() 
{
    for(int i = 0; i < MAX_FISH; i++) {
        Fish *cur_fish = FISHES + i;
        cur_fish->hunger += 5;
    }
}

void update_feeding()
{
    for(int i = 0; i < MAX_FISH; i++) {
        Fish *cur_fish = FISHES + i;

        bool fish_tracking = cur_fish->tracking_food != RANDOM_TRACKING;
        bool food_not_null = FOOD[cur_fish->tracking_food].id != -1;
        if(fish_tracking && food_not_null) {
            bool food_in_range = Vector2Distance(cur_fish->next_pos, cur_fish->pos) < 0.1;
            if(food_in_range) {
                destroy_food(cur_fish->tracking_food);
                cur_fish->ate_food = true;
            }
        }
    }
}

void update_tracking()
{            
    for(int i = 0; i < MAX_FISH; i++) {
        Fish *cur_fish = FISHES + i;

        if(cur_fish->tracking_food == RANDOM_TRACKING) {
            int min_dist_idx = -1;
            float min_dist = 10000000.0f;

            for(int j = 0; j < MAX_FOOD; j++) {
                if(FOOD[j].id == -1) {
                    continue;
                }

                float dist = Vector2Distance(cur_fish->pos, FOOD[j].pos);
                if(dist < min_dist) {
                    min_dist_idx = j;
                    min_dist = dist;
                }
            }
        } else if(cur_fish->ate_food) {
            cur_fish->tracking_food = RANDOM_TRACKING;
        }
    }
}

void update_next_pos()
{
    for(int i = 0; i < MAX_FISH; i++) {
        Fish *cur_fish = FISHES + i;
        bool needs_new_random = false;

        if (cur_fish->tracking_food != RANDOM_TRACKING) {
            cur_fish->next_pos = FOOD[cur_fish->tracking_food].pos;
        }
        else if(cur_fish->ate_food) {
            needs_new_random = true;
        } else {
            bool reached_dest = Vector2Distance(cur_fish->pos, cur_fish->next_pos);
            if(reached_dest) {
                needs_new_random = true;
            }
        }

        if(needs_new_random) {
            cur_fish->next_pos.x = (rand() / RAND_MAX) * WIDTH;
            cur_fish->next_pos.y = (rand() / RAND_MAX) * HEIGHT;
        }
    }
}

void update_pos() 
{
    for(int i = 0; i < MAX_FISH; i++) {
        Fish *cur_fish = FISHES + i;

        float dx = SIGN(cur_fish->next_pos.x - cur_fish->pos.x) * FISH_SPEED * GetFrameTime();
        float dy = SIGN(cur_fish->next_pos.y - cur_fish->pos.y) * FISH_SPEED * GetFrameTime();

        cur_fish->pos.x += dx;
        cur_fish->pos.y += dy;
    }

    for(int i = 0; i < MAX_FOOD; i++) {
        Food *cur_food = FOOD + i;

        if(cur_food->id != -1) {
            cur_food->pos.y += FOOD_SPEED * GetFrameTime();
            if(cur_food->pos.y > HEIGHT) {
                destroy_food(i);
            }
        }
    }
}

void clear_events() 
{
    for(int i = 0; i < MAX_FISH; i++) {
        Fish *cur_fish = FISHES + i;

        cur_fish->ate_food = false;
    }
}

void draw() 
{
    ClearBackground(BLACK);
    BeginDrawing();

    char debug_text[100];
    sprintf(debug_text, "Next Point: (%f, %f)", FISHES[0].next_pos.x, FISHES[0].next_pos.y);
    DrawText(debug_text, 0, 0, 20, RAYWHITE);
    sprintf(debug_text, "Tracking: %s ", FISHES[0].tracking_food == RANDOM_TRACKING ? "random" : "food");
    DrawText(debug_text, 0, 22, 20, RAYWHITE);

//    for(int i = 0; i < MAX_FISH; i++) {
//        Fish *cur_fish = FISHES + i;
//        DrawCircleLines((int)cur_fish->pos.x, (int)cur_fish->pos.y, FISH_RANGE, BLUE);
//    }

    for(int i = 0; i < MAX_FISH; i++) {
        Fish *cur_fish = FISHES + i;
        DrawCircle((int)cur_fish->pos.x, (int)cur_fish->pos.y, cur_fish->hunger+10, RAYWHITE);
    }

    for(int i = 0; i < MAX_FOOD; i++) {
        Food *cur_food = FOOD + i;
        if(cur_food->id != -1) {
            DrawCircle(cur_food->pos.x, cur_food->pos.y, 10, RED);
        }
    }

    EndDrawing();
}

void place_food(Food food)
{
    if(FOOD_POOL.next) {
        Node *temp = FOOD_POOL.next->next;
        *(Food *)FOOD_POOL.next = food;
        FOOD_POOL.next = temp;
    }
}

void destroy_food(int idx) 
{
    FOOD[idx].id = -1;
    ((Node *)(FOOD + idx))->next = FOOD_POOL.next;
    FOOD_POOL.next = (Node *)(FOOD + idx);
}
