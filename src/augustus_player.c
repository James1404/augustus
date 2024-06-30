#include "augustus_player.h"
#include "augustus_level.h"

#include <raylib.h>
#include <raymath.h>

static bool useCollisions = true;

Player Player_make(void) {
    return (Player) {
        .pos = (Vector2) {0},
        .size = (Vector2) { 1.0f, 2.0f },
        .state = PLAYER_STANDING,
        .has_collision = false,
    };
}

void Player_free(Player* player) {
}

#define WALK_SPEED 10.0f

void Player_update(Player* player) {
    Vector2 vel = {0};
    if(IsKeyDown(KEY_A)) vel.x -= 1;
    if(IsKeyDown(KEY_D)) vel.x += 1;

    if(IsKeyDown(KEY_W)) vel.y -= 1;
    if(IsKeyDown(KEY_S)) vel.y += 1;

    f32 speed = WALK_SPEED;

    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        speed = WALK_SPEED * 2;
    }

    if(IsKeyPressed(KEY_G)) Player_toggle_collisions();

    if(Player_is_grounded(*player)) {
    }

    vel = Vector2Scale(vel, GetFrameTime() * speed);

    player->has_collision = false;
    if(useCollisions) {
        player->pos = Vector2Add(player->pos, vel);
    }
    else {
        player->pos = Vector2Add(player->pos, vel);
    }
}

void Player_draw(Player* player) {
    Vector2 center = Vector2Scale(Vector2Add(player->pos, player->size), 0.5f);
    Vector2 half = Vector2Scale(player->size, 0.5f);
    DrawRectangleV(player->pos, player->size, BLUE);
}

bool Player_is_grounded(Player player) {
    return false;
}

void Player_toggle_collisions(void) {
    useCollisions = !useCollisions;
}
