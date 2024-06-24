#include "augustus_physics.h"

#include "box2d/box2d.h"
#include "box2d/collision.h"
#include "raymath.h"

#include <math.h>

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

void SAT_projection(Vector2 vertices[], u32 vertices_len, Vector2 axis, f32* min, f32* max) {
    *min = Vector2DotProduct(vertices[0], axis);
    *max = *min;
    
    for(u32 i = 1; i < vertices_len; i++) {
        f32 projection = Vector2DotProduct(vertices[i], axis);
        *min = fmin(*min, projection);
        *max = fmax(*max, projection);
    }
}

f32 Signed2DTriArea(Vector2 a, Vector2 b, Vector2 c) {
    return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
}

bool LineVsLine(Vector2 a, Vector2 b, Vector2 c, Vector2 d, f32* t, Vector2* p) {
    f32 a1 = Signed2DTriArea(a, b, d);
    f32 a2 = Signed2DTriArea(a, b, c);

    if(a1 * a2 < 0.0f) {
        f32 a3 = Signed2DTriArea(c, d, a);
        f32 a4 = a3 + a2 - a1;

        if(a3 * a4 < 0.0f) {

            *t = a3 / (a3 - a4);
            *p = Vector2Multiply(Vector2AddValue(a, *t), Vector2Subtract(b, a));
            return true;
        }
    }

    return false;
}

Vector2 ClosestPointToLine(Vector2 a, Vector2 b, Vector2 point) {
    Vector2 ab = Vector2Subtract(b, a);

    f32 t = Vector2DotProduct(Vector2Subtract(point, a), ab);
    t /= Vector2DotProduct(ab, ab);

    if(t < 0.0f) t = 0.0f;
    if(t > 1.0f) t = 1.0f;

    return Vector2Add(a, Vector2Scale(ab, t));
}
