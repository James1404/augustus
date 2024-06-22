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

static const int screenWidth = 1280, screenHeight = 720;

typedef enum {
    GAMESTATE_GAMEPLAY,
    GAMESTATE_EDITOR
} GameState;


#define FOR_EDITORTOOLS(DO)\
    DO(EDITORTOOL_NONE)\
    DO(EDITORTOOL_DRAW)\
    DO(EDITORTOOL_SELECT)\

typedef enum {
#define ENUM(x) x,
    FOR_EDITORTOOLS(ENUM)
#undef ENUM
} EditorTool;

static GameState currentState = GAMESTATE_EDITOR;
static EditorTool currentTool = EDITORTOOL_NONE;
static char currentLevelName[LEVEL_NAME_LEN] = "";

static Segment currentDrawSegment = {0};


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

    Player player = Player_make();

    while(!WindowShouldClose()) {
        if(!io->WantCaptureMouse) {
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
        }

        if(currentState == GAMESTATE_GAMEPLAY) {
            Player_update(&player);
            Physics_sim();
        }

        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);

        Player_draw(&player);

        Level_draw(level);
        Segment_draw(&currentDrawSegment, RED);

        if(IsKeyPressed(KEY_TAB) && !io->WantCaptureKeyboard) {
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

            if(igBegin("Settings", NULL, 0)) {
                igInputText("##Level Name", currentLevelName, LEVEL_NAME_LEN, 0, NULL, NULL);

                igSameLine(0, -1);

                if(igButton("Save", (ImVec2) {0,0})) {
                    Level_write_to_file(&level, TextFormat("resources/levels/%s.bin", currentLevelName));
                }

                igSameLine(0, -1);

                if(igButton("Load", (ImVec2) {0,0})) {
                    if(!Level_read_from_file(&level, TextFormat("resources/levels/%s.bin", currentLevelName))) {
                        printf("Failed to load level from '%s.bin'", currentLevelName);
                    }
                }

                igSeparator();
                        
#define BUTTON(x) if(igButton(#x, (ImVec2) {0,0})) currentTool = x;
                FOR_EDITORTOOLS(BUTTON)
#undef BUTTON

                igSeparator();

                switch(currentTool) {
                    case EDITORTOOL_NONE: {} break;
                    case EDITORTOOL_DRAW: {
                        if(igButton("Confirm", (ImVec2) {0,0})) {
                            Level_new_segment(&level, currentDrawSegment);
                            currentDrawSegment = (Segment) {0};
                        }

                        if(igButton("Cancel", (ImVec2) {0,0})) {
                            currentDrawSegment = (Segment) {0};
                        }

                        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !io->WantCaptureMouse) {
                            Vector2 pos = GetScreenToWorld2D(GetMousePosition(), camera);

                            Segment_add_vertex(&currentDrawSegment, pos);
                        }
                    } break;
                    case EDITORTOOL_SELECT: {} break;
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
