#pragma once
#include <windows.h>
#include <xinput.h>
#include <SDL3/SDL.h>
#include <string.h>

static SHORT flip_y(Sint16 v) { return (SHORT)(-1 - (int)v); }
static BYTE trigger_byte(Sint16 v) { return v <= 0 ? 0 : (BYTE)(((unsigned)v * 255 + 16383) / 32767); }
static XINPUT_GAMEPAD translate(const Sint16 axes[6], const bool buttons[SDL_GAMEPAD_BUTTON_COUNT]) {
    XINPUT_GAMEPAD g = {0};
    static const struct { SDL_GamepadButton s; WORD x; } map[] = {
        {SDL_GAMEPAD_BUTTON_SOUTH,XINPUT_GAMEPAD_A}, {SDL_GAMEPAD_BUTTON_EAST,XINPUT_GAMEPAD_B},
        {SDL_GAMEPAD_BUTTON_WEST,XINPUT_GAMEPAD_X}, {SDL_GAMEPAD_BUTTON_NORTH,XINPUT_GAMEPAD_Y},
        {SDL_GAMEPAD_BUTTON_BACK,XINPUT_GAMEPAD_BACK}, {SDL_GAMEPAD_BUTTON_START,XINPUT_GAMEPAD_START},
        {SDL_GAMEPAD_BUTTON_LEFT_STICK,XINPUT_GAMEPAD_LEFT_THUMB},
        {SDL_GAMEPAD_BUTTON_RIGHT_STICK,XINPUT_GAMEPAD_RIGHT_THUMB},
        {SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,XINPUT_GAMEPAD_LEFT_SHOULDER},
        {SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,XINPUT_GAMEPAD_RIGHT_SHOULDER},
        {SDL_GAMEPAD_BUTTON_DPAD_UP,XINPUT_GAMEPAD_DPAD_UP}, {SDL_GAMEPAD_BUTTON_DPAD_DOWN,XINPUT_GAMEPAD_DPAD_DOWN},
        {SDL_GAMEPAD_BUTTON_DPAD_LEFT,XINPUT_GAMEPAD_DPAD_LEFT}, {SDL_GAMEPAD_BUTTON_DPAD_RIGHT,XINPUT_GAMEPAD_DPAD_RIGHT}
    };
    for (unsigned i=0;i<sizeof(map)/sizeof(map[0]);++i) if(buttons[map[i].s]) g.wButtons |= map[i].x;
    g.sThumbLX=axes[0]; g.sThumbLY=flip_y(axes[1]);
    g.sThumbRX=axes[2]; g.sThumbRY=flip_y(axes[3]);
    g.bLeftTrigger=trigger_byte(axes[4]); g.bRightTrigger=trigger_byte(axes[5]);
    return g;
}
