#include "augustus_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

static inline int imin(int a, int b) {
    return a < b ? a : b;
}

World world;

Vector2 Vector2_tile(Vector2 v) {
    return (Vector2) {
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
    if(room->enemies) free(room->enemies);
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
            Color color = BLANK;
            switch(tile->type) {
                case TILE_Solid:
                    color = WHITE;
                    break;
                case TILE_Spike:
                    color = RED;
                    break;
                default: break;
            }

            DrawRectangle(x, y, 1, 1, color);
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

    room->enemies[room->enemies_len] = enemy;
    room->enemies_len++;

    if(room->enemies_len >= room->enemies_allocated) {
        room->enemies_allocated *= 2;
        room->enemies = realloc(room->enemies, sizeof(room->enemies[0])*room->enemies_allocated);
    }
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
    };
}

void World_free(World* world) {
    Player_free(&world->player);

    for(u32 i = 0; i < world->rooms_len; i++) {
        Room_free(world->rooms + i);
    }

    if(world->rooms) free(world->rooms);

    *world = World_make();
}

void World_update(World* world) {
    Player_update(&world->player);
    Room_update(World_get(world));
}

void World_draw(World world) {
    Room_draw(world.rooms + world.current_room);
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
