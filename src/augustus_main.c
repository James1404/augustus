#include "augustus_common.h"

#include <raylib.h>
#include <raymath.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

const int screenWidth = 800, screenHeight = 600;

int main() {
    InitWindow(screenWidth, screenHeight, "hey window");

    bool showMessageBox = false;

    while(!WindowShouldClose()) {
        BeginDrawing();

        DrawText("hello, hello, hello", 10, 10, 80, BLACK);

        ClearBackground(MAROON);

        if (GuiButton((Rectangle){ 24, 24, 120, 30 }, "#191#Show Message")) showMessageBox = true;

        if (showMessageBox)
        {
            int result = GuiMessageBox((Rectangle){ 85, 70, 250, 100 },
                "#191#Message Box", "Hi! This is a message!", "Nice;Cool");

            if (result >= 0) showMessageBox = false;
        }

        EndDrawing();
    }

    CloseWindow();
}
