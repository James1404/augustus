#include "augustus_level.h"

#include "raylib.h"
#include "raymath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static u32 format_version = 1;

Level level;

Segment Segment_make(void) {
    return (Segment) {
        .vertices = NULL,
        .len = 0,
    };
}

void Segment_free(Segment* segment) {
    if(segment->vertices) free(segment->vertices);
    *segment = Segment_make();
}

void Segment_add_vertex(Segment* segment, Vector2 pos) {
    u32 idx = segment->len;
    segment->len++;

    segment->vertices = realloc(segment->vertices, sizeof(segment->vertices[0]) * segment->len);

    segment->vertices[idx] = pos;
}

void Segment_delete_vertex(Segment* segment, u64 idx) {
    if(idx < 0 || idx > segment->len) return;

    for(u64 i = idx; i < segment->len - 1; i++) {
        segment->vertices[i] = segment->vertices[i + 1];
    }

    segment->len--;

    segment->vertices = realloc(segment->vertices, sizeof(segment->vertices[0]) * segment->len);
}

void Segment_insert(Segment* segment, Vector2 elem, u64 at) {
    segment->len++;
    segment->vertices = realloc(segment->vertices, sizeof(segment->vertices[0]) * segment->len);


    memcpy(segment->vertices + at + 1, segment->vertices + at, (segment->len - at) * sizeof(segment->vertices[0]));

    segment->vertices[at] = elem;
}

void Segment_draw(Segment* segment, Color color) {
    if(segment->len >= 2) {
        for(u32 j = 0; (j < segment->len && segment->wrap) || j < segment->len - 1; j++) {
            Vector2 a = segment->vertices[j];
            Vector2 b = segment->vertices[(j + 1) % segment->len];
            Vector2 delta = Vector2Subtract(b, a);
            Vector2 normal = { -delta.y, delta.x };
            Vector2 midpoint = Vector2Scale(Vector2Add(a,b), 0.5);

            DrawLineV(a, b, color);

            DrawLineV(midpoint, Vector2Add(midpoint, Vector2Scale(normal, 0.1f)), RED);
        }
    }
}

Splat Splat_make(const char* filepath) {
    return (Splat) {
        .texture = LoadTexture(filepath),
        .filepath = filepath,
        .pos = Vector2Zero(),
        .scl = 1.0f,
        .layer = 0
    };
    
}

void Splat_free(Splat* splat) {
    UnloadTexture(splat->texture);
}

void Splat_draw(Splat* splat) {
    DrawTextureEx(splat->texture, splat->pos, splat->rot, splat->scl, WHITE);
}

Level Level_make(void) {
    return (Level) {
        .segments = NULL,
        .segments_len = 0,
        .splats = NULL,
        .splats_len = 0,
        .enemies = NULL,
        .enemies_len = 0,
    };
}

void Level_free(Level* level) {
    for(u32 i = 0; i < level->segments_len; i++) {
        Segment* segment = level->segments + i;
        Segment_free(segment);
    }

    if(level->segments) free(level->segments);
    if(level->splats) free(level->splats);
    if(level->enemies) free(level->enemies);

    *level = Level_make();
}

void Level_draw(Level level) {
    for(u32 i = 0; i < level.segments_len; i++) {
        Segment* seg = level.segments + i;
        Segment_draw(seg, GREEN);
    }
}

void Level_new_segment(Level* level, Segment segment) {
    u32 idx = level->segments_len;
    level->segments_len++;

    level->segments = realloc(level->segments, sizeof(level->segments[0]) * level->segments_len);

    level->segments[idx] = segment;
}

void Level_remove_segment(Level* level, u64 idx) {
    if(idx < 0 || idx > level->segments_len) return;

    for(u64 i = idx; i < level->segments_len - 1; i++) {
        level->segments[i] = level->segments[i + 1];
    }

    level->segments_len--;

    level->segments = realloc(level->segments, sizeof(level->segments[0]) * level->segments_len);
}

void Level_write_to_file(Level* level, const char* filename) {
    FILE* file;

    file = fopen(filename, "wb");

    fwrite(&format_version, sizeof(format_version), 1, file);

    fwrite(&level->segments_len, sizeof(level->segments_len), 1, file);
    for(u32 i = 0; i < level->segments_len; i++) {
        Segment* segment = level->segments + i;
        fwrite(&segment->len, sizeof(segment->len), 1, file);
        fwrite(segment->vertices, sizeof(segment->vertices[0]), segment->len, file);
        fwrite(&segment->wrap, sizeof(segment->wrap), 1, file);
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

static bool Level_read_V1(Level* level, FILE* file) {
    fread(&level->segments_len, sizeof(level->segments_len), 1, file);
    level->segments = malloc(sizeof(level->segments[0]) * level->segments_len);

    for(u32 i = 0; i < level->segments_len; i++) {
        Segment* segment = level->segments + i;
        fread(&segment->len, sizeof(segment->len), 1, file);
        segment->vertices = malloc(sizeof(segment->vertices[0])*segment->len);
        fread(segment->vertices, sizeof(segment->vertices[0]), segment->len, file);

        fread(&segment->wrap, sizeof(segment->wrap), 1, file);
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

bool Level_read_from_file(Level* level, const char* filename) {
    FILE* file;

    file = fopen(filename, "rb");

    bool success = false;

    if(file) {
        Level_free(level);

        u32 version = 0;
        fread(&version, sizeof(u32), 1, file);

        switch(version) {
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
