#ifndef AUGUSTUS_GFX_H
#define AUGUSTUS_GFX_H

#include "augustus_common.h"
#include "augustus_hash_functions.h"
#include "augustus_math.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <vk_mem_alloc.h>

#include <threads.h>

void GFX_ClearColor(vec3s color);

void GFX_BeginFrame(void);
void GFX_EndFrame(void);

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

    // texture data
    i32 w, h, channels;
    VkFormat format;
    VkDeviceSize vkSize;

    VkImage image;
    VmaAllocation imageAllocation;

    VkImageView imageView;
    VkSampler sampler;

    vec3s pos;
    vec2s size;

    char* texture_filename;
} Sprite;

Sprite Sprite_make(char* filename);
void Sprite_free(Sprite* sprite);
void Sprite_draw(Sprite* sprite);

#endif//AUGUSTUS_GFX_H
