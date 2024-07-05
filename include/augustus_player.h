#ifndef AUGUSTUS_PLAYER_H
#define AUGUSTUS_PLAYER_H

#include "augustus_common.h"
#include "raylib.h"

typedef struct {
    Vector2 pos, dir, size;
    f32 speed;
} Bullet;

typedef enum {
    PLAYER_STANDING,
    PLAYER_CROUCHING,
} PlayerState;

typedef enum {
    PLAYER_LEFT,
    PLAYER_RIGHT,
} PlayerDirection;

typedef enum {
    PLAYER_MIDDLE,
    PLAYER_UP,
    PLAYER_DOWN,
} PlayerArmAngle;

typedef struct {
    Vector2 pos, vel, size;
    u32 state;
    
    u8 direction, arm_angle;

    bool has_collision;

    bool is_grounded;

    Bullet* bullets;
    u32 bullets_len, bullets_allocated;
} Player;

#define PLAYER_MIN(p) ((Vector2) { (p).pos.x, (p).pos.y - (p).size.y })
#define PLAYER_MAX(p) ((Vector2) { (p).pos.x + (p).size.x, (p).pos.y })
#define PLAYER_CENTRE(p) ((Vector2) { PLAYER_MIN((p)).x + ((p).size.x / 2.0f), PLAYER_MIN((p)).y + ((p).size.y / 2.0f) })

Player Player_make(void);
void Player_free(Player* player);

void Player_update(Player* player);
void Player_draw(Player* player);

#endif//AUGUSTUS_PLAYER_H
