#include "augustus_level.h"

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define TILE_SIZE 32

Vector2 Vector2_WorldToTile(Vector2 vector) {
    return (Vector2) {
        roundf(vector.x - 0.5f),
        roundf(vector.y - 0.5f),
    };
}

Room Room_make(u32 w, u32 h) {
    Room r = {
        .w = w,
        .h = h,
        .foreground = calloc(w * h, sizeof(Tile)),
        .name = "unkown name"
    };

    srand(time(NULL));
    for(u32 i = 0; i < w * h; i++) {
        r.foreground[i].color = WHITE;
        r.foreground[i].type = rand() % 2 ? TILE_SOLID : TILE_NONE;
    }

    printf("\n");

    return r;
}

void Room_free(Room room) {
    free(room.foreground);
}

void Room_draw(Room room) {
    for(u32 i = 0; i < room.w; i++) {
        for(u32 j = 0; j < room.h; j++) {
            Tile tile = room.foreground[i + j * room.w];

            switch(tile.type) {
                case TILE_SOLID: {
                    DrawRectangle(
                        i, j,
                        1, 1,
                        tile.color
                    );
                    break;
                }
                default: break;
            }

        }
    }
}

Tile* Room_get(Room* room, i32 x, i32 y) {
    return &room->foreground[x + y * room->w];
}

void Room_resize(Room* room, u32 w, u32 h) {
    room->w = w;
    room->h = h;

    Tile* temp = realloc(room->foreground, w * h* sizeof(Tile));
    if(temp) {
        room->foreground = temp;
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

u32 Level_add_room(Level* level, u32 w, u32 h) {
    if(!level->rooms) {
        level->rooms = malloc(sizeof(Room));
    }

    u32 idx = level->rooms_len++;
    level->rooms[idx] = Room_make(w, h);

    return idx;
}

Room* Level_get_room(Level level, u32 idx) {
    if(idx > level.rooms_len) return NULL;

    return level.rooms + idx;
}

