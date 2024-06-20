#include "augustus_physics.h"

#include "box2d/box2d.h"
#include "box2d/collision.h"
#include "raylib.h"

static b2WorldId world;

void Physics_init(void) {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, -9.8f};

    world = b2CreateWorld(&worldDef);
}

void Physics_free(void) {
    b2DestroyWorld(world);
}

static f32 accumulator = 0;
static f32 timestep = 1.0f / 60.0f;

void Physics_sim(void) {
    accumulator += GetFrameTime();

    while(accumulator >= timestep) {
        b2World_Step(world, timestep, 4);
        accumulator -= timestep;
    }
}

Rigidbody Rigidbody_make(f32 w, f32 h) {
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2) { 0, 0 };

    b2BodyId body = b2CreateBody(world, &bodyDef);

    b2Polygon shape = b2MakeBox(w / 2.0f, h / 2.0f);

    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1;
    shapeDef.friction = 0.3f;

    b2CreatePolygonShape(body, &shapeDef, &shape);

    return (Rigidbody) { body, w, h };
}

void Rigidbody_free(Rigidbody rb) {
    b2DestroyBody(rb.body);
}

void Rigidbody_draw(Rigidbody rb) {
    b2Vec2 pos = Rigidbody_pos(rb);
    DrawRectangle(pos.x, pos.y, rb.w, rb.h, DARKPURPLE);
}

b2Vec2 Rigidbody_pos(Rigidbody rb) {
    return b2Body_GetPosition(rb.body);
}

f32 Rigidbody_rotation(Rigidbody rb) {
    return b2Body_GetAngle(rb.body);
}
