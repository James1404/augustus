#include "augustus_common.h"
#include "augustus_physics.h"
#include "augustus_player.h"
#include "augustus_level.h"
#include "augustus_string.h"

#include <raylib.h>
#include <raymath.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

const int screenWidth = 800, screenHeight = 600;

i32 main(void) {
    String window_name = STR("Hey, window");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, window_name.raw);

    Physics_init();

    Camera2D camera = { 0 };
    camera.zoom = 1;

    Level level = Level_make(30, 20);
    Player player = Player_make();

    Rigidbody rb = Rigidbody_make();

    while(!WindowShouldClose()) {
        if(IsKeyDown(KEY_W)) {
            printf("MOVE UP\n");
        }

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
            camera.zoom = Clamp(camera.zoom*scaleFactor, 0.125f, 64.0f);
        }

        Player_update(&player);
        Physics_sim();

        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);
        DrawRectanglePro(
            (Rectangle) { 0, 0, 10, 10 },
            (Vector2) { Rigidbody_pos(rb).x, Rigidbody_pos(rb).y },
            Rigidbody_rotation(rb),
            DARKPURPLE
        );

        Player_draw(&player);

        Level_draw(level);
        EndMode2D();

        EndDrawing();
    }

    Rigidbody_free(rb);

    Player_free(&player);

    Level_free(level);

    Physics_free();

    CloseWindow();
}
