#include "augustus_common.h"
#include "augustus_physics.h"
#include "augustus_player.h"
#include "augustus_level.h"

#include <raylib.h>
#include <raymath.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

const int screenWidth = 800, screenHeight = 600;

i32 main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "hey window");

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
