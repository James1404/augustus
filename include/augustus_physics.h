#ifndef AUGUSTUS_PHYSICS_H
#define AUGUSTUS_PHYSICS_H

#include "augustus_common.h"

#include "box2d/id.h"

typedef struct {
    f32 x, y;
    b2BodyId body;
} Rigidbody;

void Physics_init(void);
void Physics_free(void);

#endif//AUGUSTUS_PHYSICS_H
