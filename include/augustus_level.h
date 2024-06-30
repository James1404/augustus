#ifndef AUGUSTUS_LEVEL_H
#define AUGUSTUS_LEVEL_H

#include "augustus_common.h"

#include <raylib.h>

Vector2 Vector2_tile(Vector2 v);

typedef struct {
    enum {
        TILE_NONE,
        TILE_SOLID
    } type;
    u64 sprite_index;
} Tile;

#define ROOM_NAME_LEN 24

typedef struct {
    char name[24];
    u64 w, h;
    Tile* data;
} Room;

Room Room_make(u64 w, u64 h);
void Room_free(Room* room);

void Room_resize(Room* room, u64 w, u64 h);

Tile* Room_at(Room* room, u64 x, u64 y);

void Room_draw(Room* room);

#define LEVEL_NAME_LEN 24

typedef struct {
    Room* rooms;
    u64 rooms_len;

    u64 current_room;
} Level;

extern Level level;

Level Level_make(void);
void Level_free(Level* level);

void Level_draw(Level level);

void Level_new_room(Level* level, u64 w, u64 h);

Room* Level_get(Level* level);

void Level_write_to_file(Level* level, char name[LEVEL_NAME_LEN]);
bool Level_read_from_file(Level* level, char name[LEVEL_NAME_LEN]);

#endif//AUGUSTUS_LEVEL_H
