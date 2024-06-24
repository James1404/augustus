#include "augustus_player.h"
#include "augustus_level.h"
#include "augustus_physics.h"
#include <raylib.h>
#include <raymath.h>

#include <stdio.h>

static bool useCollisions = true;

Player Player_make(void) {
    return (Player) {
        .pos = (Vector2) {0},
        .height = 2,
        .state = PLAYER_STANDING,
        .has_collision = false,
    };
}

void Player_free(Player* player) {
}

#define WALK_SPEED 20.0f
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

    vel = Vector2Scale(vel, GetFrameTime() * speed);

    player->has_collision = false;
    if(useCollisions) {
        Vector2 min = { player->pos.x - HALF_WIDTH, player->pos.y - (player->height / 2.0) };
        Vector2 max = { player->pos.x + HALF_WIDTH, player->pos.y + (player->height / 2.0) };

        Vector2 d = {
            fmax(min.x - player->pos.x, fmax(0, player->pos.x - max.x)),
            fmax(min.y - player->pos.y, fmax(0, player->pos.y - max.y)),
        };
        f32 EdgeDist = Vector2Length(d);

        f32 t = 1.0f;
        Vector2 p = {0}, normal = {0};

        for(u32 i = 0; i < level.segments_len; i++) {
            Segment* segment = level.segments + i;
            for(u32 j = 0; j < segment->len; j++) {
                Vector2 a = segment->vertices[j];
                Vector2 b = segment->vertices[(j + 1) % segment->len];
                Vector2 delta = Vector2Subtract(b, a);

                f32 newt = 1.0f;
                Vector2 newp = {0}, newnormal = {0};
                bool intersect = LineVsLine(player->pos, Vector2Add(player->pos, vel), a, b, &newt, &newp);
                if(intersect) {
                    if(newt < t) {
                        t = newt;
                        p = newp;
                        normal = newnormal;

                        Vector2 u = { b.y - a.y, a.x - b.x };
                        Vector2 n = Vector2Normalize(u);
                        f32 dir = Vector2DotProduct(Vector2Subtract(player->pos, a), n);
                    }
                }
            }
        }

        player->pos = Vector2Add(player->pos, Vector2Scale(vel, t - EdgeDist - EPSILON));
    }
    else {
        player->pos = Vector2Add(player->pos, vel);
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
        (player->has_collision) ? RED : WHITE
    );
}

bool Player_is_grounded(Player player) {
    return false;
}

void Player_toggle_collisions(void) {
    useCollisions = !useCollisions;
}
