#ifndef AUGUSTUS_PHYSICS_H
#define AUGUSTUS_PHYSICS_H

#include "augustus_common.h"

#include "box2d/id.h"
#include "box2d/math_functions.h"
#include "box2d/collision.h"
#include "raylib.h"

extern b2WorldId world;

void Physics_init(void);
void Physics_free(void);
void Physics_sim(void);
void Physics_from_level(void);

typedef struct {
    b2BodyId body;
    f32 w, h;
} Rigidbody;

Rigidbody Rigidbody_make(f32 w, f32 h);
void Rigidbody_free(Rigidbody rb);

void Rigidbody_draw(Rigidbody rb);

b2Vec2 Rigidbody_pos(Rigidbody rb);
f32 Rigidbody_rotation(Rigidbody rb);

void SAT_projection(Vector2 vertices[], u32 vertices_len, Vector2 axis, f32* min, f32* max);

f32 Signed2DTriArea(Vector2 a, Vector2 b, Vector2 c);

bool LineVsLine(Vector2 a, Vector2 b, Vector2 c, Vector2 d, f32* t, Vector2* p, Vector2* normal);

bool LineVsAABB(Vector2 pos, Vector2 size, Vector2 l1, Vector2 l2);

Vector2 ClosestPointToLine(Vector2 a, Vector2 b, Vector2 point);

bool AABBvsAABB(Vector2 min1, Vector2 max1, Vector2 min2, Vector2 max2);

#endif//AUGUSTUS_PHYSICS_H
