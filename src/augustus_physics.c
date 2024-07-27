#include "augustus_physics.h"

#include "augustus_world.h"

#include <math.h>

void SAT_projection(vec2s vertices[], u32 vertices_len, vec2s axis, f32* min, f32* max) {
    *min = glms_vec2_dot(vertices[0], axis);
    *max = *min;
    
    for(u32 i = 1; i < vertices_len; i++) {
        f32 projection = glms_vec2_dot(vertices[i], axis);
        *min = fmin(*min, projection);
        *max = fmax(*max, projection);
    }
}

f32 Signed2DTriArea(vec2s a, vec2s b, vec2s c) {
    return (a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x);
}

bool LineVsLine(vec2s a, vec2s b, vec2s c, vec2s d, f32* t, vec2s* p, vec2s* normal) {
    vec2s dir = glms_vec2_sub(b, a);

    f32 a1 = Signed2DTriArea(a, b, d);
    f32 a2 = Signed2DTriArea(a, b, c);

    if(a1 * a2 < 0.0f) {
        f32 a3 = Signed2DTriArea(c, d, a);
        f32 a4 = a3 + a2 - a1;

        if(a3 * a4 < 0.0f) {
            *t = a3 / (a3 - a4);
            *p = glms_vec2_mul(glms_vec2_adds(a, *t), glms_vec2_sub(b, a));

            vec2s delta = glms_vec2_sub(d, c);

            vec2s norm = { -delta.y, delta.x };

            if(glms_vec2_dot(dir, norm) > 0) {
                *normal = (vec2s) { delta.y, -delta.x };
            }
            else {
                *normal = norm;
            }

            return true;
        }
    }

    return false;
}

vec2s ClosestPointToLine(vec2s a, vec2s b, vec2s point) {
    vec2s ab = glms_vec2_sub(b, a);

    f32 t = glms_vec2_dot(glms_vec2_sub(point, a), ab);
    t /= glms_vec2_dot(ab, ab);

    if(t < 0.0f) t = 0.0f;
    if(t > 1.0f) t = 1.0f;

    return glms_vec2_add(a, glms_vec2_scale(ab, t));
}

bool AABBvsAABB(vec2s min1, vec2s max1, vec2s min2, vec2s max2) {
    if (max1.x < min2.x || min1.x > max2.x) return false;
    if (max1.y < min2.y || min1.y > max2.y) return false;

    return true;
}
