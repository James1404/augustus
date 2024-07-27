#include "augustus_enemies.h"

#include "augustus_window.h"
#include "augustus_world.h"
#include "raymath.h"

Enemy Enemy_make(u32 type, vec2s pos) {
    u32 health = 10;

    return (Enemy) {
        .pos = pos,
        .size = (vec2s) { 1, 1 },
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
            vec2s dir = glms_vec2_sub(PLAYER_CENTRE(world.player), enemy->pos);
            dir = glms_vec2_normalize(dir);
            enemy->pos = glms_vec2_add(enemy->pos, glms_vec2_scale(dir, Window_DeltaTime() * BAT_SPEED));

            break;
        }
        case ENEMY_Turret: {
            vec2s dir = glms_vec2_sub(PLAYER_CENTRE(world.player), enemy->pos);
            dir = glms_vec2_normalize(dir);
            enemy->pos = glms_vec2_add(enemy->pos, glms_vec2_scale(dir, Window_DeltaTime() * BAT_SPEED));

            break;
        }
    }
}

void Enemy_draw(Enemy* enemy) {
    if(!enemy->alive) return;

    switch(enemy->type) {
        case ENEMY_Bat: {
            // todo
            //DrawRectangleV(enemy->pos, enemy->size, GRAY);
            break;
        }
    }
}
