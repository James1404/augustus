#include "augustus_world.h"

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static u32 format_version = 1;

static bool World_read_V1(World* world, FILE* file) {
    fread(&world->rooms_len, sizeof(world->rooms_len), 1, file);
    world->rooms = malloc(sizeof(world->rooms[0]) * world->rooms_len);

    for(u32 i = 0; i < world->rooms_len; i++) {
        Room* room = world->rooms + i;
        *room = (Room) { 0 };

        fread(&room->name, sizeof(room->name[0]), ROOM_NAME_LEN, file);
        fread(&room->w, sizeof(room->w), 1, file);
        fread(&room->h, sizeof(room->h), 1, file);
        room->data = malloc(sizeof(room->data[0])*room->w*room->h);
        fread(room->data, sizeof(room->data[0]), room->w * room->h, file);

        fread(&room->enemies_len, sizeof(room->enemies_len), 1, file);
        room->enemies = malloc(sizeof(room->enemies[0]) * room->enemies_len);
        fread(room->enemies, sizeof(room->enemies[0]), room->enemies_len, file);
    }

    return true;
}

void World_write_to_file(World* world, char name[WORLD_NAME_LEN]) {
    FILE* file;

    file = fopen(TextFormat("resources/levels/%s.bin", name), "wb");

    fwrite(&world->rooms_len, sizeof(world->rooms_len), 1, file);
    for(u32 i = 0; i < world->rooms_len; i++) {
        Room* room = world->rooms + i;
        fwrite(&room->name, sizeof(room->name[0]), ROOM_NAME_LEN, file);
        fwrite(&room->w, sizeof(room->w), 1, file);
        fwrite(&room->h, sizeof(room->h), 1, file);
        fwrite(room->data, sizeof(room->data[0]), room->w * room->h, file);

        fwrite(&room->enemies_len, sizeof(room->enemies_len), 1, file);
        fwrite(room->enemies, sizeof(room->enemies[0]), room->enemies_len, file);
    }

    fclose(file);
}

bool World_read_from_file(World* world, char name[WORLD_NAME_LEN]) {
    FILE* file;

    file = fopen(TextFormat("resources/levels/%s.bin", name), "rb");

    bool success = false;

    if(file) {
        World_free(world);

        World_read_V1(world, file);

        success = true;
    }

    fclose(file);

    return success;
}
