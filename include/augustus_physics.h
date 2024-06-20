#ifndef AUGUSTUS_PHYSICS_H
#define AUGUSTUS_PHYSICS_H

#include "augustus_common.h"

#include "box2d/id.h"
#include "box2d/math_functions.h"

void Physics_init(void);
void Physics_free(void);
void Physics_sim(void);

typedef struct {
    b2BodyId body;
    f32 w, h;
} Rigidbody;

Rigidbody Rigidbody_make(f32 w, f32 h);
void Rigidbody_free(Rigidbody rb);

void Rigidbody_draw(Rigidbody rb);

b2Vec2 Rigidbody_pos(Rigidbody rb);
f32 Rigidbody_rotation(Rigidbody rb);

#endif//AUGUSTUS_PHYSICS_H
