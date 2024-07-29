#ifndef AUGUSTUS_WINDOW_H
#define AUGUSTUS_WINDOW_H

#include "SDL2/SDL_scancode.h"
#include "augustus_common.h"
#include "augustus_math.h"

typedef enum {
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
} MouseButton;

vec2s Window_MousePosition(void);
vec2s Window_MouseDelta(void);
f32 Window_MouseWheelDelta(void);

bool Window_MouseButtonDown(MouseButton btn);
bool Window_MouseButtonUp(MouseButton btn);
bool Window_MouseButtonHeld(MouseButton btn);

bool Window_KeyDown(SDL_Scancode btn);
bool Window_KeyUp(SDL_Scancode btn);
bool Window_KeyHeld(SDL_Scancode btn);

bool Window_ShouldClose(void);

f32 Window_DeltaTime(void);

#endif//AUGUSTUS_WINDOW_H
