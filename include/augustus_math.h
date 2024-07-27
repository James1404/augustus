#ifndef AUGUSTUS_MATH_H
#define AUGUSTUS_MATH_H

#include "augustus_common.h"

#include <cglm/cglm.h>
#include <cglm/struct.h>

i32 imin(i32 a, i32 b);
i32 imax(i32 a, i32 b);

f32 clamp(f32 v, f32 min, f32 max);

#endif//AUGUSTUS_MATH_H
