#ifndef APPLICATION_H
#define APPLICATION_H

#include "engine.h"

#define VERSION "v1.0"
#define WINDOW_WIDTH 920
#define WINDOW_HEIGHT 1624
#define BACKGROUND_COLOR \
    (Color) { 60, 60, 60, 255 }

#ifdef DEV_FEATURES
// #define DEV_SET_GAMEOVER
#endif

typedef enum {
    STATE_MENU,
    STATE_GAME,
} AppState;

typedef enum {
    MODE_ONE_PLAYER,
    MODE_TWO_PLAYER,
} GameMode;

typedef struct {
    Square square;
    Texture texture;
    i32 texture_padding;
    b32 is_pressed;
} Button;

extern AppState app_state;

// Language
const char *get_localized_text(const char *key);

// Pressed buttons
b32 is_released(const b32 pressed_state);
void update_pressable_object_state(const Square square, b32 *is_pressed);

// web interface
void update_canvas_size(i32 width, i32 height);
void set_device_type(b32 _is_mobile);

// drawing buttons
void draw_button(const Button button);
void draw_pressed_button(const Button button);

// Application
void update_draw_application(void);
void init_application(void);
void exit_application(void);

#endif