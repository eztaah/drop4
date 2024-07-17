#include "application.h"
#include "game.h"
#include "menu.h"

AppState app_state = STATE_MENU;

static cJSON *lang_data = NULL;
static b32 is_mobile = 0;
static Vec2f real_window_dimensions;

static void init_localization(const char *lang)
{
    char filepath[128];
    sprintf(filepath, "./src/assets/languages/%s.json", lang);
    char *file_content = load_text_file(filepath);
    lang_data = json_parse(file_content);
    free(file_content);
}

const char *get_localized_text(const char *key)
{
    cJSON *item = json_get_object_item(lang_data, key);
    if (json_is_string(item)) {
        return item->valuestring;
    }
    return "Unknown key";
}

static b32 was_mouse_left_button_down_last_frame_var = false;
static Vec2f initial_pos_mouse_left_button_pressed = {-1, -1};
static b32 is_click_enabled_var = true;

static Vec2f get_mouse_position_v2()
{
    Vec2f mouse_pos = get_mouse_position();

    if (!is_mobile) {
        mouse_pos.x *= (WINDOW_WIDTH / real_window_dimensions.x);
        mouse_pos.y *= (WINDOW_HEIGHT / real_window_dimensions.y);
    }

    return mouse_pos;
}

static void update_mouse_pressed(void)
{
    if (!is_mouse_button_down(MOUSE_BUTTON_LEFT) && was_mouse_left_button_down_last_frame_var) {
        was_mouse_left_button_down_last_frame_var = false;
        is_click_enabled_var = false;
        initial_pos_mouse_left_button_pressed = (Vec2f){-1, -1};
    }
    else if (is_mouse_button_down(MOUSE_BUTTON_LEFT) && !was_mouse_left_button_down_last_frame_var) {
        is_click_enabled_var = true;
        initial_pos_mouse_left_button_pressed = get_mouse_position_v2(); // Enregistrez la position de début de clic
        was_mouse_left_button_down_last_frame_var = true;
    }
}

static b32 is_click_enabled(void)
{
    return is_click_enabled_var;
}

static Vec2f get_initial_pos_mouse_left_button_pressed(void)
{
    return initial_pos_mouse_left_button_pressed;
}

b32 is_released(const b32 pressed_state)
{
    const b32 is_mouse_button_released = !is_mouse_button_down(MOUSE_BUTTON_LEFT);

    return ((pressed_state == true) && is_mouse_button_released);
}

void update_pressable_object_state(const Square square, b32 *is_pressed)
{
    Vec2f current_mouse_position = get_mouse_position_v2();
    if (is_click_enabled() && check_collision_point_square(current_mouse_position, square) && check_collision_point_square(get_initial_pos_mouse_left_button_pressed(), square)) {
        *is_pressed = true;
    }
    else {
        *is_pressed = false;
    }
}

static void init_real_window_dimensions(const i32 window_witdh, const i32 window_height)
{
    real_window_dimensions.x = window_witdh;
    real_window_dimensions.y = window_height;
}

// WEB INTERFACE
void update_canvas_size(i32 width, i32 height)
{
    real_window_dimensions.x = width;
    real_window_dimensions.y = height;
}

void set_device_type(b32 _is_mobile)
{
    is_mobile = _is_mobile;
}

static void update_draw_menu(void)
{
    update_menu(menu);
    draw_menu(menu);

    if (menu->order == START_ONE_PLAYER_GAME) {
        exit_menu();
        app_state = STATE_GAME;
        instanciate_game(MODE_ONE_PLAYER);
    }
    else if (menu->order == START_TWO_PLAYER_GAME) {
        exit_menu();
        app_state = STATE_GAME;
        instanciate_game(MODE_TWO_PLAYER);
    }
}

static void update_draw_game(void)
{
    update_game_logic(game_logic_data, game_global_animations_data);
    update_game_rendering(game_global_animations_data, game_logic_data);
    draw_game(game_logic_data, game_global_animations_data);

    if (game_logic_data->order == GO_TO_MENU) {
        exit_game();
        app_state = STATE_MENU;
        instanciate_menu();
    }
    else if (game_logic_data->order == RESTART_GAME) {
        instanciate_game(game_logic_data->mode);
        game_logic_data->game_state = GAME_STATE_PLAYING;
    }
}

void draw_button(const Button button)
{
    // < CUSTOMIZABLE
    const f32 roundness = 0.2;
    const i32 segments = 20;
    const i32 border_thickness = 8;
    const Color background_color = {37, 37, 37, 255};
    const Color border_color = BLACK;
    // >

    // Calculate the appropriate texture size
    f32 aspect_ratio = (f32)button.texture.width / (f32)button.texture.height;
    i32 max_texture_height = button.square.size - 2 * button.texture_padding;

    i32 texture_width = max_texture_height * aspect_ratio;
    i32 texture_height = max_texture_height;

    i32 texture_x = button.square.x + (button.square.size - texture_width) / 2;
    i32 texture_y = button.square.y + (button.square.size - texture_height) / 2;
    Rect new_texture_rect = {texture_x, texture_y, texture_width, texture_height};

    draw_square_rounded(button.square, roundness, segments, background_color);
    draw_texture(button.texture, new_texture_rect);
    draw_square_rounded_line_ex(button.square, roundness, segments, border_thickness, border_color);
}

void draw_pressed_button(const Button button)
{
    // < CONFIGURABLE
    const i32 size_difference = 7;
    // >

    const i32 new_width = button.square.size - 2 * size_difference;
    const f32 resize_ratio = (f32)new_width / (f32)button.square.size;

    Button new_button;
    new_button.square = (Square){
        button.square.x + size_difference,
        button.square.y + size_difference,
        button.square.size - 2 * size_difference,
    };
    new_button.texture = button.texture;
    new_button.texture_padding = button.texture_padding * resize_ratio;

    draw_button(new_button);
}

void update_draw_application(void)
{
    switch (app_state) {
        case STATE_MENU: {
            update_draw_menu();
        } break;

        case STATE_GAME: {
            update_draw_game();
        } break;

        default: {
            UNREACHABLE();
        }
    }
    update_mouse_pressed();
}

void init_application(void)
{
#ifdef DEV_FEATURES
    set_trace_log_level(LOG_DEBUG);
#else
    set_trace_log_level(LOG_INFO);
#endif

#ifdef LANG_EN
    init_localization("en");
#elif LANG_FR
    init_localization("fr");
#else
    application_panic(__FILE__, __LINE__, "no languague initialized");
#endif

    srand(time(NULL));
    init_real_window_dimensions(WINDOW_WIDTH, WINDOW_HEIGHT);
    init_window(WINDOW_WIDTH, WINDOW_HEIGHT, "drop4");
    set_target_fps(60);
    trace_log(LOG_INFO, "Game fully initialized\n");
    instanciate_menu();
}

void exit_application(void)
{
    if (app_state == STATE_MENU) {
        exit_menu();
    }
    else if (app_state == STATE_GAME) {
        exit_game();
    }
    close_window();
}
