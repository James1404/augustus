#include "augustus_gfx.h"
#include "augustus_hash_functions.h"
#include "json_tokener.h"

#include <stdlib.h>
#include <stdio.h>

#include <json.h>

Animation Animation_make(void) {
    return (Animation) {
        .frames = NULL,
        .len = 0,
        .fps = 12,
    };
}

void Animation_free(Animation* animation) {
    if(animation->frames) free(animation->frames);
}

void Animation_push(Animation* animation, Frame frame) {
}

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

AnimationMap AnimationMap_load(const char* filepath) {
    AnimationMap map = AnimationMap_make();

    FILE* file = fopen(filepath, "r");

    char* buffer = NULL;
    u64 len = 0;

    if(file) {

        fseek(file, 0, SEEK_END);
        len = ftell(file);
        fseek(file, 0, SEEK_SET);

        buffer = malloc(len);
        fread(buffer, 1, len, file);

        fclose(file);
    }
    else {
        printf("Failed to read file: \"%s\"", filepath);
    }

    if(buffer) {
        json_tokener* tok = json_tokener_new();

        json_object* obj = json_tokener_parse_ex(tok, buffer, len);

        json_object_object_foreach(obj, key, val) {
            u64 array_len = json_object_array_length(val);
            for(u64 i = 0; i < array_len; i++) {
                json_object* frame = json_object_array_get_idx(val, i);
                json_object_object_foreach(frame, key, val) {
                    Frame frame = {0};

                    frame.x = json_object_get_int(json_object_object_get(val, "x"));
                    frame.y = json_object_get_int(json_object_object_get(val, "y"));
                    frame.width = json_object_get_int(json_object_object_get(val, "w"));
                    frame.height = json_object_get_int(json_object_object_get(val, "h"));
                }
            }
        }

        json_tokener_free(tok);
    }

    return map;
}

Sprite Sprite_make(void) {
    return (Sprite) {
        .animations = AnimationMap_make()
    };
}

void Sprite_free(Sprite* sprite) {
    AnimationMap_free(&sprite->animations);
}
