#ifndef AUGUSTUS_LEVEL_H
#define AUGUSTUS_LEVEL_H

#include "augustus_common.h"

#include <raylib.h>

Vector2 Vector2_WorldToTile(Vector2 vector);

#define FOR_TILES(DO)\
    DO(TILE_NONE)\
    DO(TILE_SOLID)\
    DO(TILE_SLOPE)\
    DO(TILE_SPIKE)\

typedef enum {
#define TILES_ENUM(x) x,
    FOR_TILES(TILES_ENUM)

    TILE_MAX,
} TileType;

typedef struct {
    u8 type;
    Color color;
} Tile;

typedef struct {
    u32 to;

    i32 x, y;
} Door;

#define MAX_NAME_LEN 24
typedef struct {
    u32 w, h;
    Tile* foreground;

    Door* doors;
    u32 doors_len;

    char name[MAX_NAME_LEN];
} Room;

Room Room_make(u32 w, u32 h);
void Room_free(Room level);

void Room_draw(Room room);

Tile* Room_get(Room* room, i32 x, i32 y);

void Room_resize(Room* room, u32 w, u32 h);

typedef struct {
    Room* rooms;
    u32 rooms_len;
} Level;

Level Level_make(void);
void Level_free(Level* level);

u32 Level_add_room(Level* level, u32 w, u32 h);
Room* Level_get_room(Level level, u32 idx);

#endif//AUGUSTUS_LEVEL_H
