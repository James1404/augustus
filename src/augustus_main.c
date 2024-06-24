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
    DO(Draw)\
    DO(Edit_vertices)\
    DO(Add_vertex)\
    DO(Add_splat)\
    DO(Edit_splats)\
    DO(New_enemies)\
    DO(Edit_enemies)\

typedef enum {
#define ENUM(x) EDITORTOOL_##x,
    FOR_EDITORTOOLS(ENUM)
#undef ENUM
} EditorTool;

static GameState state = GAMESTATE_EDITOR;
static EditorTool tool = EDITORTOOL_None;
static char levelName[LEVEL_NAME_LEN] = "";

static Segment drawSegment = {0};

static bool selectedVertex = false;
static u64 editVertexSegment, editVertex;

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
        Vector2 mouseWorldPosition = GetScreenToWorld2D(GetMousePosition(), camera);

        if(!io->WantCaptureMouse) {
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
                if(tool == EDITORTOOL_Edit_vertices) {
                }
            } break;
        }

        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);

        Player_draw(&player);

        Level_draw(level);
        Segment_draw(&drawSegment, RED);

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

        EndMode2D();

        if(state == GAMESTATE_EDITOR) {
            rlImGuiBegin();

            if(igBegin("Settings", NULL, 0)) {
                igInputText("##Level Name", levelName, LEVEL_NAME_LEN, 0, NULL, NULL);

                igSameLine(0, -1);

                if(igButton("Save", (ImVec2) {0,0})) {
                    Level_write_to_file(&level, TextFormat("resources/levels/%s.bin", levelName));
                }

                igSameLine(0, -1);

                if(igButton("Load", (ImVec2) {0,0})) {
                    if(!Level_read_from_file(&level, TextFormat("resources/levels/%s.bin", levelName))) {
                        printf("Failed to load level from '%s.bin'", levelName);
                    }
                }

                igSeparator();
                        
#define BUTTON(x) if(igButton(#x, (ImVec2) {0,0})) tool = EDITORTOOL_##x;
                FOR_EDITORTOOLS(BUTTON)
#undef BUTTON

                igSeparator();

                switch(tool) {
                    case EDITORTOOL_None: {} break;
                    case EDITORTOOL_Draw: {
                        if(igButton("Confirm", (ImVec2) {0,0})) {
                            Level_new_segment(&level, drawSegment);
                            drawSegment = (Segment) {0};
                        }

                        if(igButton("Cancel", (ImVec2) {0,0})) {
                            Segment_free(&drawSegment);
                            tool = EDITORTOOL_None;
                        }

                        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !io->WantCaptureMouse) {
                            Segment_add_vertex(&drawSegment, mouseWorldPosition);
                        }
                    } break;
                    case EDITORTOOL_Edit_vertices: {
                        if(selectedVertex) {
                            Vector2* vertex = level.segments[editVertexSegment].vertices + editVertex;

                            *vertex = mouseWorldPosition;

                            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !io->WantCaptureMouse) {
                                selectedVertex = false;
                            }
                        }
                        else {
                            for(u32 i = 0; i < level.segments_len; i++) {
                                Segment* segment = level.segments + i;

                                for(u32 j = 0; j < segment->len; j++) {
                                    Vector2 vertex = segment->vertices[j];
                                    Vector2 vertexScreen = GetWorldToScreen2D(vertex, camera);

                                    static const f32 radius = 5.0f;
                                    
                                    DrawCircleV(vertexScreen, radius, BLUE);

                                    if(Vector2Distance(GetMousePosition(), vertexScreen) < radius) {
                                        DrawCircleV(vertexScreen, radius * 5, RED);

                                        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !io->WantCaptureMouse) {
                                            selectedVertex = true;
                                            editVertexSegment = i;
                                            editVertex = j;
                                        }
                                        else if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && !io->WantCaptureMouse) {
                                            Segment_delete_vertex(segment, j);

                                            if(segment->len <= 1) {
                                                Level_remove_segment(&level, i);
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    } break;
                    case EDITORTOOL_Add_vertex: {
                        Vector2 closest = {0};
                        f32 dist = INFINITY;
                        u64 segment_idx = 0;
                        u64 vertex_idx = 0;

                        for(u32 i = 0; i < level.segments_len; i++) {
                            Segment* segment = level.segments + i;

                            for(u32 j = 0; (j < segment->len && segment->wrap) || j < segment->len - 1; j++) {
                                Vector2 a = segment->vertices[j];
                                Vector2 b = segment->vertices[(j + 1) % segment->len];

                                Vector2 p = ClosestPointToLine(a, b, mouseWorldPosition);
                                f32 d = Vector2Distance(mouseWorldPosition, p);

                                if(d < dist) {
                                    dist = d;
                                    closest = p;
                                    segment_idx = i;
                                    vertex_idx = j + 1;
                                }
                            }
                        }

                        DrawCircleV(GetWorldToScreen2D(closest, camera), 5, BLUE);

                        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                            Segment_insert(&level.segments[segment_idx], closest, vertex_idx);

                            tool = EDITORTOOL_Edit_vertices;
                            selectedVertex = true;
                            editVertexSegment = segment_idx;
                            editVertex = vertex_idx;
                        }
                    } break;
                    default: {} break;
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
