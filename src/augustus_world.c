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
        .data = calloc(w * h, sizeof(Tile))
    };
}

void Room_free(Room* room) {
    free(room->data);

    room->data = NULL;
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
}

World World_make(void) {
    return (World) {
        .rooms = NULL,
        .rooms_len = 0,
    };
}

void World_free(World* world) {
    if(world->rooms) free(world->rooms);

    *world = World_make();
}

void World_draw(World world) {
    Room_draw(world.rooms + world.current_room);
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
