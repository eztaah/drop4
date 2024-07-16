#ifndef ENGINE_H
#define ENGINE_H

#include <math.h>   // for fmax(), fabs() functions
#include <stdio.h>  // for sprintf() function
#include <stdlib.h> // for srand(), free(), malloc() functions
#include <time.h>   // for time() function

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef int i32;
typedef int b32;
typedef float f32;

#define true 1
#define false 0

// Some Basic Colors
#define YELLOW  (Color){253, 249, 0, 255}
#define ORANGE  (Color){255, 161, 0, 255}
#define RED     (Color){230, 41, 55, 255}
#define GREEN   (Color){0, 228, 48, 255}
#define SKYBLUE (Color){102, 191, 255, 255}
#define BLUE    (Color){0, 121, 241, 255}
#define PURPLE  (Color){200, 122, 255, 255}
#define BROWN   (Color){127, 106, 79, 255}
#define WHITE   (Color){255, 255, 255, 255}
#define BLACK   (Color){0, 0, 0, 255}

typedef enum {
    MOUSE_BUTTON_LEFT = 0,
    MOUSE_BUTTON_RIGHT = 1,
} MouseButton;

typedef enum {
    ALIGN_LEFT,
    ALIGN_RIGHT,
    ALIGN_CENTER,
} Align;

typedef enum {
    LOG_ALL = 0,
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
    LOG_NONE
} TraceLogLevel;

typedef struct {
    f32 x;
    f32 y;
} Vector2;
typedef Vector2 Vec2f;

typedef struct {
    i32 x;
    i32 y;
} Vec2i;

typedef struct {
    f32 x;
    f32 y;
    f32 width;
    f32 height;
} Rectangle;
typedef Rectangle Rect;

typedef struct {
    f32 x;
    f32 y;
    i32 size;
} Square;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

typedef struct {
    u32 id;
    i32 width;
    i32 height;
    i32 mipmaps;
    i32 format;
} Texture;

typedef struct {
    void *data;
    int width;
    int height;
    int mipmaps;
    int format;
} Image;

typedef struct {
    int value;
    int offsetX;
    int offsetY;
    int advanceX;
    Image image;
} GlyphInfo;

typedef struct {
    int baseSize;
    int glyphCount;
    int glyphPadding;
    Texture texture;
    Rectangle *recs;
    GlyphInfo *glyphs;
} Font;

typedef struct cJSON {
    struct cJSON *next;
    struct cJSON *prev;
    struct cJSON *child;
    int type;
    char *valuestring;
    int valueint;
    double valuedouble;
    char *string;
} cJSON;
typedef cJSON JSON;

// Core functions
void set_trace_log_level(i32 log_level);
void trace_log(i32 logLevel, const char *text, ...);

// Improved generic memory allocation macro
#define ALLOC_VAR(ptr, type)                                                   \
    do {                                                                       \
        (ptr) = (type *)calloc(1, sizeof(type));                               \
        if ((ptr) == NULL) {                                                   \
            application_panic(__FILE__, __LINE__, "Memory allocation failed"); \
        }                                                                      \
    } while (0)

// Json functions
JSON *json_parse(const char *value);
JSON *json_get_object_item(const JSON *const object, const char *const string);
b32 json_is_string(const JSON *const item);

// Error management related functions
void application_panic(const char *file_path, i32 line, const char *message);
void application_assert(const char *file, i32 line, b32 cond, const char *message);
#define ASSERT(cond, message) application_assert(__FILE__, __LINE__, cond, message)
#define UNREACHABLE() application_panic(__FILE__, __LINE__, "unreachable")

// File related functions
char *load_text_file(const char *filepath);

// Window-related functions
void init_window(i32 width, i32 height, const char *title);
void close_window(void);
b32 window_should_close(void);

// Timing-related functions
void set_target_fps(i32 fps);
f32 get_frame_time(void);

// Input-related functions
b32 is_mouse_button_pressed(i32 button);
b32 is_mouse_button_down(i32 button);
Vec2f get_mouse_position(void);

// Drawing-related functions
void clear_background(Color color);
void begin_drawing(void);
void end_drawing(void);

// Text related functions
void draw_text(const char *text, i32 pos_x, i32 pos_y, u32 font_size, Color color, Align align);
void draw_text_ex(Font font, const char *text, i32 pos_x, i32 pos_y, u32 font_size, Color color, Align align);
void draw_inverted_text(const char *text, i32 pos_x, i32 pos_y, u32 font_size, Color color, Align align);
const char *text_format(const char *text, ...);

// Shapes drawing functions
// void draw_line_ex(Vec2f start_pos, Vec2f end_pos, f32 thickness, Color color);
void draw_circle(i32 center_x, i32 center_y, f32 radius, Color color);
void draw_square_rounded(Square square, f32 roundness, i32 segments, Color color);
void draw_square_rounded_line_ex(Square square, f32 roundness, i32 segments, f32 line_thickness, Color color);
void draw_rectangle_rounded(Rect rec, f32 roundness, i32 segments, Color color);
void draw_rectangle_rounded_line_ex(Rect rec, f32 roundness, i32 segments, f32 line_thickness, Color color);
void draw_triangle(Vec2f v1, Vec2f v2, Vec2f v3, Color color);
void draw_rounded_line(Vec2f start_pos, Vec2f end_pos, f32 thickness, Color color);
void draw_donut(i32 center_x, i32 center_y, f32 radius, i32 thickness, i32 border_thickness, Color color, Color border_color);

// Basic shapes collision detection functions
Rect square_to_rect(Square square);
b32 check_collision_point_square(Vec2f point, Square square);

// Texture releated functions
Texture load_texture(const char *file_name);
void draw_texture(Texture texture, Rect desired_dimensions);

// font
Font load_font(const char *file_name);

#endif