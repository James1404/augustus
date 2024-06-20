#include "augustus_player.h"
#include "raylib.h"

Player Player_make(void) {
    return (Player) {
        .x = 0,
        .y = 0,
        .state = PLAYER_STANDING
    };
}

void Player_free(Player* player) {
}

#define WALK_SPEED 0.5f

void Player_update(Player* player) {
    f32 vx = 0, vy = 0;
    if(IsKeyDown(KEY_A)) vx -= 1;
    if(IsKeyDown(KEY_D)) vx += 1;

    if(IsKeyDown(KEY_W)) vy -= 1;
    if(IsKeyDown(KEY_S)) vy += 1;

    f32 speed = WALK_SPEED;

    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        speed = 2.0f;
    }

    player->x += vx * GetFrameTime() * speed;
    player->y += vy * GetFrameTime() * speed;
}

void Player_draw(Player* player) {
    DrawRectangleRec((Rectangle) { player->x, player->y, 1, 2}, RED);
}
