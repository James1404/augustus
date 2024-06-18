#include "augustus_physics.h"

static b2WorldId world;

void Physics_init() {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, -9.8f};

    world = b2CreateWorld(&worldDef);
}

void Physics_free() {
    b2DestroyWorld(world);
}
