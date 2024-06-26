#include "augustus_level.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

static inline int imin(int a, int b) {
    return a < b ? a : b;
}

Level level;

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

Level Level_make(void) {
    return (Level) {
        .rooms = NULL,
        .rooms_len = 0,
    };
}

void Level_free(Level* level) {
    if(level->rooms) free(level->rooms);

    *level = Level_make();
}

void Level_draw(Level level) {
    Room_draw(level.rooms + level.current_room);
}

Room* Level_get(Level* level) {
    return level->rooms + level->current_room;
}

u64 Level_new_room(Level* level) {
    u64 idx = level->rooms_len;

    level->rooms_len++;

    level->rooms = realloc(level->rooms, sizeof(level->rooms[0]) * level->rooms_len);
    level->rooms[idx] = Room_make(DEFAULT_ROOM_WIDTH, DEFAULT_ROOM_HEIGHT);

    return idx;
}

void Level_remove_room(Level* level, u64 idx) {
    if(idx < 0 || idx >= level->rooms_len) return;

    if(idx < level->rooms_len - 1) { // is not last
        memmove(level->rooms + idx, level->rooms + idx + 1, sizeof(level->rooms[0]) * (level->rooms_len - idx));
    }

    level->rooms_len--;
    level->rooms = realloc(level->rooms, sizeof(level->rooms[0]) * level->rooms_len);

    if(level->current_room == idx) {
        level->current_room = idx <= 1 ? 0 : idx - 1;
    }

    if(level->rooms_len == 0) {
        Level_new_room(level);
    }
}
