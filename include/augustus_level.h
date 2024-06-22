#ifndef AUGUSTUS_LEVEL_H
#define AUGUSTUS_LEVEL_H

#include "augustus_common.h"

#include <raylib.h>

typedef struct {
    Vector2* vertices;
    u64 len;
} Segment;

Segment Segment_make(void);
void Segment_free(Segment* segment);

void Segment_add_vertex(Segment* segment, Vector2 pos);
void Segment_draw(Segment* segment, Color color);

typedef struct {
    u32 to;

    i32 x, y;
} Door;

typedef struct {
    Image image;

    Vector2 pos, scl;
    f32 rot;
} Splat;

#define LEVEL_NAME_LEN 24

typedef struct {
    Segment* segments;
    u64 segments_len;

    Door* doors;
    u32 doors_len;
} Level;

extern Level level;

Level Level_make(void);
void Level_free(Level* level);

void Level_draw(Level level);

void Level_new_segment(Level* level, Segment segment);

void Level_write_to_file(Level* level, const char* filename);
bool Level_read_from_file(Level* level, const char* filename);

#endif//AUGUSTUS_LEVEL_H
