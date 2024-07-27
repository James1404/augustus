#ifndef AUGUSTUS_CAMERA_H
#define AUGUSTUS_CAMERA_H

#include "augustus_common.h"
#include "augustus_math.h"

typedef struct {
    vec2s target, offset;
    f32 zoom;
} Camera;

vec2s ScreenToWorld(vec2s pos);

#endif//AUGUSTUS_CAMERA_H
