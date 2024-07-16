#include <stdarg.h> // for va_start(), va_list, va_end()
#include <stdio.h>   // spri32f() function
#include <stdlib.h>     // srand() function

#include "./engine.h"
#include "./3dparty/raylib/raylib.h"
#include "./3dparty/cJSON/cJSON.h"

#define MAX_LOG_LENGTH 255

// Core functions
void set_trace_log_level(i32 log_level)
{
    SetTraceLogLevel(log_level);
}

void trace_log(int logLevel, const char *text, ...) {
    char buffer[MAX_LOG_LENGTH];
    va_list args;
    va_start(args, text);

    // Use vsnprintf for safe formatting with a maximum buffer size
    vsnprintf(buffer, MAX_LOG_LENGTH, text, args);
    va_end(args);

    // Now call TraceLog with the formatted string
    TraceLog(logLevel, "%s", buffer);
}

// Json functions
JSON *json_parse(const char *value)
{   
    return cJSON_Parse(value);
}

JSON *json_get_object_item(const JSON * const object, const char * const string)
{
    return cJSON_GetObjectItemCaseSensitive(object, string);
}

b32 json_is_string(const JSON * const item)
{
    return cJSON_IsString(item);
}

// Error management related functions
void application_panic(const char *file_path, i32 line, const char *message)
{
    TraceLog(LOG_FATAL, "%s:%d: APPLICATION PANIC: %s\n", file_path, line, message);
    abort();
}

void application_assert(const char *file, i32 line, b32 cond, const char *message)
{
    if (!cond) {
        application_panic(file, line, message);
    }
}

// File related functions
char *load_text_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        application_panic(__FILE__, __LINE__, "Failed to open file");
    }

    // Seek to the end of the file to determine the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (file_size == -1) {
        fclose(file);
        application_panic(__FILE__, __LINE__, "Failed to determine file size");
    }

    // Allocate memory for the file contents
    char *buffer = (char *)malloc(file_size + 1); // +1 for null-terminator
    if (buffer == NULL) {
        fclose(file);
        application_panic(__FILE__, __LINE__, "Failed to allocate memory for file content");
    }

    // Rewind the file pointer to the beginning of the file and read the file into the buffer
    rewind(file);
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < (size_t)file_size) {
        if (feof(file)) {
            application_panic(__FILE__, __LINE__, "Unexpected end of file.");
        }
        if (ferror(file)) {
            application_panic(__FILE__, __LINE__, "Error reading file");
        }
        free(buffer);
        fclose(file);
        return NULL;
    }

    // Null-terminate the string
    buffer[file_size] = '\0';

    // Close the file and return the buffer
    fclose(file);
    return buffer;
}


// Window-related functions
void init_window(i32 width, i32 height, const char *title) 
{ 
    InitWindow(width, height, title); 
}

void close_window(void) 
{ 
    CloseWindow(); 
} 

b32 window_should_close(void) 
{ 
    return WindowShouldClose(); 
}


// Timing-related functions
void set_target_fps(i32 fps)
{
    SetTargetFPS(fps);
}

f32 get_frame_time(void)
{
    return GetFrameTime();
}

// Input-related functions
b32 is_mouse_button_pressed(i32 button)
{
    return IsMouseButtonPressed(button);
}

b32 is_mouse_button_down(i32 button)
{
    return IsMouseButtonDown(button);
}

Vec2f get_mouse_position(void)
{
    return GetMousePosition();
}

// Drawing-related functions
void clear_background(Color color)
{
    ClearBackground(color);
}

void begin_drawing(void)
{
    BeginDrawing();
}

void end_drawing(void)
{
    EndDrawing();
}


// Text related functions
void draw_text(const char *text, i32 pos_x, i32 pos_y, u32 font_size, Color color, Align align)
{
    u32 width = MeasureText(text, font_size);
    switch (align) {
        case ALIGN_LEFT:                 
            break;
        case ALIGN_CENTER: 
            pos_x -= width/2; 
            break;
        case ALIGN_RIGHT:  
            pos_x -= width;   
            break;
    }
    DrawText(text, pos_x, pos_y, font_size, color);
}

void draw_text_ex(Font font, const char *text, i32 pos_x, i32 pos_y, u32 font_size, Color color, Align align)
{
    u32 width = MeasureText(text, font_size);
    switch (align) {
        case ALIGN_LEFT:                 
            break;
        case ALIGN_CENTER: 
            pos_x -= width/2; 
            break;
        case ALIGN_RIGHT:  
            pos_x -= width;   
            break;
    }
    DrawTextEx(font, text, (Vec2f){pos_x, pos_y}, font_size, 1, color);
}

void draw_inverted_text(const char *text, i32 pos_x, i32 pos_y, u32 font_size, Color color, Align align)
{
    u32 width = MeasureText(text, font_size);
    switch (align) {
        case ALIGN_LEFT:                 
            break;
        case ALIGN_CENTER: 
            pos_x += width/2; 
            break;
        case ALIGN_RIGHT:  
            pos_x += width;   
            break;
    }
    DrawTextPro(GetFontDefault(), text, (Vec2f){pos_x, pos_y + font_size}, (Vec2f){0, 0}, 180, font_size, font_size/GetFontDefault().baseSize, color);
}

const char *text_format(const char *text, ...)
{
    char buffer[MAX_LOG_LENGTH];
    va_list args;
    va_start(args, text);

    // Use vsnprintf for safe formatting with a maximum buffer size
    vsnprintf(buffer, MAX_LOG_LENGTH, text, args);
    va_end(args);

    // Now call TraceLog with the formatted string
    return TextFormat("%s", buffer);
}

// Shapes drawing functions
void draw_circle(i32 center_x, i32 center_y, f32 radius, Color color)
{
    DrawCircle(center_x, center_y, radius, color);
}

void draw_square_rounded(Square square, f32 roundness, i32 segments, Color color)
{
    DrawRectangleRounded(square_to_rect(square), roundness, segments, color);
}

void draw_square_rounded_line_ex(Square square, f32 roundness, i32 segments, f32 line_thickness, Color color)
{
    DrawRectangleRoundedLinesEx(square_to_rect(square), roundness, segments, line_thickness, color);
}


void draw_rectangle_rounded(Rect rec, f32 roundness, i32 segments, Color color)
{
    DrawRectangleRounded(rec, roundness, segments, color);
}

void draw_rectangle_rounded_line_ex(Rect rec, f32 roundness, i32 segments, f32 line_thickness, Color color)
{
    DrawRectangleRoundedLinesEx(rec, roundness, segments, line_thickness, color);
}

void draw_triangle(Vec2f v1, Vec2f v2, Vec2f v3, Color color)
{
    DrawTriangle(v1, v2, v3, color);
} 

void draw_rounded_line(Vec2f start_pos, Vec2f end_pos, f32 thickness, Color color)
{
    DrawLineEx(start_pos, end_pos, thickness, color);
    DrawCircleV(start_pos, thickness/2, color);
    DrawCircleV(end_pos, thickness/2, color);
}

void draw_donut(i32 center_x, i32 center_y, f32 radius, i32 thickness, i32 border_thickness, Color color, Color border_color)
{
    const float roundness = 1;
    const int segments = 20;

    // Calculate diameter based on radius
    i32 diameter = 2 * radius;  
    Vec2f pos = {center_x - radius, center_y - radius};

    // Outer black border
    Rect rec_outer = (Rect){pos.x, pos.y, diameter, diameter};
    DrawRectangleRoundedLinesEx(rec_outer, roundness, segments, border_thickness, border_color);
    
    // Inner black border
    Rect rec_inner = (Rect){
        pos.x + border_thickness + thickness,
        pos.y + border_thickness + thickness,
        diameter - 2 * border_thickness - 2 * thickness,
        diameter - 2 * border_thickness - 2 * thickness
    };
    DrawRectangleRoundedLinesEx(rec_inner, roundness, segments, border_thickness, border_color);

    // White donut body
    Rect rec_donut = (Rect){
        pos.x + thickness,
        pos.y + thickness,
        diameter - 2 * thickness,
        diameter - 2 * thickness
    };
    DrawRectangleRoundedLinesEx(rec_donut, roundness, segments, thickness, color);
}       


Rect square_to_rect(Square square)
{
    Rect rect = (Rect){
        square.x, 
        square.y,
        square.size,
        square.size
    };
    return rect;
}

// Basic shapes collision detection functions
b32 check_collision_point_square(Vec2f point, Square square)
{
    return CheckCollisionPointRec(point, square_to_rect(square));
}        

// Texture releated functions
Texture2D load_texture(const char *file_name)
{
    return LoadTexture(file_name);
}

void draw_texture(Texture2D texture, Rect desired_dimensions)
{
    Rect source_rect = {0, 0, texture.width, texture.height};
    Rect dest_rect = {desired_dimensions.x, desired_dimensions.y, desired_dimensions.width, desired_dimensions.height};
    DrawTexturePro(texture, source_rect, dest_rect, (Vec2f){0, 0}, 0, WHITE);
}

Font load_font(const char *file_name)
{
    return LoadFont(file_name);
}

