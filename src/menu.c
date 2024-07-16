#include "menu.h"

Menu *menu = NULL;

static void draw_menu_overlay(void)
{
    // < CONFIGURABLE
    const f32 title_pos_y = 50;
    const i32 title_font_size = 230;
    const Color title_color = WHITE;

    const f32 version_text_margin = 30;
    const i32 version_font_size = 60;
    const Color version_color = WHITE;
    // >

    // Draw background
    clear_background(BACKGROUND_COLOR);

    // Draw title
    const f32 title_pos_x = WINDOW_WIDTH / 2;
    draw_text("drop4", title_pos_x, title_pos_y, title_font_size, title_color, ALIGN_CENTER);

    // Draw game version
    const f32 version_pos_x = WINDOW_WIDTH - version_text_margin;
    const f32 version_pos_y = WINDOW_HEIGHT - version_font_size - version_text_margin;
    draw_text(VERSION, version_pos_x, version_pos_y, version_font_size, version_color, ALIGN_RIGHT);
}

static void draw_rules_background(void)
{
    // < CONFIGURABLE
    const f32 roundness = 0.1;
    const i32 segments = 20;
    const i32 border_thickness = 6;
    const Color background_color = (Color){50, 50, 50, 255};
    const Color border_color = (Color){15, 15, 15, 255};
    const i32 margin_x = 35;
    const f32 rules_background_pos_y = 370;
    const f32 rules_background_height = 880;
    // >

    Rect rules_background_rect = (Rect){
        margin_x,
        rules_background_pos_y,
        WINDOW_WIDTH - margin_x * 2,
        rules_background_height};
    draw_rectangle_rounded(rules_background_rect, roundness, segments, background_color);
    draw_rectangle_rounded_line_ex(rules_background_rect, roundness, segments, border_thickness, border_color);
}

static void draw_rules_text(const Texture *rules_image)
{
    // < CONFIGURABLE
    const i32 rules_text_width = WINDOW_WIDTH - 100;
    const f32 rules_text_pos_x = 50;
    const f32 rules_text_pos_y = 400;
    // >

    const f32 aspect_ratio = (f32)rules_image->height / (f32)rules_image->width;
    const f32 rules_text_height = rules_text_width * aspect_ratio;
    Rect rules_text_rect = (Rect){
        rules_text_pos_x,
        rules_text_pos_y,
        rules_text_width,
        rules_text_height};
    draw_texture(*rules_image, rules_text_rect);
}

static void draw_board_overview_image(const Texture *game_overview_image)
{
    // < CONFIGURABLE
    const i32 board_overview_image_width = 600;
    const f32 board_overview_image_pos_y = 760;
    // >

    const f32 aspect_ratio = (f32)game_overview_image->height / (f32)game_overview_image->width;
    const f32 board_overview_image_pos_x = WINDOW_WIDTH / 2 - board_overview_image_width / 2;
    const i32 board_overview_image_height = board_overview_image_width * aspect_ratio;
    Rect board_overview_image_rect = (Rect){
        board_overview_image_pos_x,
        board_overview_image_pos_y,
        board_overview_image_width,
        board_overview_image_height};
    draw_texture(*game_overview_image, board_overview_image_rect);
}

static void draw_rules(const Texture *rules_image, const Texture *game_overview_image)
{
    draw_rules_background();
    draw_rules_text(rules_image);
    draw_board_overview_image(game_overview_image);
}

static void draw_play_button(const Button *play_button)
{
    if (play_button->is_pressed) {
        draw_pressed_button(*play_button);
    }
    else {
        draw_button(*play_button);
    }
}

static void draw_mode_buttons(const Button *one_player_button, const Button *two_player_button)
{
    // Draw one player button
    if (one_player_button->is_pressed) {
        draw_pressed_button(*one_player_button);
    }
    else {
        draw_button(*one_player_button);
    }

    // Draw two players button
    if (two_player_button->is_pressed) {
        draw_pressed_button(*two_player_button);
    }
    else {
        draw_button(*two_player_button);
    }
}

void update_menu(Menu *menu)
{
    if (menu->page == PAGE_MAIN_MENU) {
        if (is_released(menu->play_button.is_pressed)) {
            menu->page = PAGE_CHOOSE_MODE;
        }
        update_pressable_object_state(menu->play_button.square, &menu->play_button.is_pressed);
    }
    else if (menu->page == PAGE_CHOOSE_MODE) {
        if (is_released(menu->one_player_button.is_pressed)) {
            menu->order = START_ONE_PLAYER_GAME;
        }
        else if (is_released(menu->two_players_button.is_pressed)) {
            menu->order = START_TWO_PLAYER_GAME;
        }
        update_pressable_object_state(menu->one_player_button.square, &menu->one_player_button.is_pressed);
        update_pressable_object_state(menu->two_players_button.square, &menu->two_players_button.is_pressed);
    }
    else {
        UNREACHABLE();
    }
}

void draw_menu(const Menu *menu)
{
    begin_drawing();

    draw_menu_overlay();
    draw_rules(&menu->rules_image, &menu->game_overview_image);

    if (menu->page == PAGE_MAIN_MENU) {
        draw_play_button(&menu->play_button);
    }
    else if (menu->page == PAGE_CHOOSE_MODE) {
        draw_mode_buttons(&menu->one_player_button, &menu->two_players_button);
    }

    end_drawing();
}

void init_menu_data(Menu *menu)
{
    // < CONFIGURABLE
    const i32 button_size = 160;
    const f32 buttons_pos_y = 1320;
    const i32 spacing_between_buttons = 20;
    const i32 texture_padding = 38;
    // >

    menu->page = PAGE_MAIN_MENU;
    menu->order = NONE;

// init language
#ifdef LANG_EN
    menu->rules_image = load_texture("./src/assets/rules-en.png");
#elif LANG_FR
    menu->rules_image = load_texture("./src/assets/rules-fr.png");
#else
    application_panic(__FILE__, __LINE__, "no languague initialized");
#endif

    menu->game_overview_image = load_texture("./src/assets/game-overview.png");

    // Init play button
    menu->play_button.square = (Square){
        WINDOW_WIDTH / 2 - button_size / 2,
        buttons_pos_y,
        button_size,
    };
    menu->play_button.texture = load_texture("./src/assets/play_image.png");
    menu->play_button.texture_padding = texture_padding;
    menu->play_button.is_pressed = false;

    // one player button
    menu->one_player_button.square = (Square){
        WINDOW_WIDTH / 2 - button_size - spacing_between_buttons,
        buttons_pos_y,
        button_size,
    };
    menu->one_player_button.texture = load_texture("./src/assets/one_player_image.png");
    menu->one_player_button.texture_padding = texture_padding;
    menu->one_player_button.is_pressed = false;

    // two players button
    menu->two_players_button.square = (Square){
        WINDOW_WIDTH / 2 + spacing_between_buttons,
        buttons_pos_y,
        button_size,
    };
    menu->two_players_button.texture = load_texture("./src/assets/two_players_image.png");
    menu->two_players_button.texture_padding = texture_padding;
    menu->two_players_button.is_pressed = false;
}

void instanciate_menu(void)
{
    ALLOC_VAR(menu, Menu);
    init_menu_data(menu);
}

void exit_menu(void)
{
    free(menu);
}
