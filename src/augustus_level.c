#include "augustus_level.h"

#include "raylib.h"
#include "raymath.h"

#include <stdio.h>
#include <stdlib.h>
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

void Segment_draw(Segment* segment, Color color) {
    for(u32 j = 0; j < segment->len; j++) {
        DrawCircleV(segment->vertices[j], 0.5, color);
    }

    if(segment->len >= 2) {
        for(u32 j = 0; j < segment->len; j++) {
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

Level Level_make(void) {
    return (Level) {
        .segments = NULL,
        .segments_len = 0,
        .doors = NULL,
        .doors_len = 0,
    };
}

void Level_free(Level* level) {
    for(u32 i = 0; i < level->segments_len; i++) {
        Segment* segment = level->segments + i;
        Segment_free(segment);
    }

    if(level->segments) free(level->segments); if(level->doors) free(level->doors);

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

void Level_write_to_file(Level* level, const char* filename) {
    FILE* file;

    file = fopen(filename, "wb");

    fwrite(&format_version, sizeof(format_version), 1, file);

    fwrite(&level->segments_len, sizeof(level->segments_len), 1, file);

    for(u32 i = 0; i < level->segments_len; i++) {
        Segment* segment = level->segments + i;
        fwrite(&segment->len, sizeof(segment->len), 1, file);
        fwrite(segment->vertices, sizeof(segment->vertices[0]), segment->len, file);
    }

    fclose(file);
}

static bool Level_read_V1(Level* level, FILE* file) {
    fread(&level->segments_len, sizeof(level->segments_len), 1, file);
    level->segments = malloc(sizeof(Segment) * level->segments_len);

    for(u32 i = 0; i < level->segments_len; i++) {
        Segment* segment = level->segments + i;
        fread(&segment->len, sizeof(segment->len), 1, file);
        segment->vertices = malloc(sizeof(segment->vertices[0])*segment->len);
        fread(segment->vertices, sizeof(segment->vertices[0]), segment->len, file);
    }

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
