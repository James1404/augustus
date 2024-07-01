#ifndef AUGUSTUS_GFX_H
#define AUGUSTUS_GFX_H

#include "augustus_common.h"
#include "augustus_hash_functions.h"

typedef struct {
    i32 x, y, width, height;
} Frame;

typedef struct {
    Frame* frames;
    u32 len;

    f32 fps;
} Animation;

Animation Animation_make(void);
void Animation_free(Animation* animation);

void Animation_push(Animation* animation, Frame frame);

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

AnimationMap AnimationMap_load(const char* filepath);

typedef struct {
    AnimationMap animations;
} Sprite;

Sprite Sprite_make(void);
void Sprite_free(Sprite* sprite);

#endif//AUGUSTUS_GFX_H
