#include "augustus_player.h"

Player Player_make(void) {
    return (Player) {
        .x = 0,
        .y = 0,
        .state = PLAYER_STANDING
    };
}

void Player_free(Player* player) {
}

void Player_update(Player* player) {
}

void Player_draw(Player* player) {
}
