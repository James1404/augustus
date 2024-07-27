#include "augustus_world.h"
#include "augustus_physics.h"
#include "augustus_window.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

World world;

vec2s vec2s_tile(vec2s v) {
    return (vec2s) {
        roundf(v.x - 0.5f),
        roundf(v.y - 0.5f),
    };
}

Room Room_make(u64 w, u64 h) {
    return (Room) {
        .name = "Unnamed Room",
        .w = w,
        .h = h,
        .data = calloc(w * h, sizeof(Tile)),

        .enemies = NULL,
        .enemies_len = 0,
        .enemies_allocated = 0,
    };
}

void Room_free(Room* room) {
    free(room->enemies);
    room->enemies = NULL;

    free(room->data);
    room->data = NULL;
}

void Room_update(Room* room) {
    for(u32 i = 0; i < room->enemies_len; i++) {
        Enemy_update(room->enemies + i);
    }
}

void Room_draw(Room* room) {
    for(u64 x = 0; x < room->w; x++) {
        for(u64 y = 0; y < room->h; y++) {
            Tile* tile = &room->data[x + y * room->w];
#if 0 // TODO:
            Color color = BLANK;
            switch(tile->type) {
                case TILE_None:
                    color = BLANK;
                    break;
                case TILE_Solid:
                    color = WHITE;
                    break;
                case TILE_Spike:
                    color = RED;
                    break;
                case TILE_Water:
                    color = BLUE;
                    break;
                case TILE_Lava:
                    color = ORANGE;
                    break;
            }

            DrawRectangle(x, y, 1, 1, color);
#endif
        }
    }

    for(u32 i = 0; i < room->enemies_len; i++) {
        Enemy_draw(room->enemies + i);
    }
}

void Room_add_enemy(Room* room, Enemy enemy) {
    if(!room->enemies) {
        room->enemies_allocated = 8;
        room->enemies = malloc(sizeof(room->enemies[0]) * room->enemies_allocated);
    }

    u32 idx = room->enemies_len;
    room->enemies_len++;

    if(room->enemies_len >= room->enemies_allocated) {
        room->enemies_allocated *= 2;
        room->enemies = realloc(room->enemies, sizeof(room->enemies[0])*room->enemies_allocated);
    }

    room->enemies[idx] = enemy;
}

void Room_resize(Room* room, u64 w, u64 h) {
    Tile* temp = calloc(w * h, sizeof(Tile));
    for(u64 y = 0; y < room->h && y < h; y++) {
        u64 new_w = imin(room->w, w);
        memcpy(&temp[w * y], &room->data[room->w * y], new_w * sizeof(Tile));
    }

    room->data = temp;

    room->w = w;
    room->h = h;
}

Tile* Room_at(Room* room, u64 x, u64 y) {
    if(x > room->w || x < 0) return NULL;
    if(y > room->h || y < 0) return NULL;

    return &room->data[x + y * room->w];
}

World World_make(void) {
    return (World) {
        .rooms = NULL,
        .rooms_len = 0,

        .player = Player_make(),
        .current_room = 0,

        .bullets_len = 0,
        .bullets_allocated = 0,
        .bullets = NULL,
    };
}

void World_free(World* world) {
    free(world->bullets);

    Player_free(&world->player);

    for(u32 i = 0; i < world->rooms_len; i++) {
        Room_free(world->rooms + i);
    }

    free(world->rooms);

    *world = World_make();
}

static void World_delete_bullet(World* world, u32 idx) {
    if(world->bullets_len <= 0) return;
    if(idx < 0 || idx >= world->bullets_len) return;

    if(idx < world->bullets_len - 1) { // is not last
        memmove(world->bullets + idx, world->bullets + idx + 1, sizeof(world->bullets[0]) * (world->bullets_len - idx));
    }

    world->bullets_len--;
}

static void World_update_bullets(World* world) {
    Room* room = World_get(world);

    for(u32 i = 0; i < world->bullets_len; i++) {
        Bullet* bullet = world->bullets + i;

        f32 speed = bullet->speed * Window_DeltaTime();

        bullet->pos = glms_vec2_add(bullet->pos, glms_vec2_scale(bullet->dir, speed));

        if( (bullet->pos.x < 0 || bullet->pos.x > room->w) ||
            (bullet->pos.y < 0 || bullet->pos.y > room->h)) {
            World_delete_bullet(world, i);
            continue;
        }

        vec2s min = bullet->pos;
        vec2s max = glms_vec2_add(min, bullet->size);

        if(bullet->from_player) {
            for(u32 i = 0; i < room->enemies_len; i++) {
                Enemy* enemy = room->enemies + i;

                if(!enemy->alive) continue;

                vec2s enemymin = enemy->pos;
                vec2s enemymax = glms_vec2_add(enemymin, enemy->size);

                if(AABBvsAABB(min, max, enemymin, enemymax)) {
                    enemy->alive = false;
                    World_delete_bullet(world, i);
                }
            }
        }

        vec2s checkmin = vec2s_tile(bullet->pos);
        vec2s checkmax = vec2s_tile(glms_vec2_adds(glms_vec2_add(bullet->pos, bullet->size), 1));

        for(i64 x = imax(checkmin.x, 0); x < imin(checkmax.x, room->w); x++) {
            for(i64 y = imax(checkmin.y, 0); y < imin(checkmax.y, room->h); y++) {
                Tile* tile = Room_at(room, x, y);

                if(!tile) continue;
                if(tile->type == TILE_None) continue;

                vec2s tilemin = (vec2s) { x, y };
                vec2s tilemax = (vec2s) { x + 1, y + 1 };

                if(AABBvsAABB(min, max, tilemin, tilemax)) {
                    World_delete_bullet(world, i);
                }
            }
        }
    }
}

void World_update(World* world) {
    Player_update(&world->player);
    Room_update(World_get(world));

    World_update_bullets(world);
}

void World_draw(World world) {
    Room_draw(world.rooms + world.current_room);

    for(u32 i = 0; i < world.bullets_len; i++) {
        Bullet* bullet = world.bullets + i;

        // todo draw
        // DrawRectangleV(bullet->pos, bullet->size, RED);
#if 0
        Rectangle rec = { bullet->pos.x, bullet->pos.y, bullet->size.x, bullet->size.y };
        vec2s origin = vec2sMulValue(vec2sAdd(bullet->pos, bullet->size), 0.5f);
        f32 angle = atan2f(bullet->dir.y, bullet->dir.x) * RAD2DEG;

        DrawRectanglePro(rec, bullet->pos, angle, RED);
#endif
    }


    Player_draw(&world.player);
}

Room* World_get(World* world) {
    return world->rooms + world->current_room;
}

u64 World_new_room(World* world) {
    u64 idx = world->rooms_len;

    world->rooms_len++;

    world->rooms = realloc(world->rooms, sizeof(world->rooms[0]) * world->rooms_len);
    world->rooms[idx] = Room_make(DEFAULT_ROOM_WIDTH, DEFAULT_ROOM_HEIGHT);

    return idx;
}

void World_remove_room(World* world, u64 idx) {
    if(idx < 0 || idx >= world->rooms_len) return;

    if(idx < world->rooms_len - 1) { // is not last
        memmove(world->rooms + idx, world->rooms + idx + 1, sizeof(world->rooms[0]) * (world->rooms_len - idx));
    }

    world->rooms_len--;
    world->rooms = realloc(world->rooms, sizeof(world->rooms[0]) * world->rooms_len);

    if(world->current_room == idx) {
        world->current_room = idx <= 1 ? 0 : idx - 1;
    }

    if(world->rooms_len == 0) {
        World_new_room(world);
    }
}

void World_spawn_bullet(World* world, Bullet bullet) {
    if(!world->bullets) {
        world->bullets_allocated = 8;
        world->bullets = malloc(sizeof(world->bullets[0]) * world->bullets_allocated);
    }

    world->bullets[world->bullets_len] = bullet;
    world->bullets_len++;

    if(world->bullets_len >= world->bullets_allocated) {
        world->bullets_allocated *= 2;
        world->bullets = realloc(world->bullets, sizeof(world->bullets[0])*world->bullets_allocated);
    }
}
