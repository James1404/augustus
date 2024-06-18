#include "augustus_gfx.h"
#include "augustus_hash_functions.h"
#include <stdlib.h>

static void AnimationMap_realloc(AnimationMap* map) {
    if(!map->buckets) {
        map->buckets = calloc(map->allocated, sizeof(Animation));
        return;
    }

    map->buckets = realloc(map->buckets, map->allocated * sizeof(Animation));
}

#define MAX_LOADFACTOR 0.75f
static f32 AnimationMap_loadfactor(AnimationMap* map) {
    return (f32)map->length / (f32)map->allocated;
}

static AnimationMapEntry* AnimationMap_find_entry(AnimationMap* map, Hash hash) {
    Hash idx = hash % map->allocated;

    AnimationMapEntry* entry = map->buckets + idx;

    while(entry->next) {
        if(entry->hash == hash) break;
        entry = entry->next;
    }

    return entry;
}

AnimationMap AnimationMap_make(void) {
    return (AnimationMap) {
        .buckets = NULL,
        .allocated = 100,
        .length = 0,
    };
}

void AnimationMap_free(AnimationMap* map) {
    if(map->buckets) free(map->buckets);
    *map = AnimationMap_make();
}

void AnimationMap_set(AnimationMap* map, char* key, Animation value) {
    if(!map->buckets) {
        AnimationMap_realloc(map);
    }

    if(AnimationMap_loadfactor(map) >= MAX_LOADFACTOR) {
        AnimationMap_realloc(map);
    }

    Hash hash = hash_string(key);

    AnimationMapEntry* entry = AnimationMap_find_entry(map, hash);
    if(entry->hash != hash) map->length++;

    *entry = (AnimationMapEntry) {
        .hash = hash,
        .key = key,
        .value = value,
        .next = NULL
    };
}

Animation AnimationMap_get(AnimationMap* map, char* key) {
    Hash idx = hash_string(key) % map->allocated;
    return map->buckets[idx].value;
}

bool AnimationMap_exists(AnimationMap* map, char* key) {
    Hash hash = hash_string(key);

    AnimationMapEntry* entry = AnimationMap_find_entry(map, hash);

    return entry->hash == hash;
}

Sprite Sprite_make() {
    return (Sprite) {
        .animations = AnimationMap_make()
    };
}

void Sprite_free(Sprite* sprite) {
    AnimationMap_free(&sprite->animations);
}
