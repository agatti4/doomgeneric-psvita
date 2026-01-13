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

#define KEYQUEUE_SIZE 16
static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static unsigned int old_buttons = 0;
static int old_lx = 128, old_ly = 128;
static int old_rx = 128;

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

static void addKeyToQueue(int pressed, unsigned int keyCode){
  unsigned short keyData = (pressed << 8) | keyCode;

  s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
  s_KeyQueueWriteIndex++;
  s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

static void Vita_HandleKeyInput()
{
    SceCtrlData ctrl;
    sceCtrlPeekBufferPositive(0, &ctrl, 1);

    unsigned int changed = ctrl.buttons ^ old_buttons;
    unsigned int pressed = changed & ctrl.buttons;
    unsigned int released = changed & old_buttons;

    // Only queue newly pressed buttons
    if (pressed & SCE_CTRL_CROSS)    addKeyToQueue(1, KEY_ENTER);
    if (pressed & SCE_CTRL_CIRCLE)   addKeyToQueue(1, KEY_USE);
    if (pressed & SCE_CTRL_SQUARE)   addKeyToQueue(1, KEY_RSHIFT);
    if (pressed & SCE_CTRL_TRIANGLE) addKeyToQueue(1, KEY_TAB);
    if (pressed & SCE_CTRL_START)    addKeyToQueue(1, KEY_ESCAPE);
    if (pressed & SCE_CTRL_SELECT)   addKeyToQueue(1, KEY_F1);
    if (pressed & SCE_CTRL_RTRIGGER) addKeyToQueue(1, KEY_FIRE);
    if (pressed & SCE_CTRL_LTRIGGER) addKeyToQueue(1, KEY_USE);

    // Only queue releases if needed
    if (released & SCE_CTRL_CROSS)    addKeyToQueue(0, KEY_ENTER);
    if (released & SCE_CTRL_RTRIGGER) addKeyToQueue(0, KEY_FIRE);
    // add for other buttons if needed

    old_buttons = ctrl.buttons;

    // ----- LEFT STICK (MOVE) -----
    if (stick_left(ctrl.ly) != stick_left(old_ly)) {
        addKeyToQueue(stick_left(ctrl.ly), KEY_UPARROW);
        old_ly = ctrl.ly;
    }
    if (stick_right(ctrl.ly) != stick_right(old_ly)) {
        addKeyToQueue(stick_right(ctrl.ly), KEY_DOWNARROW);
        old_ly = ctrl.ly;
    }
    if (stick_left(ctrl.lx) != stick_left(old_lx)) {
        addKeyToQueue(stick_left(ctrl.lx), KEY_LEFTARROW);
        old_lx = ctrl.lx;
    }
    if (stick_right(ctrl.lx) != stick_right(old_lx)) {
        addKeyToQueue(stick_right(ctrl.lx), KEY_RIGHTARROW);
        old_lx = ctrl.lx;
    }

    // ----- RIGHT STICK (TURN) -----
    if (stick_left(ctrl.rx) != stick_left(old_rx)) {
        addKeyToQueue(stick_left(ctrl.rx), KEY_LEFTARROW);
        old_rx = ctrl.rx;
    }
    if (stick_right(ctrl.rx) != stick_right(old_rx)) {
        addKeyToQueue(stick_right(ctrl.rx), KEY_RIGHTARROW);
        old_rx = ctrl.rx;
    }
}

void DG_DrawFrame()
{
  SDL_UpdateTexture(texture, NULL, DG_ScreenBuffer, DOOMGENERIC_RESX * sizeof(uint32_t));

  SDL_RenderClear(renderer);

  // Optional: preserve aspect ratio with letterbox
#ifdef NO_STRETCH
  int scale = WINDOW_RESY / DOOMGENERIC_RESY; // 544 / 200 = 2
  int w = DOOMGENERIC_RESX * scale;
  int h = DOOMGENERIC_RESY * scale;
  SDL_Rect dst = { (WINDOW_RESX - w)/2, (WINDOW_RESY - h)/2, w, h };
  SDL_RenderCopy(renderer, texture, NULL, &dst);
#else
  // Scale the Doom texture to fit the Vita screen
  SDL_Rect dst = { 0, 0, WINDOW_RESX, WINDOW_RESY };
    SDL_RenderCopy(renderer, texture, NULL, NULL);
#endif

  SDL_RenderPresent(renderer);

  // Poll Vita buttons and queue them
  Vita_HandleKeyInput();
}

void DG_SleepMs(uint32_t ms)
{
  SDL_Delay(ms);
}

uint32_t DG_GetTicksMs()
{
  return SDL_GetTicks();
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
  if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex){
    //key queue is empty
    return 0;
  }else{
    unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
    s_KeyQueueReadIndex++;
    s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

    *pressed = keyData >> 8;
    *doomKey = keyData & 0xFF;

    return 1;
  }

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