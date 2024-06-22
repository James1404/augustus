#include "augustus_player.h"
#include "augustus_level.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <math.h>

static bool useCollisions = true;

Player Player_make(void) {
    return (Player) {
        .pos = (Vector2) {0},
        .height = 2,
        .state = PLAYER_STANDING
    };
}

void Player_free(Player* player) {
}

#define WALK_SPEED 0.5f
#define HALF_WIDTH 0.5f

void Player_update(Player* player) {
    Vector2 vel = {0};
    if(IsKeyDown(KEY_A)) vel.x -= 1;
    if(IsKeyDown(KEY_D)) vel.x += 1;

    if(IsKeyDown(KEY_W)) vel.y -= 1;
    if(IsKeyDown(KEY_S)) vel.y += 1;

    f32 speed = WALK_SPEED;

    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        speed = 2.0f;
    }

    if(IsKeyPressed(KEY_G)) Player_toggle_collisions();

    if(Player_is_grounded(*player)) {
    }

    Vector2 final = Vector2Scale(vel, GetFrameTime() * speed);

    if(useCollisions) {
        Vector2 new = Vector2Add(player->pos, final);
    }
    else {
        player->pos = Vector2Add(player->pos, final);
    }
}

void Player_draw(Player* player) {
    DrawRectangleRec(
        (Rectangle) {
            player->pos.x - HALF_WIDTH, 
            player->pos.y - (player->height / 2.0f),
            HALF_WIDTH*2,
            player->height
        },
        RED
    );
}

bool Player_is_grounded(Player player) {
}

void Player_toggle_collisions(void) {
    useCollisions = !useCollisions;
}
