#ifndef AUGUSTUS_PLAYER_H
#define AUGUSTUS_PLAYER_H

#include "augustus_common.h"
#include "raylib.h"

typedef enum {
    PLAYER_STANDING,
    PLAYER_CROUCHING,
} PlayerState;

typedef struct {
    Vector2 pos;
    f32 height;
    u32 state;
    bool has_collision;
} Player;

Player Player_make(void);
void Player_free(Player* player);

void Player_update(Player* player);
void Player_draw(Player* player);

bool Player_is_grounded(Player player);

void Player_toggle_collisions(void);

#endif//AUGUSTUS_PLAYER_H
