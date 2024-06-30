#include "augustus_level.h"

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static u32 format_version = 1;

static bool Level_read_V1(Level* level, FILE* file) {
    fread(&level->rooms_len, sizeof(level->rooms_len), 1, file);
    level->rooms = malloc(sizeof(level->rooms[0]) * level->rooms_len);

    for(u32 i = 0; i < level->rooms_len; i++) {
        Room* room = level->rooms + i;
        fread(&room->name, sizeof(room->name[0]), ROOM_NAME_LEN, file);
        fread(&room->w, sizeof(room->w), 1, file);
        fread(&room->h, sizeof(room->h), 1, file);
        room->data = malloc(sizeof(room->data[0])*room->w*room->h);
        fread(room->data, sizeof(room->data[0]), room->w * room->h, file);
    }

    return true;
}

void Level_write_to_file(Level* level, char name[LEVEL_NAME_LEN]) {
    FILE* file;

    file = fopen(TextFormat("resources/levels/%s.bin", name), "wb");

    fwrite(&level->rooms_len, sizeof(level->rooms_len), 1, file);
    for(u32 i = 0; i < level->rooms_len; i++) {
        Room* room = level->rooms + i;
        fwrite(&room->name, sizeof(room->name[0]), ROOM_NAME_LEN, file);
        fwrite(&room->w, sizeof(room->w), 1, file);
        fwrite(&room->h, sizeof(room->h), 1, file);
        fwrite(room->data, sizeof(room->data[0]), room->w * room->h, file);
    }

    fclose(file);
}

bool Level_read_from_file(Level* level, char name[LEVEL_NAME_LEN]) {
    FILE* file;

    file = fopen(TextFormat("resources/levels/%s.bin", name), "rb");

    bool success = false;

    if(file) {
        Level_free(level);

        Level_read_V1(level, file);

        success = true;
    }

    fclose(file);

    return success;
}
