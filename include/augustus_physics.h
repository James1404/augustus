#ifndef AUGUSTUS_PHYSICS_H
#define AUGUSTUS_PHYSICS_H

#include "augustus_common.h"
#include "augustus_math.h"

void SAT_projection(vec2s vertices[], u32 vertices_len, vec2s axis, f32* min, f32* max);

f32 Signed2DTriArea(vec2s a, vec2s b, vec2s c);

bool LineVsLine(vec2s a, vec2s b, vec2s c, vec2s d, f32* t, vec2s* p, vec2s* normal);

bool LineVsAABB(vec2s pos, vec2s size, vec2s l1, vec2s l2);

vec2s ClosestPointToLine(vec2s a, vec2s b, vec2s point);

bool AABBvsAABB(vec2s min1, vec2s max1, vec2s min2, vec2s max2);

#endif//AUGUSTUS_PHYSICS_H
