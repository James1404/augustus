#include "augustus_player.h"
#include "augustus_level.h"
#include "augustus_physics.h"

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

static inline int imin(int a, int b) {
    return a < b ? a : b;
}

static inline int imax(int a, int b) {
    return a > b ? a : b;
}

static bool useCollisions = true;

Player Player_make(void) {
    return (Player) {
        .pos = (Vector2) { -2, -2 },
        .size = (Vector2) { 1.0f, 2.0f },
        .state = PLAYER_STANDING,
        .has_collision = false,
    };
}

void Player_free(Player* player) {
}

#define WALK_SPEED 10.0f

static bool Player_collision(Player* player, Vector2* min, Vector2* max) {
    Room* room = Level_get(&level);

    Vector2 pmin = player->pos;
    Vector2 pmax = Vector2Add(player->pos, player->size);

    Vector2 checkmin = Vector2_tile(player->pos);
    Vector2 checkmax = Vector2_tile(Vector2AddValue(Vector2Add(player->pos, player->size), 1));

    // check only in the tiles that the player could be
    // speeds up checks
    // TODO: Add level bounds checking

    for(i64 x = imax(checkmin.x, 0); x < imin(checkmax.x, room->w); x++) {
        for(i64 y = imax(checkmin.y, 0); y < imin(checkmax.y, room->h); y++) {
            Tile* tile = Room_at(room, x, y);

            if(tile->type == TILE_None) continue;

            Vector2 tilemin = (Vector2) { x, y };
            Vector2 tilemax = (Vector2) { x + 1, y + 1 };

            if(AABBvsAABB(pmin, pmax, tilemin, tilemax)) {
                *min = tilemin;
                *max = tilemax;
                return true;
            }
        }
    }

    return false;
}

static void Player_do_collision(Player* player, Vector2  vel) {
    Vector2 tilemin, tilemax;
    Vector2 min, max;

    if(vel.x != 0) {
        player->pos.x += vel.x;

        min = player->pos;
        max = Vector2Add(player->pos, player->size);

        if(Player_collision(player, &tilemin, &tilemax)) {
            player->has_collision = true;

            f32 delta = 0;
            if(vel.x > 0) {
                delta = tilemin.x - max.x - EPSILON;
            }
            else {
                delta = tilemax.x - min.x + EPSILON;
            }

            player->pos.x += delta;
        }
    }

    if(vel.y != 0) {
        player->pos.y += vel.y;

        min = player->pos;
        max = Vector2Add(player->pos, player->size);

        if(Player_collision(player, &tilemin, &tilemax)) {
            player->has_collision = true;

            f32 delta = 0;
            if(vel.y > 0) {
                delta = tilemin.y - max.y - EPSILON;
            }
            else {
                delta = tilemax.y - min.y + EPSILON;
            }

            player->pos.y += delta;
        }
    }
}

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
        Player_do_collision(player, vel);
    }
    else {
        player->pos = Vector2Add(player->pos, vel);
    }
}

void Player_draw(Player* player) {
    DrawRectangleV(player->pos, player->size, player->has_collision ? PURPLE :BLUE);
}

bool Player_is_grounded(Player player) {
    return false;
}

void Player_toggle_collisions(void) {
    useCollisions = !useCollisions;
}
