#include "augustus_player.h"
#include "augustus_world.h"
#include "augustus_physics.h"

#include <raylib.h>
#include <raymath.h>

#include <stdio.h>
#include <stdlib.h>

#define EPSILON 0.00001f
#define GRAVITY 30.0f
#define VERTICAL_CAP 0.05f 
#define JUMP_HEIGHT -0.005f

static inline int imin(int a, int b) {
    return a < b ? a : b;
}

static inline int imax(int a, int b) {
    return a > b ? a : b;
}

static bool useCollisions = true;

static void Player_toggle_collisions(void) {
    useCollisions = !useCollisions;
}

static Vector2 StandingSize = { 0.9f, 1.9f };
static Vector2 CrouchingSize = { 0.9f, 0.9f };

Player Player_make(void) {
    return (Player) {
        .pos = (Vector2) { -2, -2 },
        .vel = Vector2Zero(),
        .size = StandingSize,
        .state = PLAYER_STANDING,

        .direction = PLAYER_RIGHT,
        .arm_angle = PLAYER_MIDDLE,

        .has_collision = false,
        .is_grounded = false,

        .bullets_len = 0,
        .bullets_allocated = 0,
        .bullets = NULL,
    };
}

void Player_free(Player* player) {
    if(player->bullets) free(player->bullets);
}

#define WALK_SPEED 10.0f

#define PLAYER_MIN ((Vector2) { player->pos.x, player->pos.y - player->size.y })
#define PLAYER_MAX ((Vector2) { player->pos.x + player->size.x, player->pos.y })


static bool Player_collision(Player* player, Vector2* min, Vector2* max) {
    Room* room = World_get(&world);

    Vector2 checkmin = Vector2_tile(PLAYER_MIN);
    Vector2 checkmax = Vector2_tile(Vector2AddValue(PLAYER_MAX, 1));

    // check only in the tiles that the player could be
    // speeds up checks
    // TODO: Add world bounds checking

    for(i64 x = imax(checkmin.x, 0); x < imin(checkmax.x, room->w); x++) {
        for(i64 y = imax(checkmin.y, 0); y < imin(checkmax.y, room->h); y++) {
            Tile* tile = Room_at(room, x, y);

            if(!tile) continue;
            if(tile->type == TILE_None) continue;

            Vector2 tilemin = (Vector2) { x, y };
            Vector2 tilemax = (Vector2) { x + 1, y + 1 };

            if(AABBvsAABB(PLAYER_MIN, PLAYER_MAX, tilemin, tilemax)) {
                *min = tilemin;
                *max = tilemax;
                return true;
            }
        }
    }

    return false;
}

static void Player_move_x(Player* player, f32 velX) {
    Vector2 tilemin, tilemax;
    Vector2 min, max;

    player->vel.x = velX;
    player->pos.x += player->vel.x;

    min = PLAYER_MIN;
    max = PLAYER_MAX;

    if(player->vel.x > 0) {
        player->direction = PLAYER_RIGHT;
    }
    else if(player->vel.x < 0) {
        player->direction = PLAYER_LEFT;
    }

    if(Player_collision(player, &tilemin, &tilemax)) {
        player->has_collision = true;

        f32 delta = 0;
        if(player->vel.x > 0) {
            delta = tilemin.x - max.x - EPSILON;
        }
        else if(player->vel.x < 0) {
            delta = tilemax.x - min.x + EPSILON;
        }

        player->pos.x += delta;
    }
}

static void Player_move_y(Player* player, f32 velY) {
    Vector2 tilemin, tilemax;
    Vector2 min, max;

    player->vel.y += velY;
    player->pos.y += player->vel.y;

    min = PLAYER_MIN;
    max = PLAYER_MAX;

    if(Player_collision(player, &tilemin, &tilemax)) {
        player->has_collision = true;

        f32 delta = 0;
        if(player->vel.y > 0) {
            player->is_grounded = true;
            player->vel.y = 0.0001f;

            delta = tilemin.y - max.y - EPSILON;
        }
        else {
            player->vel.y = 0;
            delta = tilemax.y - min.y + EPSILON;
        }

        player->pos.y += delta;
    }
}

static void Player_shoot(Player* player) {
    Vector2 bullet_dir = { 0, 0 };

    switch(player->direction) {
        case PLAYER_LEFT:
            bullet_dir.x = -1;
            break;
        case PLAYER_RIGHT:
            bullet_dir.x = 1;
            break;
    }

    switch(player->arm_angle) {
        case PLAYER_MIDDLE:
            bullet_dir.y = 0;
            break;
        case PLAYER_UP:
            bullet_dir.y = -1;
            break;
        case PLAYER_DOWN:
            bullet_dir.y = 1;
            break;
    }

    bullet_dir = Vector2Normalize(bullet_dir);

    if(!player->bullets) {
        player->bullets_allocated = 8;
        player->bullets = malloc(sizeof(player->bullets[0]) * player->bullets_allocated);
    }

    player->bullets[player->bullets_len] = (Bullet) {
        .pos = (Vector2) { PLAYER_MIN.x, PLAYER_MIN.y + player->size.y / 2.0f },
        .dir = bullet_dir,
        .size = (Vector2) { 0.5f, 0.2f },
        .speed = 30.0f,
    };
    player->bullets_len++;

    if(player->bullets_len >= player->bullets_allocated) {
        player->bullets_allocated *= 2;
        player->bullets = realloc(player->bullets, sizeof(player->bullets[0])*player->bullets_allocated);
    }
}

static void Player_delete_bullet(Player* player, u32 idx) {
    if(player->bullets_len <= 0) return;
    if(idx < 0 || idx >= player->bullets_len) return;

    if(idx < player->bullets_len - 1) { // is not last
        memmove(player->bullets + idx, player->bullets + idx + 1, sizeof(player->bullets[0]) * (player->bullets_len - idx));
    }

    player->bullets_len--;
}

static void Player_update_bullets(Player* player) {
    Room* room = World_get(&world);

    for(u32 i = 0; i < player->bullets_len; i++) {
        Bullet* bullet = player->bullets + i;

        f32 speed = bullet->speed * GetFrameTime();

        bullet->pos = Vector2Add(bullet->pos, Vector2Scale(bullet->dir, speed));

        Vector2 min = bullet->pos;
        Vector2 max = Vector2Add(min, bullet->size);

        Vector2 checkmin = Vector2_tile(bullet->pos);
        Vector2 checkmax = Vector2_tile(Vector2AddValue(Vector2Add(bullet->pos, bullet->size), 1));

        for(i64 x = imax(checkmin.x, 0); x < imin(checkmax.x, room->w); x++) {
            for(i64 y = imax(checkmin.y, 0); y < imin(checkmax.y, room->h); y++) {
                Tile* tile = Room_at(room, x, y);

                if(!tile) continue;
                if(tile->type == TILE_None) continue;

                Vector2 tilemin = (Vector2) { x, y };
                Vector2 tilemax = (Vector2) { x + 1, y + 1 };

                if(AABBvsAABB(min, max, tilemin, tilemax)) {
                    Player_delete_bullet(player, i);
                }
            }
        }
    }
}

void Player_update(Player* player) {
    Player_update_bullets(player);

    player->is_grounded = false;

    f32 xdir = 0;
    if(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) xdir -= 1;
    if(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) xdir += 1;

    f32 speed = WALK_SPEED;

    if(IsKeyDown(KEY_LEFT_SHIFT)) {
        speed = WALK_SPEED * 2;
    }

    if(IsKeyDown(KEY_LEFT_CONTROL)) {
        speed = WALK_SPEED / 4;
    }

    player->arm_angle = PLAYER_MIDDLE;
    if(IsKeyDown(KEY_Q)) {
        player->arm_angle = PLAYER_UP;
    }
    if(IsKeyDown(KEY_E)) {
        player->arm_angle = PLAYER_DOWN;
    }

    if(IsKeyPressed(KEY_Z)) {
        Player_shoot(player);
    }

    if(IsKeyPressed(KEY_G)) Player_toggle_collisions();

    if(IsKeyPressed(KEY_S) || IsKeyDown(KEY_DOWN)) {
        player->state = PLAYER_CROUCHING;
    }
    if(IsKeyPressed(KEY_W) || IsKeyDown(KEY_UP)) {
        Vector2 tilemin, tilemax;
        player->state = PLAYER_STANDING;
        player->size = StandingSize;
        if(Player_collision(player, &tilemin, &tilemax)) {
            player->state = PLAYER_CROUCHING;
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

    player->has_collision = false;
    if(useCollisions) {
        Player_move_x(player, xdir * GetFrameTime() * speed);
        Player_move_y(player, GRAVITY * GetFrameTime() * GetFrameTime());

        if(player->vel.y > VERTICAL_CAP) player->vel.y = VERTICAL_CAP;

        if(player->is_grounded) {
            if(IsKeyPressed(KEY_SPACE)) {
                printf("JUMP");
                player->vel.y = JUMP_HEIGHT;
            }
        }

    }
    else {
        f32 ydir = 0;
        if(IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) ydir -= 1;
        if(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) ydir += 1;

        player->vel.x = xdir * GetFrameTime() * speed;
        player->vel.y = ydir * GetFrameTime() * speed;

        player->pos = Vector2Add(player->pos, player->vel);
    }
}

void Player_draw(Player* player) {
    Vector2 p = (Vector2) { player->pos.x, player->pos.y - player->size.y };
    DrawRectangleV(p, player->size, player->is_grounded ? GREEN : BLUE);

    for(u32 i = 0; i < player->bullets_len; i++) {
        Bullet* bullet = player->bullets + i;

        DrawRectangleV(bullet->pos, bullet->size, RED);
#if 0
        Rectangle rec = { bullet->pos.x, bullet->pos.y, bullet->size.x, bullet->size.y };
        Vector2 origin = Vector2Scale(Vector2Add(bullet->pos, bullet->size), 0.5f);
        f32 angle = atan2f(bullet->dir.y, bullet->dir.x) * RAD2DEG;

        DrawRectanglePro(rec, bullet->pos, angle, RED);
#endif
    }
}
