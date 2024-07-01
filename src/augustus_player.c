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

static Vector2 StandingSize = { 0.9f, 1.9f };
static Vector2 CrouchingSize = { 0.9f, 0.9f };

Player Player_make(void) {
    return (Player) {
        .pos = (Vector2) { -2, -2 },
        .vel = Vector2Zero(),
        .size = StandingSize,
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

#define EPSILON 0.000001f
#define GRAVITY 13.0f
#define VERTICAL_CAP 0.05f 
#define JUMP_HEIGHT -0.002f

static void Player_do_collision(Player* player) {
    Vector2 tilemin, tilemax;
    Vector2 min, max;

    if(player->vel.x != 0) {
        player->pos.x += player->vel.x;

        min = player->pos;
        max = Vector2Add(player->pos, player->size);

        if(Player_collision(player, &tilemin, &tilemax)) {
            player->has_collision = true;

            f32 delta = 0;
            if(player->vel.x > 0) {
                delta = tilemin.x - max.x - EPSILON;
            }
            else {
                delta = tilemax.x - min.x + EPSILON;
            }

            player->pos.x += delta;
        }
    }

    if(player->vel.y != 0) {
        player->pos.y += player->vel.y;

        min = player->pos;
        max = Vector2Add(player->pos, player->size);

        if(Player_collision(player, &tilemin, &tilemax)) {
            player->has_collision = true;

            f32 delta = 0;
            if(player->vel.y > 0) {
                player->is_grounded = true;
                delta = tilemin.y - max.y - EPSILON;
            }
            else {
                player->vel.y = 0;
                delta = tilemax.y - min.y + EPSILON;
            }

            player->pos.y += delta;
        }
    }
}

void Player_update(Player* player) {
    player->is_grounded = false;

    f32 xDir = 0;
    if(IsKeyDown(KEY_A)) xDir -= 1;
    if(IsKeyDown(KEY_D)) xDir += 1;

    f32 speed = WALK_SPEED;

    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        speed = WALK_SPEED * 2;
    }

    if(IsKeyPressed(KEY_G)) Player_toggle_collisions();

    if(IsKeyPressed(KEY_C)) {
        if(player->state != PLAYER_CROUCHING) {
            player->state = PLAYER_CROUCHING;
        }
        else {
            player->state = PLAYER_STANDING;
        }
    }

    switch(player->state) {
        case PLAYER_STANDING:
            player->size = StandingSize;
            break;
        case PLAYER_CROUCHING:
            player->size = CrouchingSize;
            speed /= 2.0f;
            break;
    }

    player->vel.x = xDir * GetFrameTime() * speed;

    player->has_collision = false;
    if(useCollisions) {
        player->vel.y += GRAVITY * GetFrameTime() * GetFrameTime();
        if(player->vel.y > VERTICAL_CAP) player->vel.y = VERTICAL_CAP;

        Player_do_collision(player);

        if(player->is_grounded) {
            player->vel.y = 0.0001f;

            if(IsKeyPressed(KEY_SPACE)) {
                printf("JUMP");
                player->vel.y = JUMP_HEIGHT;
            }
        }

    }
    else {
        f32 yDir = 0;
        if(IsKeyDown(KEY_W)) yDir -= 1;
        if(IsKeyDown(KEY_S)) yDir += 1;
        player->vel.y = yDir * GetFrameTime() * speed;

        player->pos = Vector2Add(player->pos, player->vel);
    }
}

void Player_draw(Player* player) {
    DrawRectangleV(player->pos, player->size, player->is_grounded ? GREEN : BLUE);
}

void Player_toggle_collisions(void) {
    useCollisions = !useCollisions;
}
