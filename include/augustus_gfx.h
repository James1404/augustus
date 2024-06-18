#ifndef AUGUSTUS_GFX_H
#define AUGUSTUS_GFX_H

#include "augustus_common.h"
#include "augustus_hash_functions.h"

typedef struct {
    i32 x, y, width, height;
} Frame;

typedef struct {
    Frame* frames;
    u32 allocated, length;

    f32 fps;
} Animation;

typedef struct AnimationMapEntry {
    Hash hash;
    char* key;
    Animation value;

    struct AnimationMapEntry* next;
} AnimationMapEntry;

typedef struct {
    usize allocated, length;
    AnimationMapEntry* buckets;
} AnimationMap;

AnimationMap AnimationMap_make(void);
void AnimationMap_free(AnimationMap* map);

void AnimationMap_set(AnimationMap* map, char* key, Animation value);
Animation AnimationMap_get(AnimationMap* map, char* key);

bool AnimationMap_exists(AnimationMap* map, char* key);

typedef struct {
    AnimationMap animations;
} Sprite;

Sprite Sprite_make();
void Sprite_free(Sprite* sprite);

#endif//AUGUSTUS_GFX_H
