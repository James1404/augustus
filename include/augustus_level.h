#ifndef AUGUSTUS_LEVEL_H
#define AUGUSTUS_LEVEL_H

#include "augustus_common.h"
#include "augustus_enemies.h"

#include <raylib.h>

typedef struct {
    Vector2* vertices;
    u64 len;

    bool wrap;
} Segment;

Segment Segment_make(void);
void Segment_free(Segment* segment);

void Segment_add_vertex(Segment* segment, Vector2 pos);
void Segment_delete_vertex(Segment* segment, u64 idx);
void Segment_insert(Segment* segment, Vector2 elem, u64 at);
void Segment_draw(Segment* segment, Color color);

typedef struct {
    Texture texture;
    const char* filepath;

    Vector2 pos;
    f32 scl, rot;

    i64 layer;
} Splat;

Splat Splat_make(const char* filepath);
void Splat_free(Splat* splat);

void Splat_draw(Splat* splat);

#define LEVEL_NAME_LEN 24

typedef struct {
    Segment* segments;
    u64 segments_len;

    Splat* splats;
    u64 splats_len;

    Enemy* enemies;
    u64 enemies_len;
} Level;

extern Level level;

Level Level_make(void);
void Level_free(Level* level);

void Level_draw(Level level);

void Level_new_segment(Level* level, Segment segment);
void Level_remove_segment(Level* level, u64 idx);

void Level_write_to_file(Level* level, const char* filename);
bool Level_read_from_file(Level* level, const char* filename);

#endif//AUGUSTUS_LEVEL_H
