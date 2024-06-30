#include "augustus_common.h"
#include "augustus_physics.h"
#include "augustus_player.h"
#include "augustus_level.h"
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
#define MAX_ZOOM 100.0f

static const int screenWidth = 1280, screenHeight = 720;

typedef enum {
    GAMESTATE_GAMEPLAY,
    GAMESTATE_EDITOR
} GameState;

#define FOR_EDITORTOOLS(DO)\
    DO(None)\
    DO(Brush)\

typedef enum {
#define ENUM(x) EDITORTOOL_##x,
    FOR_EDITORTOOLS(ENUM)
#undef ENUM
} EditorTool;

static GameState state = GAMESTATE_EDITOR;
static EditorTool tool = EDITORTOOL_None;
static char levelName[LEVEL_NAME_LEN] = "";

static i64 roomW = 0, roomH = 0;

i32 main(void) {
    String window_name = STR("Hey, window");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, window_name.raw);

    rlImGuiSetup(true);

    ImGuiIO* io = igGetIO();

    Physics_init();

    Camera2D camera = { 0 };
    camera.zoom = 20;

    level = Level_make();

    Level_new_room(&level, 40, 20);

    Player player = Player_make();

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
                Player_update(&player);
                Physics_sim();
            } break;
            case GAMESTATE_EDITOR: {
            } break;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);

        Player_draw(&player);

        Rigidbody_draw(rb);

        Level_draw(level);

        Room* room = Level_get(&level);
        DrawRectangleLinesEx((Rectangle) { 0, 0, room->w, room->h }, 0.1f, GREEN);

        Vector2 cursor = Vector2_tile(mouseWorldPosition);
        DrawRectangleV(cursor, Vector2One(), (Color) { 255, 255, 255, 100 });

        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Tile* tile = Room_at(room, cursor.x, cursor.y);
            if((cursor.x < room->w && cursor.x >= 0) && (cursor.y < room->h && cursor.y >= 0)) {
                tile->type = TILE_SOLID;
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
                igInputText("##Level Name", levelName, LEVEL_NAME_LEN, 0, NULL, NULL);

                igSameLine(0, -1);

                if(igButton("Save", (ImVec2) {0,0})) {
                    Level_write_to_file(&level, levelName);
                }

                igSameLine(0, -1);

                if(igButton("Load", (ImVec2) {0,0})) {
                    if(!Level_read_from_file(&level, levelName)) {
                        printf("Failed to load level from '%s.bin'", levelName);
                    }
                }

                igSeparator();

                if(igButton("New Room", (ImVec2) {0,0})) {
                    Level_new_room(&level, 40, 20);
                }

                igSameLine(0, -1);

                if(igButton("Delete Current Room", (ImVec2) {0,0})) {
                }

                igSeparator();

                if(igBeginListBox("Rooms", (ImVec2){0,0})) {
                    for(u32 i = 0; i < level.rooms_len; i++) {
                        igPushID_Int(i);
                        bool selected = (level.current_room == i);
                        if(igSelectable_Bool(level.rooms[i].name, selected, 0, (ImVec2){0,0})) {
                            level.current_room = i;
                        }

                        if (selected) igSetItemDefaultFocus();
                        igPopID();
                    }

                    igEndListBox();
                }

                igEnd();
            }

            if(igBegin("Room", NULL, 0)) {
                Room* room = Level_get(&level);
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
                    case EDITORTOOL_None: {} break;
                    default: {} break;
                }
            }

            rlImGuiEnd();
        }

        EndDrawing();
    }

    Rigidbody_free(rb);

    Player_free(&player);

    Level_free(&level);

    Physics_free();

    rlImGuiShutdown();
    CloseWindow();
}
