#include "augustus_player.h"
#include "augustus_level.h"
#include "augustus_physics.h"
#include "box2d/collision.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <math.h>

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

    Vector2 final = Vector2Scale(vel, GetFrameTime() * speed);
    Vector2 new = Vector2Add(player->pos, final);

    player->has_collision = false;
    if(useCollisions) {
        Vector2 boundingBoxVertices[] = {
            Vector2Add(new, (Vector2) { -HALF_WIDTH, -(player->height / 2.0f) }),
            Vector2Add(new, (Vector2) { -HALF_WIDTH, player->height / 2.0f }),
            Vector2Add(new, (Vector2) { HALF_WIDTH, -(player->height / 2.0f) }),
            Vector2Add(new, (Vector2) { HALF_WIDTH, player->height / 2.0f }),
        };

        for(u32 i = 0; i < level.segments_len; i++) {
            Segment* segment = level.segments + i;
            for(u32 j = 0; j < segment->len; j++) {
                Vector2 a = segment->vertices[j];
                Vector2 b = segment->vertices[(j + 1) % segment->len];
                Vector2 delta = Vector2Subtract(b, a);

                u32 normals_len = 2;
                Vector2 normals[] = { { -delta.y, delta.x }, delta };

                Vector2 line_vertices[] = { a, b };

                for(u32 k = 0; k < normals_len; k++) {
                    f32 min1, max1, min2, max2;
                    SAT_projection(boundingBoxVertices, 4, normals[k], &min1, &max1);
                    SAT_projection(line_vertices, 2, normals[k], &min2, &max2);

                    if (max1 < min2 || max2 < min1) {
                        player->has_collision = true;
                        goto done;
                    }
                }
            }
        }
done:
        player->pos = new;
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
        (player->has_collision) ? RED : WHITE
    );
}

bool Player_is_grounded(Player player) {
}

void Player_toggle_collisions(void) {
    useCollisions = !useCollisions;
}
