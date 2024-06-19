#include "augustus_level.h"

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TILE_SIZE 32

Level Level_make(u32 w, u32 h) {
    Level r = {
        .w = w,
        .h = h,
        .data = calloc(w * h, sizeof(Tile)),
        .tilesheet = LoadTexture("resources/tileset.png"),
    };

    srand(time(NULL));
    for(u32 i = 0; i < w * h; i++) {
        u32 tw = r.tilesheet.width / TILE_SIZE;
        u32 th = r.tilesheet.height / TILE_SIZE;
        r.data[i].sprite_index = rand() % (tw * th);
        r.data[i].type = rand() % TILE_MAX; 
        printf("%hu, ", r.data[i].sprite_index);
    }

    printf("\n");

    return r;
}

void Level_free(Level level) {
    UnloadTexture(level.tilesheet);
    free(level.data);
}

void Level_draw(Level level) {
    for(u32 i = 0; i < level.w; i++) {
        for(u32 j = 0; j < level.h; j++) {
            Tile tile = level.data[i + level.w + j];

            if(tile.type == TILE_NONE) continue;

            DrawTextureRec(level.tilesheet,
                (Rectangle) {
                    .x = (tile.sprite_index * TILE_SIZE) % (u32)(level.tilesheet.width / (f32)TILE_SIZE),
                    .y = (tile.sprite_index * TILE_SIZE) / (level.tilesheet.height / (f32)TILE_SIZE),
                    .width = TILE_SIZE,
                    .height = TILE_SIZE
                },
                (Vector2) { i * TILE_SIZE, j * TILE_SIZE },
                WHITE
            );
        }
    }
}
