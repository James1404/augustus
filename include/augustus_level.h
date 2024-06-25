#ifndef AUGUSTUS_LEVEL_H
#define AUGUSTUS_LEVEL_H

#include "augustus_common.h"
#include "augustus_enemies.h"
#include "augustus_physics.h"
#include "box2d/id.h"

#include <raylib.h>

typedef struct {
    Vector2* vertices;
    u64 len;

    bool body_created;
    b2BodyId body;
    b2ChainId shape;
} Segment;

Segment Segment_make(void);
void Segment_free(Segment* segment);

void Segment_add_vertex(Segment* segment, Vector2 pos);
void Segment_delete_vertex(Segment* segment, u64 idx);
void Segment_insert(Segment* segment, Vector2 elem, u64 at);
void Segment_draw(Segment* segment, Color color);

void Segment_update_body(Segment* segment);

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

typedef struct {
    u64 version;
    char author[5];
    char name[LEVEL_NAME_LEN];
} LevelHeader;

extern Level level;

Level Level_make(void);
void Level_free(Level* level);

void Level_draw(Level level);

void Level_new_segment(Level* level, Segment segment);
void Level_remove_segment(Level* level, u64 idx);

void Level_write_to_file(Level* level, char name[LEVEL_NAME_LEN]);
bool Level_read_from_file(Level* level, char name[LEVEL_NAME_LEN]);

#endif//AUGUSTUS_LEVEL_H
