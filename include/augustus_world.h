#ifndef AUGUSTUS_WORLD_H
#define AUGUSTUS_WORLD_H

#include "augustus_common.h"
#include "augustus_enemies.h"

#include <raylib.h>

Vector2 Vector2_tile(Vector2 v);

#define FOR_TILE_TYPES(DO)\
    DO(None)\
    DO(Solid)\
    DO(Spike)

typedef enum {
#define ENUM(x) TILE_##x,
    FOR_TILE_TYPES(ENUM)
#undef ENUM
} TileType;

typedef struct {
    TileType type;
    u64 sprite_index;
} Tile;

#define ROOM_NAME_LEN 24

#define DEFAULT_ROOM_WIDTH 40
#define DEFAULT_ROOM_HEIGHT 30

typedef struct {
    char name[24];
    u64 w, h;
    Tile* data;

    Enemy* enemies;
    u32 enemies_len;
} Room;

Room Room_make(u64 w, u64 h);
void Room_free(Room* room);

void Room_resize(Room* room, u64 w, u64 h);

Tile* Room_at(Room* room, u64 x, u64 y);

void Room_draw(Room* room);

#define WORLD_NAME_LEN 24

typedef struct {
    Room* rooms;
    u64 rooms_len;

    u64 current_room;
} World;

extern World world;

World World_make(void);
void World_free(World* world);

void World_draw(World world);

u64 World_new_room(World* world);
void World_remove_room(World* world, u64 idx);

Room* World_get(World* world);

void World_write_to_file(World* world, char name[WORLD_NAME_LEN]);
bool World_read_from_file(World* world, char name[WORLD_NAME_LEN]);

#endif//AUGUSTUS_WORLD_H
