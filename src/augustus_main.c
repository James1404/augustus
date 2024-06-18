#include "augustus_common.h"
#include "augustus_physics.h"

#include <raylib.h>
#include <raymath.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

const int screenWidth = 800, screenHeight = 600;

i32 main() {
    InitWindow(screenWidth, screenHeight, "hey window");

    Physics_init();

    while(!WindowShouldClose()) {
        if(IsKeyDown(KEY_W)) {
            printf("MOVE UP\n");
        }

        BeginDrawing();

        ClearBackground(MAROON);

        DrawText("hello, hello, hello", 10, 10, 80, BLACK);

        EndDrawing();
    }

    Physics_free();

    CloseWindow();
}
