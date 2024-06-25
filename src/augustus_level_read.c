#include "augustus_level.h"

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static u32 format_version = 1;

static bool Level_read_V1(Level* level, FILE* file) {
    fread(&level->segments_len, sizeof(level->segments_len), 1, file);
    level->segments = malloc(sizeof(level->segments[0]) * level->segments_len);

    for(u32 i = 0; i < level->segments_len; i++) {
        Segment* segment = level->segments + i;
        *segment = Segment_make();
        fread(&segment->len, sizeof(segment->len), 1, file);
        segment->vertices = malloc(sizeof(segment->vertices[0])*segment->len);
        fread(segment->vertices, sizeof(segment->vertices[0]), segment->len, file);
    }

    fread(&level->splats_len, sizeof(level->splats_len), 1, file);
    level->splats = malloc(sizeof(level->splats[0]) * level->splats_len);
    for(u32 i = 0; i < level->splats_len; i++) {
        Splat* splat = level->splats + i;

        u64 name_len = 0;
        fread(&name_len, sizeof(name_len), 1, file);
        fread(&splat->filepath, sizeof(splat->filepath[0]), name_len, file);

        fread(&splat->pos, sizeof(splat->pos), name_len, file);
        fread(&splat->scl, sizeof(splat->scl), name_len, file);
        fread(&splat->rot, sizeof(splat->rot), name_len, file);
        fread(&splat->layer, sizeof(splat->layer), name_len, file);
    }

    fread(&level->enemies_len, sizeof(level->enemies_len), 1, file);
    level->enemies = malloc(sizeof(level->enemies[0]) * level->enemies_len);
    fread(level->enemies, sizeof(level->enemies[0]), level->enemies_len, file);

    return true;
}

void Level_write_to_file(Level* level, char name[LEVEL_NAME_LEN]) {
    FILE* file;

    file = fopen(TextFormat("resources/levels/%s.bin", name), "wb");

    LevelHeader header = {
        .version = format_version,
        .author = "James",
        .name = "Empty",
    };

    strcpy(header.name, name);

    fwrite(&header, sizeof(header), 1, file);

    fwrite(&level->segments_len, sizeof(level->segments_len), 1, file);
    for(u32 i = 0; i < level->segments_len; i++) {
        Segment* segment = level->segments + i;
        fwrite(&segment->len, sizeof(segment->len), 1, file);
        fwrite(segment->vertices, sizeof(segment->vertices[0]), segment->len, file);
    }

    fwrite(&level->splats_len, sizeof(level->splats_len), 1, file);
    for(u32 i = 0; i < level->splats_len; i++) {
        Splat* splat = level->splats + i;

        u64 name_len = strlen(splat->filepath);
        fwrite(&name_len, sizeof(name_len), 1, file);
        fwrite(splat->filepath, sizeof(splat->filepath[0]), name_len, file);

        fwrite(&splat->pos, sizeof(splat->pos), 1, file);
        fwrite(&splat->scl, sizeof(splat->scl), 1, file);
        fwrite(&splat->rot, sizeof(splat->rot), 1, file);
        fwrite(&splat->layer, sizeof(splat->layer), 1, file);
    }

    fwrite(&level->enemies_len, sizeof(level->enemies_len), 1, file);
    if(level->enemies) {
        fwrite(level->enemies, sizeof(level->enemies[0]), level->enemies_len, file);
    }

    fclose(file);
}

bool Level_read_from_file(Level* level, char name[LEVEL_NAME_LEN]) {
    FILE* file;

    file = fopen(TextFormat("resources/levels/%s.bin", name), "rb");

    bool success = false;

    if(file) {
        Level_free(level);

        LevelHeader header = {0};

        fread(&header, sizeof(header), 1, file);

        switch(header.version) {
            case 1:
                Level_read_V1(level, file);
                break;
            default:
                *level = Level_make();
                break;
        }

        success = true;
    }

    fclose(file);

    return success;
}
