#include "augustus_level.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

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
    room->data = realloc(room->data, w * h * sizeof(Tile));
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
                case TILE_SOLID:
                    color = WHITE;
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

void Level_new_room(Level* level, u64 w, u64 h) {
    u64 idx = level->rooms_len;

    level->rooms_len++;

    level->rooms = realloc(level->rooms, sizeof(level->rooms[0]) * level->rooms_len);
    level->rooms[idx] = Room_make(w, h);
}
