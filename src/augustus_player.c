#include "augustus_player.h"
#include "augustus_gfx.h"
#include "augustus_window.h"
#include "augustus_world.h"
#include "augustus_physics.h"

#include <stdio.h>

#undef EPSILON
#define EPSILON 0.00001f
#define GRAVITY 30.0f
#define VERTICAL_CAP 20.0f 
#define JUMP_HEIGHT -30.0f

#define SLOW_SPEED 0.5f
#define WALK_SPEED 8.0f
#define CROUCH_SPEED WALK_SPEED
#define RUN_SPEED 15.0f

static bool useCollisions = true;

static void Player_toggle_collisions(void) {
    useCollisions = !useCollisions;
}

static vec2s StandingSize = { 0.9f, 1.9f };
static vec2s CrouchingSize = { 0.9f, 0.9f };

Player Player_make(void) {
    return (Player) {
        .pos = (vec2s) { -2, -2 },
        .vel = glms_vec2_zero(),
        .size = StandingSize,
        .state = PLAYER_STANDING,

        .direction = PLAYER_RIGHT,
        .arm_angle = PLAYER_MIDDLE,

        .max_health = 10,
        .health = 7,

        .has_collision = false,
        .is_grounded = false,

        .sprite = Sprite_make("resources/textures/test.jpg")
    };
}

void Player_free(Player* player) {
    Sprite_free(&player->sprite);
}

static bool Player_collision(Player* player, vec2s* min, vec2s* max, TileType* type) {
    Room* room = World_get(&world);

    vec2s checkmin = vec2s_tile(PLAYER_MIN(*player));
    vec2s checkmax = vec2s_tile(glms_vec2_adds(PLAYER_MAX(*player), 1));

    // check only in the tiles that the player could be
    // speeds up checks
    // TODO: Add world bounds checking

    for(i64 x = imax(checkmin.x, 0); x < imin(checkmax.x, room->w); x++) {
        for(i64 y = imax(checkmin.y, 0); y < imin(checkmax.y, room->h); y++) {
            Tile* tile = Room_at(room, x, y);

            if(!tile) continue;
            if(tile->type == TILE_None) continue;

            vec2s tilemin = (vec2s) { x, y };
            vec2s tilemax = (vec2s) { x + 1, y + 1 };

            if(AABBvsAABB(PLAYER_MIN(*player), PLAYER_MAX(*player), tilemin, tilemax)) {
                *min = tilemin;
                *max = tilemax;
                *type = tile->type;
                return true;
            }
        }
    }

    return false;
}

static void Player_move_x(Player* player, f32 velX) {
    vec2s tilemin, tilemax;
    TileType tiletype;

    vec2s min, max;

    player->vel.x = velX;
    player->pos.x += player->vel.x;

    if(player->vel.x > 0) {
        player->direction = PLAYER_RIGHT;
    }
    else if(player->vel.x < 0) {
        player->direction = PLAYER_LEFT;
    }

    if(Player_collision(player, &tilemin, &tilemax, &tiletype)) {
        player->has_collision = true;

        min = PLAYER_MIN(*player);
        max = PLAYER_MAX(*player);

        f32 delta = 0;
        if(player->vel.x > 0) {
            delta = tilemin.x - max.x - EPSILON;
        }
        else if(player->vel.x < 0) {
            delta = tilemax.x - min.x + EPSILON;
        }

        player->pos.x += delta;
    }
}

static void Player_move_y(Player* player, f32 velY) {
    vec2s tilemin, tilemax;
    TileType tiletype;

    vec2s min, max;

    player->vel.y += velY;
    player->vel.y = clamp(player->vel.y, -VERTICAL_CAP, VERTICAL_CAP);

    player->pos.y += player->vel.y * Window_DeltaTime();

    if(Player_collision(player, &tilemin, &tilemax, &tiletype)) {
        player->has_collision = true;

        min = PLAYER_MIN(*player);
        max = PLAYER_MAX(*player);

        f32 delta = 0;
        if(player->vel.y > 0) {
            player->is_grounded = true;
            player->vel.y = 0.001f;

            delta = tilemin.y - max.y - EPSILON;
        }
        else {
            player->vel.y = 0;
            delta = tilemax.y - min.y + EPSILON;
        }

        player->pos.y += delta;
    }
}

static void Player_shoot(Player* player) {
    vec2s bullet_dir = { 0, 0 };

    switch(player->direction) {
        case PLAYER_LEFT:
            bullet_dir.x = -1;
            break;
        case PLAYER_RIGHT:
            bullet_dir.x = 1;
            break;
    }

    switch(player->arm_angle) {
        case PLAYER_MIDDLE:
            bullet_dir.y = 0;
            break;
        case PLAYER_UP:
            bullet_dir.y = -1;
            break;
        case PLAYER_DOWN:
            bullet_dir.y = 1;
            break;
    }

    bullet_dir = glms_vec2_normalize(bullet_dir);

    World_spawn_bullet(&world, (Bullet) {
        .pos = PLAYER_CENTRE(*player),
        .dir = bullet_dir,
        .size = (vec2s) { 0.5f, 0.2f },
        .speed = 30.0f,
        .from_player = true,
    });
}

void Player_update(Player* player) {
    player->is_grounded = false;

    f32 xdir = 0;
    if(Window_KeyHeld(SDL_SCANCODE_A) || Window_KeyHeld(SDL_SCANCODE_LEFT)) xdir -= 1;
    if(Window_KeyHeld(SDL_SCANCODE_D) || Window_KeyHeld(SDL_SCANCODE_RIGHT)) xdir += 1;

    f32 speed = WALK_SPEED;

    if(Window_KeyHeld(SDL_SCANCODE_LSHIFT)) {
        speed = RUN_SPEED;
    }

    if(Window_KeyHeld(SDL_SCANCODE_LCTRL)) {
        speed = SLOW_SPEED;
    }

    player->arm_angle = PLAYER_MIDDLE;
    if(Window_KeyHeld(SDL_SCANCODE_Q)) {
        player->arm_angle = PLAYER_UP;
    }
    if(Window_KeyHeld(SDL_SCANCODE_E)) {
        player->arm_angle = PLAYER_DOWN;
    }

    if(Window_KeyHeld(SDL_SCANCODE_Z)) {
        Player_shoot(player);
    }

    if(Window_KeyDown(SDL_SCANCODE_G)) Player_toggle_collisions();

    if((Window_KeyDown(SDL_SCANCODE_S) || Window_KeyDown(SDL_SCANCODE_DOWN))) {
        player->state = PLAYER_CROUCHING;
    }
    if(Window_KeyDown(SDL_SCANCODE_W) || Window_KeyDown(SDL_SCANCODE_UP)) {
        vec2s tilemin, tilemax;
        TileType tiletype;
        player->state = PLAYER_STANDING;
        player->size = StandingSize;
        if(Player_collision(player, &tilemin, &tilemax, &tiletype)) {
            player->state = PLAYER_CROUCHING;
        }
    }

    switch(player->state) {
        case PLAYER_STANDING:
            player->size = StandingSize;
            break;
        case PLAYER_CROUCHING:
            player->size = CrouchingSize;
            speed = CROUCH_SPEED;
            break;
    }

    player->has_collision = false;
    if(useCollisions) {
        Player_move_x(player, xdir * speed * Window_DeltaTime() );
        Player_move_y(player, GRAVITY * Window_DeltaTime());

        if(player->is_grounded) {
            if(Window_KeyDown(SDL_SCANCODE_SPACE)) {
                printf("JUMP");
                player->vel.y = JUMP_HEIGHT;
            }
        }
        else {
            player->state = PLAYER_STANDING;
            player->size = StandingSize;
        }

    }
    else {
        f32 ydir = 0;
        if(Window_KeyHeld(SDL_SCANCODE_W) || Window_KeyHeld(SDL_SCANCODE_UP)) ydir -= 1;
        if(Window_KeyHeld(SDL_SCANCODE_S) || Window_KeyHeld(SDL_SCANCODE_DOWN)) ydir += 1;

        player->vel.x = xdir * Window_DeltaTime() * speed;
        player->vel.y = ydir * Window_DeltaTime() * speed;

        player->pos = glms_vec2_add(player->pos, player->vel);
    }
}

void Player_draw(Player* player) {
    player->sprite.pos = (vec3s) {{
        player->pos.x,
        player->pos.y - player->size.y,
        0.0f
    }};
    player->sprite.size = player->size;
    
    Sprite_draw(&player->sprite);
}
