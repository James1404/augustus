#ifndef AUGUSTUS_ENEMIES_H
#define AUGUSTUS_ENEMIES_H

#include "augustus_common.h"
#include "raylib.h"

typedef enum {
    ENEMY_BAT,
} EnemyType;

typedef struct {
    Vector2 pos;

    u32 type;

    u32 health;

    bool alive;
} Enemy;

Enemy Enemy_make(u32 type, Vector2 pos);
void Enemy_free(Enemy* enemy);

void Enemy_update(Enemy* enemy);
void Enemy_draw(Enemy* enemy);

#endif//AUGUSTUS_ENEMIES_H
