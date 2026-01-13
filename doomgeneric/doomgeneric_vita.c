//doomgeneric for cross-platform development library 'Simple DirectMedia Layer'

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <unistd.h>

#include <stdbool.h>
#include <SDL2/SDL.h>

#include <psp2/ctrl.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture;

// Vita SDL window resolution
#define WINDOW_RESX 960
#define WINDOW_RESY 544

#define STICK_DEADZONE 32

static int stick_left(int v)  { return v < (128 - STICK_DEADZONE); }
static int stick_right(int v) { return v > (128 + STICK_DEADZONE); }

void DG_Init(){
  window = SDL_CreateWindow("DOOM",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            WINDOW_RESX,
                            WINDOW_RESY,
                            SDL_WINDOW_SHOWN
                            );

  // Setup renderer
  renderer =  SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
  // Clear winow
  SDL_RenderClear( renderer );
  // Render the rect to the screen
  SDL_RenderPresent(renderer);

  // Ps-vita controls enable
  sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
}

void DG_DrawFrame()
{
  SDL_UpdateTexture(texture, NULL, DG_ScreenBuffer, DOOMGENERIC_RESX * sizeof(uint32_t));

  SDL_RenderClear(renderer);

  // Scale the Doom texture to fit the Vita screen
  SDL_Rect dst = { 0, 0, WINDOW_RESX, WINDOW_RESY };

  // Optional: preserve aspect ratio with letterbox
  // int scale = WINDOW_RESY / DOOMGENERIC_RESY; // 544 / 200 = 2
  // int w = DOOMGENERIC_RESX * scale;
  // int h = DOOMGENERIC_RESY * scale;
  // SDL_Rect dst = { (WINDOW_RESX - w)/2, (WINDOW_RESY - h)/2, w, h };

  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void DG_SleepMs(uint32_t ms)
{
  SDL_Delay(ms);
}

uint32_t DG_GetTicksMs()
{
  return SDL_GetTicks();
}

int DG_GetKey(int *pressed, unsigned char *key)
{
    static unsigned int old_buttons = 0;
    static int old_lx = 128, old_ly = 128; // left stick
    static int old_rx = 128;               // right stick

    SceCtrlData ctrl;
    sceCtrlPeekBufferPositive(0, &ctrl, 1);

    // ----- DIGITAL BUTTONS -----
    unsigned int changed = ctrl.buttons ^ old_buttons;

    unsigned int mask = 1;
    for (int i = 0; i < 32; i++, mask <<= 1)
    {
        if (changed & mask)
        {
            *pressed = (ctrl.buttons & mask) != 0;

            switch (mask)
            {
                case SCE_CTRL_CROSS:     *key = KEY_ENTER;  return 1; // Confirm
                case SCE_CTRL_CIRCLE:    *key = KEY_USE;    return 1; // Alt use
                case SCE_CTRL_SQUARE:    *key = KEY_RSHIFT; return 1; // Run
                case SCE_CTRL_TRIANGLE:  *key = KEY_TAB;    return 1; // Map / Tab
                case SCE_CTRL_START:     *key = KEY_ESCAPE; return 1; // Pause / menu
                case SCE_CTRL_SELECT:    *key = KEY_F1;     return 1; // Optional / HUD
                case SCE_CTRL_RTRIGGER:  *key = KEY_FIRE;   return 1; // Fire
                case SCE_CTRL_LTRIGGER:  *key = KEY_USE;    return 1; // Use / interact
                case SCE_CTRL_UP:        *key = KEY_UPARROW; return 1;
                case SCE_CTRL_DOWN:      *key = KEY_DOWNARROW; return 1;
                case SCE_CTRL_LEFT:      *key = KEY_LEFTARROW; return 1;
                case SCE_CTRL_RIGHT:     *key = KEY_RIGHTARROW; return 1;
            }
        }
    }

    old_buttons = ctrl.buttons;

    // ----- LEFT STICK (MOVE) -----
    int move_up    = ctrl.ly < 128 - STICK_DEADZONE;
    int move_down  = ctrl.ly > 128 + STICK_DEADZONE;
    int move_left  = ctrl.lx < 128 - STICK_DEADZONE;
    int move_right = ctrl.lx > 128 + STICK_DEADZONE;

    if (move_up    != (old_ly < 128 - STICK_DEADZONE))    { *pressed = move_up;    *key = KEY_UPARROW; old_ly = ctrl.ly; return 1; }
    if (move_down  != (old_ly > 128 + STICK_DEADZONE))    { *pressed = move_down;  *key = KEY_DOWNARROW; old_ly = ctrl.ly; return 1; }
    if (move_left  != (old_lx < 128 - STICK_DEADZONE))    { *pressed = move_left;  *key = KEY_LEFTARROW; old_lx = ctrl.lx; return 1; }
    if (move_right != (old_lx > 128 + STICK_DEADZONE))    { *pressed = move_right; *key = KEY_RIGHTARROW; old_lx = ctrl.lx; return 1; }

    // ----- RIGHT STICK (TURN) -----
    int turn_left  = ctrl.rx < 128 - STICK_DEADZONE;
    int turn_right = ctrl.rx > 128 + STICK_DEADZONE;

    if (turn_left  != (old_rx < 128 - STICK_DEADZONE)) { *pressed = turn_left;  *key = KEY_LEFTARROW;  old_rx = ctrl.rx; return 1; }
    if (turn_right != (old_rx > 128 + STICK_DEADZONE)) { *pressed = turn_right; *key = KEY_RIGHTARROW; old_rx = ctrl.rx; return 1; }

    return 0;
}

void DG_SetWindowTitle(const char * title)
{
  if (window != NULL){
    SDL_SetWindowTitle(window, title);
  }
}

int main(int argc, char **argv)
{
    doomgeneric_Create(argc, argv);

    for (int i = 0; ; i++)
    {
        doomgeneric_Tick();
    }

    return 0;
}