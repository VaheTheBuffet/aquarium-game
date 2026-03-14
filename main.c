#include "main.h"

#define WIDTH 800
#define HEIGHT 600
#define MENU_PAD 100
#define TITLE "Game"

#define RANDOM_TRACKING -1
#define MAX_FISH 10
#define FISH_SPEED 50.0f
#define FISH_RANGE 200.0f
static Fish FISHES[MAX_FISH];
static Node FISH_POOL;

#define MAX_FOOD 10
#define FOOD_SPEED 30.0f
static Food FOOD[MAX_FOOD];
static Node FOOD_POOL;

static int EAT_EVENTS[MAX_FOOD];
static int EAT_ARENA;

static Texture PANEL;
Image PANEL_MASK;

int main() 
{
    InitWindow(WIDTH, HEIGHT, TITLE);
    PANEL = LoadTexture("./assets/panel-wide.png");
    PANEL_MASK = LoadImageFromTexture(PANEL);

    unsigned int *pixels = (unsigned int *)PANEL_MASK.data;
    for(int x = 0; x < PANEL.width; x++) {
        for(int y = 0; y < PANEL.height; y++) {
            if(((pixels[x + y * PANEL.width] >> 16) & 0xFF) >= 0x70) {
                pixels[x + y * PANEL.width] = 0xFF000000;
            }
        }
    }

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
    // Pool allocators for the entities
    // Since sysetms will iterate over the pool regardless of the memory being allocated or not
    // we have to set a flag for them to check
    Node *head = &FOOD_POOL;
    for(int i = 0; i < MAX_FOOD; i++) {
        head->next = (Node *)(FOOD+i);
        head = head->next;
        FOOD[i].id = -1;
    }

    head = &FISH_POOL;
    for(int i = 0; i < MAX_FISH; i++) {
        head->next = (Node *)(FISHES+i);
        head = head->next;
        FISHES[i].tracking_status = DEAD;
    }

    for(int i = 0; i < MAX_FISH; i++) {
        spawn_fish((Fish){
            (Vector2){(float)rand() / RAND_MAX * WIDTH, (float)rand() / RAND_MAX * HEIGHT}, 
            (Vector2){0.0f, 0.0f}, 
            10,
            RANDOM_TRACKING,
            FAILED,
        });
    }
}

void update() 
{
    update_feeding();
    update_hunger();
    update_tracking();
    update_next_pos();
    update_pos();
    clear_events();
    handle_input();
}

void handle_input()
{
    float scale_x = PANEL.width / WIDTH;
    float scale_y = PANEL.height / MENU_PAD;

    if(IsMouseButtonPressed(0)) {
        Vector2 pos = GetMousePosition();
        if(pos.y < MENU_PAD) {
            unsigned int * pixels = (unsigned int *)PANEL_MASK.data;
            int x = pos.x * scale_x;
            int y = pos.y * scale_y;
            if((pixels[x + y * PANEL_MASK.width] & 0xFFFFFF) == 0x000000) {
                printf("pressed\n");
            } 
        } 
        
        place_food((Food){pos, 0});
    }

    if(IsMouseButtonPressed(1)) {
        spawn_fish((Fish){
            GetMousePosition(), 
            (Vector2){0.0f, 0.0f},
            10,
            RANDOM_TRACKING,
            FAILED,
        });
    }
}

void update_hunger() 
{
    for(int i = 0; i < MAX_FISH; i++) {
        Fish *cur_fish = FISHES + i;
        if(cur_fish->tracking_status == DEAD) {
            continue;
        } else if(cur_fish->tracking_status & SUCCESS) {
            cur_fish->hunger += 5;
        } else if (cur_fish->hunger > 5){
            cur_fish->hunger -= 0.3f * GetFrameTime();
        } else {
            destroy_fish(i);
        }
    }
}

void update_feeding()
{
    for(int i = 0; i < MAX_FISH; i++) {
        if(FISHES[i].tracking_status == DEAD) {
            continue;
        }
        Fish *cur_fish = FISHES + i;

        bool fish_tracking = cur_fish->tracking_food != RANDOM_TRACKING;
        if(fish_tracking) {
            bool food_in_range = Vector2Distance(cur_fish->pos, FOOD[cur_fish->tracking_food].pos) < 0.1;
            bool food_null = FOOD[cur_fish->tracking_food].id == -1;

            if(food_in_range && !food_null) {
                destroy_food(cur_fish->tracking_food);
                cur_fish->tracking_status = SUCCESS;
            } else if(food_null) {
                cur_fish->tracking_status = FAILED;
            }
        }
    }
}

void update_tracking()
{            
    for(int i = 0; i < MAX_FISH; i++) {
        if(FISHES[i].tracking_status == DEAD) {
            continue;
        }
        Fish *cur_fish = FISHES + i;

        if(cur_fish->tracking_status & ~TRACKING) {
            int min_dist_idx = -1;
            float min_dist = FLT_MAX;

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

            if(min_dist_idx != -1 && min_dist < FISH_RANGE) {
                cur_fish->tracking_status = TRACKING;
                cur_fish->tracking_food = min_dist_idx;
            } else {
                cur_fish->tracking_status |= RANDOM;
                cur_fish->tracking_food = RANDOM_TRACKING;
            }
        }
    }
}

void update_next_pos()
{
    for(int i = 0; i < MAX_FISH; i++) {
        if(FISHES[i].tracking_status == DEAD) {
            continue;
        }
        Fish *cur_fish = FISHES + i;
        bool needs_new_random = false;

        if(cur_fish->tracking_status & TRACKING) {
            cur_fish->next_pos = FOOD[cur_fish->tracking_food].pos;
        } else if(cur_fish->tracking_status & (SUCCESS | FAILED)) {
            needs_new_random = true;
        } else {
            bool reached_dest = Vector2Distance(cur_fish->pos, cur_fish->next_pos) < 0.1;
            if(reached_dest) {
                needs_new_random = true;
            }
        }

        if(needs_new_random) {
            cur_fish->next_pos.x = ((float)rand() / RAND_MAX) * WIDTH;
            cur_fish->next_pos.y = ((float)rand() / RAND_MAX) * (HEIGHT - MENU_PAD) + MENU_PAD;
        }
    }
}

void update_pos() 
{
    float dt = GetFrameTime();
    for(int i = 0; i < MAX_FISH; i++) {
        if(FISHES[i].tracking_status == DEAD) {
            continue;
        }
        Fish *cur_fish = FISHES + i;

        float dx = SIGN(cur_fish->next_pos.x - cur_fish->pos.x) * FISH_SPEED * dt;
        float dy = SIGN(cur_fish->next_pos.y - cur_fish->pos.y) * FISH_SPEED * dt;

        cur_fish->pos.x += dx;
        cur_fish->pos.y += dy;
    }

    for(int i = 0; i < MAX_FOOD; i++) {
        Food *cur_food = FOOD + i;

        if(cur_food->id != -1) {
            cur_food->pos.y += FOOD_SPEED * dt;
            if(cur_food->pos.y > HEIGHT) {
                destroy_food(i);
            }
        }
    }
}

void clear_events() 
{
    for(int i = 0; i < MAX_FISH; i++) {
        if(FISHES[i].tracking_status == DEAD) {
            continue;
        }
        Fish *cur_fish = FISHES + i;

        cur_fish->tracking_status = RANDOM;
    }
}

void draw() 
{
    BeginDrawing();
    ClearBackground(DARKBLUE);

    draw_debug();
    draw_fish();
    draw_fish_range();
    draw_food();
    draw_menu();

    EndDrawing();
}

void draw_debug()
{
    char debug_text[100];
    sprintf(debug_text, "Next Point: (%f, %f)", FISHES[0].next_pos.x, FISHES[0].next_pos.y);
    DrawText(debug_text, 0, 0, 20, RAYWHITE);
    sprintf(debug_text, "Tracking: %s ", FISHES[0].tracking_food == RANDOM_TRACKING ? "random" : "food");
    DrawText(debug_text, 0, 22, 20, RAYWHITE);
}

void draw_food()
{
    for(int i = 0; i < MAX_FOOD; i++) {
        Food *cur_food = FOOD + i;
        if(cur_food->id != -1) {
            DrawCircle(cur_food->pos.x, cur_food->pos.y, 10, RED);
        }
    }
}

void draw_fish() 
{
    for(int i = 0; i < MAX_FISH; i++) {
        Fish *cur_fish = FISHES + i;
        if(cur_fish->tracking_status & DEAD) {
            continue;
        }
        DrawCircle((int)cur_fish->pos.x, (int)cur_fish->pos.y, cur_fish->hunger, RAYWHITE);
    }
}

void draw_fish_range()
{
    for(int i = 0; i < MAX_FISH; i++) {
        Fish *cur_fish = FISHES + i;
        if(cur_fish->tracking_status & DEAD) {
            continue;
        }
        DrawCircleLines((int)cur_fish->pos.x, (int)cur_fish->pos.y, FISH_RANGE, BLUE);
    }
}

void draw_menu()
{
    Rectangle source = {0, 0, PANEL.width, PANEL.height};
    Rectangle dest = {0, 0, WIDTH, MENU_PAD};
    Vector2 origin = {0, 0,};

    DrawTexturePro(PANEL, source, dest, origin, 0.0f, RAYWHITE);
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

void spawn_fish(Fish fish)
{
    if(FISH_POOL.next) {
        Node *temp = FISH_POOL.next->next;
        *(Fish *)FISH_POOL.next = fish;
        FISH_POOL.next = temp;
    }
}

void destroy_fish(int idx) 
{
    FISHES[idx].tracking_status = DEAD;
    ((Node *)(FISHES + idx))->next = FISH_POOL.next;
    FISH_POOL.next = (Node *)(FISHES + idx);
}

void add_eat_event(int event)
{
    EAT_EVENTS[EAT_ARENA++] = event;
}

void clear_eat_events()
{
    EAT_ARENA = 0;
}
