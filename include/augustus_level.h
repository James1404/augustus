#ifndef AUGUSTUS_LEVEL_H
#define AUGUSTUS_LEVEL_H

#include "augustus_common.h"

#include <raylib.h>

typedef enum {
    TILE_NONE,
    TILE_SOLID,
    TILE_SLOPE,
    TILE_SPIKE,

    TILE_MAX,
} TileType;

typedef struct {
    u8 type;
    u16 sprite_index;
} Tile;

typedef struct {
    Tile* data;
    u32 w, h;
    Texture2D tilesheet;
} Level;

Level Level_make(u32 w, u32 h);
void Level_free(Level level);

void Level_draw(Level level);

#endif//AUGUSTUS_LEVEL_H
