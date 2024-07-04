#include "augustus_enemies.h"

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

void Enemy_update(Enemy* enemy) {
    switch(enemy->type) {
        case ENEMY_Bat: {
            Vector2 dir = Vector2Zero();
            enemy->pos = Vector2Add(enemy->pos, dir);

            break;
        }
    }
}

void Enemy_draw(Enemy* enemy) {
    switch(enemy->type) {
        case ENEMY_Bat: {
            DrawRectangleV(enemy->pos, enemy->size, GRAY);
            break;
        }
    }
}
