#include "augustus_common.h"
#include "augustus_gfx.h"
#include "augustus_physics.h"
#include "augustus_player.h"
#include "augustus_world.h"
#include "augustus_string.h"

#include <math.h>
#include <raylib.h>
#include <raymath.h>

#include <stdio.h>
#include <string.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include <rlImGui.h>

#define MIN_ZOOM 0.125f
#define MAX_ZOOM 1000.0f

static const int screenWidth = 1280, screenHeight = 720;

typedef enum {
    GAMESTATE_GAMEPLAY,
    GAMESTATE_EDITOR
} GameState;

#define FOR_EDITORTOOLS(DO)\
    DO(None)\
    DO(Brush)\
    DO(Enemies)\

typedef enum {
#define ENUM(x) EDITORTOOL_##x,
    FOR_EDITORTOOLS(ENUM)
#undef ENUM
} EditorTool;

static GameState state = GAMESTATE_EDITOR;
static EditorTool tool = EDITORTOOL_None;
static char worldName[WORLD_NAME_LEN] = "";

static i64 roomW = 0, roomH = 0;

static TileType brushType = TILE_None;
static i32 brushSize = 1;

static EnemyType enemyBrushType = ENEMY_Bat;

i32 main(void) {
    String window_name = STR("Hey, window");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, window_name.raw);

    rlImGuiSetup(true);

    ImGuiIO* io = igGetIO();

    Physics_init();

    Camera2D camera = { 0 };
    camera.zoom = 20;

    world = World_make();

    AnimationMap_load("resources/player.json");

    World_new_room(&world);

    Rigidbody rb = Rigidbody_make(1, 1);

    while(!WindowShouldClose()) {
        Vector2 mouseWorldPosition = GetScreenToWorld2D(GetMousePosition(), camera);

        if(!io->WantCaptureMouse || state == GAMESTATE_GAMEPLAY) {
            if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 delta = GetMouseDelta();
                delta = Vector2Scale(delta, -1.0f/camera.zoom);
                camera.target = Vector2Add(camera.target, delta);
            }

            f32 wheel = GetMouseWheelMove();
            if(wheel != 0) {
                camera.offset = GetMousePosition();
                camera.target = mouseWorldPosition; 

                float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
                if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
                camera.zoom = Clamp(camera.zoom*scaleFactor, MIN_ZOOM, MAX_ZOOM);
            }
        }

        switch(state) {
            case GAMESTATE_GAMEPLAY: {
                World_update(&world);
                Physics_sim();
            } break;
            case GAMESTATE_EDITOR: {
            } break;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);

        Rigidbody_draw(rb);

        World_draw(world);

        if(state == GAMESTATE_EDITOR) {
            Room* room = World_get(&world);
            DrawRectangleLinesEx((Rectangle) { 0, 0, room->w, room->h }, 0.1f, GREEN);

            switch(tool) {
                case EDITORTOOL_None:
                    break;
                case EDITORTOOL_Brush: {
                    Vector2 size = { brushSize, brushSize };
                    Vector2 cursor = Vector2_tile(Vector2Subtract(mouseWorldPosition, Vector2Scale(size, 0.5f)));
                    DrawRectangleV(cursor, size, (Color) { 255, 255, 255, 100 });

                    if(!io->WantCaptureMouse && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                        for(i64 x = cursor.x; x < cursor.x + size.x; x++) {
                            for(i64 y = cursor.y; y < cursor.y + size.y; y++) {
                                Tile* tile = Room_at(room, x, y);

                                if((x < room->w && x >= 0) && (y < room->h && y >= 0)) {
                                    tile->type = brushType;
                                }
                            }
                        }
                    }
                } break;
                case EDITORTOOL_Enemies: {
                    Vector2 cursor = mouseWorldPosition;
                    DrawRectangleV(cursor, Vector2One(), (Color) { 255, 255, 255, 100 });

                    if(!io->WantCaptureMouse && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                        Room* room = World_get(&world);
                        Room_add_enemy(room, Enemy_make(enemyBrushType, cursor));
                    }
                } break;
                default: {} break;
            }
        }

        EndMode2D();

        if(IsKeyPressed(KEY_TAB) && !io->WantCaptureKeyboard) {
            switch(state) {
                case GAMESTATE_GAMEPLAY:
                    state = GAMESTATE_EDITOR;
                    break;
                case GAMESTATE_EDITOR:
                    state = GAMESTATE_GAMEPLAY;
                    break;
                default: break;
            }
        }

        if(state == GAMESTATE_EDITOR) {
            rlImGuiBegin();

            if(igBegin("Settings", NULL, 0)) {
                igInputText("##World Name", worldName, WORLD_NAME_LEN, 0, NULL, NULL);

                igSameLine(0, -1);

                if(igButton("Save", (ImVec2) {0,0})) {
                    World_write_to_file(&world, worldName);
                }

                igSameLine(0, -1);

                if(igButton("Load", (ImVec2) {0,0})) {
                    if(!World_read_from_file(&world, worldName)) {
                        printf("Failed to load world from '%s.bin'", worldName);
                    }

                    Room* room = World_get(&world);

                    roomW = room->w;
                    roomH = room->h;
                }

                igSeparator();

                if(igButton("New Room", (ImVec2) {0,0})) {
                    world.current_room = World_new_room(&world);
                }

                igSameLine(0, -1);

                if(igButton("Delete Current Room", (ImVec2) {0,0})) {
                    World_remove_room(&world, world.current_room);
                }

                igSeparator();

                if(igBeginListBox("Rooms", (ImVec2){0,0})) {
                    for(u32 i = 0; i < world.rooms_len; i++) {
                        igPushID_Int(i);
                        bool selected = (world.current_room == i);
                        if(igSelectable_Bool(world.rooms[i].name, selected, 0, (ImVec2){0,0})) {
                            world.current_room = i;

                            Room* room = World_get(&world);

                            roomW = room->w;
                            roomH = room->h;
                        }

                        if (selected) igSetItemDefaultFocus();
                        igPopID();
                    }

                    igEndListBox();
                }

                igEnd();
            }

            if(igBegin("Room", NULL, 0)) {
                Room* room = World_get(&world);
                igInputText("##Room Name", room->name, ROOM_NAME_LEN, 0, NULL, NULL);

                igDragInt("Width", &roomW, 1.0f, 0, 10000, "%lu", 0);
                igDragInt("Height", &roomH, 1.0f, 0, 10000, "%lu", 0);

                if(igButton("Set Size", (ImVec2) {0,0})) {
                    Room_resize(room, roomW, roomH);
                }

                igEnd();
            }

            if(igBegin("Tools", NULL, 0)) {
#define BUTTON(x) if(igButton(#x, (ImVec2) {0,0})) tool = EDITORTOOL_##x;
                FOR_EDITORTOOLS(BUTTON)
#undef BUTTON

                igSeparator();

                switch(tool) {
                    case EDITORTOOL_None:
                        break;
                    case EDITORTOOL_Brush:
#define BUTTON(x) if(igButton(#x, (ImVec2) {0,0})) { brushType = TILE_##x; }
                        FOR_TILE_TYPES(BUTTON)
#undef BUTTON

                        igDragInt("Size", &brushSize, 0.2f, 0, 10000, "%d", 0);
                        break;
                    case EDITORTOOL_Enemies:
#define BUTTON(x) if(igButton(#x, (ImVec2) {0,0})) { enemyBrushType = ENEMY_##x; }
                        FOR_ENEMY_TYPES(BUTTON)
#undef BUTTON
                        break;
                    default: {} break;
                }

                igEnd();
            }

            rlImGuiEnd();
        }

        EndDrawing();
    }

    Rigidbody_free(rb);

    World_free(&world);

    Physics_free();

    rlImGuiShutdown();
    CloseWindow();
}
