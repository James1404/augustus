#include "augustus_common.h"
#include "augustus_physics.h"
#include "augustus_player.h"
#include "augustus_level.h"
#include "augustus_string.h"

#include <raylib.h>
#include <raymath.h>

#include <stdio.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

#include <rlImGui.h>

#define MIN_ZOOM 0.125f
#define MAX_ZOOM 100.0f

static const int screenWidth = 800, screenHeight = 600;

static i32 currentTileType = TILE_NONE;
static Vector2 cursorPos;
#define TILE_STRING(x) #x,
static const char* tileTypes[] = { FOR_TILES(TILE_STRING) };

static i32 newRoomSizeW = 0;
static i32 newRoomSizeH = 0;

static u32 currentRoom = 0;

typedef enum {
    GAMESTATE_GAMEPLAY,
    GAMESTATE_EDITOR
} GameState;

static GameState currentState = GAMESTATE_EDITOR;

i32 main(void) {
    String window_name = STR("Hey, window");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, window_name.raw);

    rlImGuiSetup(true);

    Physics_init();

    Camera2D camera = { 0 };
    camera.zoom = 20;

    Level level = Level_make();
    currentRoom = Level_add_room(&level, 30, 20);

    Player player = Player_make();

    while(!WindowShouldClose()) {
        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f/camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }

        f32 wheel = GetMouseWheelMove();
        if(wheel != 0) {
            Vector2 mouseWorldPosition = GetScreenToWorld2D(GetMousePosition(), camera);

            camera.offset = GetMousePosition();
            camera.target = mouseWorldPosition; 

            float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
            if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
            camera.zoom = Clamp(camera.zoom*scaleFactor, MIN_ZOOM, MAX_ZOOM);
        }

        if(currentState == GAMESTATE_GAMEPLAY) {
            Player_update(&player);
            Physics_sim();
        }

        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);

        Player_draw(&player);

        Room* room = Level_get_room(level, currentRoom);

        if(room) {
            Room_draw(*room);

            if(currentState == GAMESTATE_EDITOR) {
                DrawRectangleLinesEx(
                    (Rectangle) { 0, 0, room->w, room->h },
                    0.1f,
                    GREEN
                );

                cursorPos = Vector2_WorldToTile(GetScreenToWorld2D(GetMousePosition(), camera));

                if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                    if( (cursorPos.x >= 0 && cursorPos.x <= room->w) &&
                        (cursorPos.y >= 0 && cursorPos.y <= room->h)) {
                        Tile* t = Room_get(room, cursorPos.x, cursorPos.y);
                        t->type = currentTileType;
                    }
                }
            }
        }

        if(currentState == GAMESTATE_EDITOR) {
            DrawRectangleLinesEx(
                (Rectangle) { cursorPos.x, cursorPos.y, 1, 1 },
                0.1f,
                RED
            );
        }

        if(IsKeyPressed(KEY_TAB)) {
            switch(currentState) {
                case GAMESTATE_GAMEPLAY:
                    currentState = GAMESTATE_EDITOR;
                    break;
                case GAMESTATE_EDITOR:
                    currentState = GAMESTATE_GAMEPLAY;
                    break;
                default: break;
            }
        }

        EndMode2D();

        if(currentState == GAMESTATE_EDITOR) {
            rlImGuiBegin();

            if(igBegin("Tile Brush Settings", NULL, 0)) {
                igCombo_Str_arr("Type", &currentTileType, tileTypes, TILE_MAX, 100);

                igEnd();
            }

            if(igBegin("Room Settings", NULL, 0)) {
                if(room) {
                    igText("Current Room: '%lu'", currentRoom);

                    igInputText("Room name", room->name, MAX_NAME_LEN, 0, NULL, NULL);

                    igDragInt("width", &newRoomSizeW, 1.0f, 1, UINT32_MAX, "%d", 0);
                    igDragInt("height", &newRoomSizeH, 1.0f, 1, UINT32_MAX, "%d", 0);

                    if(igButton("Update Size", (ImVec2) {0,0})) {
                        Room_resize(room, newRoomSizeW, newRoomSizeH);
                    }
                }

                igEnd();
            }

            if(igBegin("Level Settings", NULL, 0)) {
                if(igButton("Save Level", (ImVec2) {0,0})) {
                    Level_write_to_file(&level, "resources/levels/test0.bin");
                }

                if(igButton("Load Level", (ImVec2) {0,0})) {
                    Level_free(&level);
                    level = Level_read_from_file("resources/levels/test0.bin");
                    currentRoom = 0;
                    room = Level_get_room(level, currentRoom);
                }

                igSeparator();

                if(igButton("Make Room", (ImVec2) {0,0})) {
                    currentRoom = Level_add_room(&level, 10, 10);
                }

                if(igBeginListBox("Rooms", (ImVec2){0,0})) {
                    for(u32 i = 0; i < level.rooms_len; i++) {
                        igPushID_Int(i);
                        bool selected = (currentRoom == i);
                        if(igSelectable_Bool(level.rooms[i].name, selected, 0, (ImVec2){0,0})) {
                            currentRoom = i;
                        }

                        if (selected) igSetItemDefaultFocus();
                        igPopID();
                    }

                    igEndListBox();
                }

                igEnd();
            }

            rlImGuiEnd();
        }

        EndDrawing();
    }

    Player_free(&player);

    Level_free(&level);

    Physics_free();

    rlImGuiShutdown();
    CloseWindow();
}

