#include "augustus_game.h"

#include "augustus_math.h"
#include "augustus_gfx.h"
#include "augustus_physics.h"
#include "augustus_player.h"
#include "augustus_world.h"
#include "augustus_string.h"
#include "augustus_window.h"
#include "augustus_camera.h"

#include <math.h>

#include <stdio.h>
#include <string.h>

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

i32 Augustus_main(void) {
    String window_name = STR("Hey, window");

    Camera camera = { 0 };
    camera.zoom = 20;

    world = World_make();

    World_new_room(&world);

    while(!Window_ShouldClose()) {
        vec2s mouseWorldPosition = ScreenToWorld(Window_MousePosition());

        if(state == GAMESTATE_GAMEPLAY) {
            if(Window_MouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                vec2s delta = Window_MouseDelta();
                delta = glms_vec2_scale(delta, -1.0f/camera.zoom);
                camera.target = glms_vec2_add(camera.target, delta);
            }

            f32 wheel = Window_MouseWheelDelta();
            if(wheel != 0) {
                camera.offset = Window_MousePosition();
                camera.target = mouseWorldPosition; 

                float scaleFactor = 1.0f + (0.25f*fabsf(wheel));
                if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
                camera.zoom = clamp(camera.zoom*scaleFactor, MIN_ZOOM, MAX_ZOOM);
            }
        }

        switch(state) {
            case GAMESTATE_GAMEPLAY: {
                World_update(&world);
            } break;
            case GAMESTATE_EDITOR: {
            } break;
        }

        GFX_BeginFrame();
        GFX_ClearColor((vec3) { 0, 0, 0 });

        //BeginMode2D(camera);

        World_draw(world);

        if(state == GAMESTATE_EDITOR) {
#if 0
            Room* room = World_get(&world);
            DrawRectangleLinesEx((Rectangle) { 0, 0, room->w, room->h }, 0.1f, GREEN);

            switch(tool) {
                case EDITORTOOL_None:
                    break;
                case EDITORTOOL_Brush: {
                    vec2 size = { brushSize, brushSize };
                    vec2 cursor = vec2_tile(vec2Sub(mouseWorldPosition, vec2MulValue(size, 0.5f)));
                    cursor = vec2Add(cursor, vec2One());

                    DrawRectangleLinesEx((Rectangle) { cursor.x, cursor.y, size.x, size.y }, 0.2f, (Color) { 255, 255, 255, 100 });
                    //DrawRectangleV(cursor, size, );

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
                    vec2 cursor = mouseWorldPosition;
                    DrawRectangleV(cursor, vec2One(), (Color) { 255, 255, 255, 100 });

                    Room* room = World_get(&world);
                    if(!io->WantCaptureMouse && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        Room_add_enemy(room, Enemy_make(enemyBrushType, cursor));
                    }

                    for(u32 i = 0; i < room->enemies_len; i++) {
                        Enemy* enemy = room->enemies + i;

                        DrawRectangleLinesEx(
                                (Rectangle) {
                                    enemy->pos.x,
                                    enemy->pos.y,
                                    enemy->size.x,
                                    enemy->size.y,
                                },
                                0.3f,
                                GREEN);
                    }
                } break;
                default: {} break;
            }
#endif
        }

        //EndMode2D();

        //DrawFPS(GetScreenWidth() - 100, 5);

        for(u32 i = 0; i < world.player.max_health; i++) {
            const i32 heart_size = 50;
            const i32 spacing = 20;
            const i32 offset = spacing / 2;

            u32 p = (i * heart_size) + (i * spacing) + offset;

            if(i <= world.player.health) {
                //DrawRectangleRec((Rectangle) {p, 10, heart_size, heart_size }, RED);
            }
            else {
                //DrawRectangleLinesEx((Rectangle) {p, 10, heart_size, heart_size }, 1, GRAY);
            }
            
        }

#if 0
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
#define BUTTON(x) if(igButton(#x"##editortool", (ImVec2) {0,0})) tool = EDITORTOOL_##x;
                FOR_EDITORTOOLS(BUTTON)
#undef BUTTON

                igSeparator();

                switch(tool) {
                    case EDITORTOOL_None:
                        break;
                    case EDITORTOOL_Brush:
                        switch(brushType) {
#define TEXT(x) case TILE_##x: { igText(#x); } break;
                            FOR_TILE_TYPES(TEXT)
#undef TEXT
                        }

#define BUTTON(x) if(igButton(#x"##tiletype", (ImVec2) {0,0})) { brushType = TILE_##x; }
                        FOR_TILE_TYPES(BUTTON)
#undef BUTTON

                        igDragInt("Size", &brushSize, 0.2f, 0, 10000, "%d", 0);
                        break;
                    case EDITORTOOL_Enemies:
                        igText("Enemy Count: %lu", World_get(&world)->enemies_len);

                        if(igButton("Clear Enemies", (ImVec2) {0,0})) {
                            World_get(&world)->enemies_len = 0;
                        }

#define BUTTON(x) if(igButton(#x"##enemyType", (ImVec2) {0,0})) { enemyBrushType = ENEMY_##x; }
                        FOR_ENEMY_TYPES(BUTTON)
#undef BUTTON
                        break;
                    default: {} break;
                }

                igEnd();
            }

            rlImGuiEnd();
        }
#endif

        GFX_EndFrame();
    }

    World_free(&world);
}
