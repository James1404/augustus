#include "augustus_enemies.h"

#include "augustus_world.h"
#include "raymath.h"

Enemy Enemy_make(u32 type, Vector2 pos) {
    u32 health = 10;

    return (Enemy) {
        .pos = pos,
        .size = (Vector2) { 1, 1 },
        .type = type,
        .health = health,
        .alive = true,
    };
}

void Enemy_free(Enemy* enemy) {
}

#define BAT_SPEED 5

void Enemy_update(Enemy* enemy) {
    if(!enemy->alive) return;

    switch(enemy->type) {
        case ENEMY_Bat: {
            Vector2 dir = Vector2Subtract(PLAYER_CENTRE(world.player), enemy->pos);
            dir = Vector2Normalize(dir);
            enemy->pos = Vector2Add(enemy->pos, Vector2Scale(dir, GetFrameTime() * BAT_SPEED));

            break;
        }
    }
}

void Enemy_draw(Enemy* enemy) {
    if(!enemy->alive) return;

    switch(enemy->type) {
        case ENEMY_Bat: {
            DrawRectangleV(enemy->pos, enemy->size, GRAY);
            break;
        }
    }
}
