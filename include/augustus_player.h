#ifndef AUGUSTUS_PLAYER_H
#define AUGUSTUS_PLAYER_H

#include "augustus_common.h"

typedef enum {
    PLAYER_STANDING,
    PLAYER_CROUCHING,
} PlayerState;

typedef struct {
    f32 x, y;
    u32 state;
} Player;

Player Player_make(void);
void Player_free(Player* player);

void Player_update(Player* player);
void Player_draw(Player* player);

#endif//AUGUSTUS_PLAYER_H
