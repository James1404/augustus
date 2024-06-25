#include "augustus_level.h"

#include "raylib.h"
#include "raymath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "box2d/box2d.h"
#include "box2d/types.h"

Level level;

Segment Segment_make(void) {
    return (Segment) {
        .vertices = NULL,
        .len = 0,
        .body_created = false,
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
        for(u32 j = 0; j < segment->len; j++) {
            Vector2 a = segment->vertices[j];
            Vector2 b = segment->vertices[(j + 1) % segment->len];
            Vector2 delta = Vector2Subtract(b, a);
            Vector2 normal = { -delta.y, delta.x };
            Vector2 midpoint = Vector2Scale(Vector2Add(a,b), 0.5);

            DrawLineV(a, b, color);
            DrawCircleV(a, 0.5f, YELLOW);

            DrawLineV(midpoint, Vector2Add(midpoint, Vector2Scale(normal, 0.1f)), RED);
        }
    }
}

void Segment_update_body(Segment* segment) {
    if(segment->body_created) {
        b2DestroyChain(segment->shape);
    }
    else {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = (b2Vec2) { 0, 0 };

        segment->body = b2CreateBody(world, &bodyDef);
    }

    b2ChainDef chainDef = b2DefaultChainDef();
    chainDef.count = segment->len;
    chainDef.points = (b2Vec2*)segment->vertices;
    chainDef.isLoop = true;

    segment->shape = b2CreateChain(segment->body, &chainDef);

    segment->body_created = true;
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
