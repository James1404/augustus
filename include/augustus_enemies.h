#ifndef AUGUSTUS_ENEMIES_H
#define AUGUSTUS_ENEMIES_H

#include "augustus_common.h"
#include "raylib.h"

typedef struct {
    Vector2 pos;
    u32 state;
    u32 type;

    u32 health;

    bool alive;

    void* payload;
    u64 payload_size;
} Enemy;

#endif//AUGUSTUS_ENEMIES_H
