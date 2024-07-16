#include "game.h"

#define DEFAULT_BORDER_THICKNESS 8

static Color darken_color(Color color)
{
    // CONFIGURABLE
    f32 factor = 0.6;

    Color darken_color;
    darken_color.r = color.r * factor;
    darken_color.g = color.g * factor;
    darken_color.b = color.b * factor;
    darken_color.a = color.a;

    return darken_color;
}

/**
 * Draw a circle
 * The position correspond to the top left of the box that would englobe the circle
 */
static void draw_circle_token(Vec2f pos, i32 diameter, i32 default_board_tile_size, b32 is_darken)
{
    // < CONFIGURABLE (assume default tile size)
    const i32 border_thickness = DEFAULT_BORDER_THICKNESS;
    const i32 donut_thickness = 10;
    Color color = WHITE;
    const Color border_color = BLACK;
    // >

    if (is_darken) {
        color = darken_color(color);
    }

    const f32 ratio = (f32)diameter / (f32)default_board_tile_size;
    const i32 radius = diameter / 2;
    const Vec2f center = {pos.x + radius, pos.y + radius};
    const i32 real_border_thickness = border_thickness * ratio;
    const i32 real_donut_thickness = donut_thickness * ratio;

    draw_donut(center.x, center.y, radius, real_donut_thickness, real_border_thickness, color, border_color);
}

/**
 * Draw a cross
 * The position correspond to the top left of the cross
 */
static void draw_cross_token(Vec2f pos, i32 width, i32 default_board_tile_size, b32 is_darken)
{
    // < CONFIGURABLE (assume default tile size)
    const i32 line_width = 12;
    const i32 border_thickness = DEFAULT_BORDER_THICKNESS;
    Color color = WHITE;
    const Color border_color = BLACK;
    // >

    if (is_darken) {
        color = darken_color(color);
    }

    // Get real values (calculate with ratio)
    const f32 ratio = (f32)width / (f32)default_board_tile_size;
    const f32 real_border_thickness = border_thickness * ratio;
    const f32 real_line_width = line_width * ratio;
    const f32 background_line_width = real_line_width + real_border_thickness * 2;

    // Calculate the endpoints of the diagonal lines
    Vec2f start1 = {pos.x + real_border_thickness, pos.y + real_border_thickness};
    Vec2f end1 = {pos.x + width - real_border_thickness, pos.y + width - real_border_thickness};
    Vec2f start2 = {pos.x + width - real_border_thickness, pos.y + real_border_thickness};
    Vec2f end2 = {pos.x + real_border_thickness, pos.y + width - real_border_thickness};

    // Draw black diagonal lines (background for the white lines)
    draw_rounded_line(start1, end1, background_line_width, border_color);
    draw_rounded_line(start2, end2, background_line_width, border_color);

    // Draw white diagonal lines (foreground)
    draw_rounded_line(start1, end1, real_line_width, color);
    draw_rounded_line(start2, end2, real_line_width, color);
}

/**
 * Draw card
 */
static void draw_card(TileType tile_type, Vec2f pos, i32 size, i32 default_board_tile_size, b32 is_darken)
{
    ASSERT(tile_type != TOKEN_PLAYER1 && tile_type != TOKEN_PLAYER2, "The tile should not be a player token");

    // < CONFIGURABLE (assume default tile size)
    const f32 roundness = 0.1f;
    const i32 segments = 20;
    const i32 border_thickness = DEFAULT_BORDER_THICKNESS;
    const Color border_color = BLACK;
    // >

    const f32 ratio = (f32)size / (f32)default_board_tile_size;
    const f32 real_border_thickness = border_thickness * ratio;
    // Define points for triangles
    Vec2f v1 = {pos.x, pos.y};
    Vec2f v2 = {pos.x + size, pos.y + size};
    Vec2f v3 = {pos.x, pos.y + size};
    Vec2f v4 = {pos.x + size, pos.y};
    // Get card colors
    Color color1 = get_tile_color(tile_type, 1);
    Color color2 = get_tile_color(tile_type, 2);
    if (is_darken) {
        color1 = darken_color(color1);
        color2 = darken_color(color2);
    }

    // Draw diagonally split triangles
    draw_triangle(v1, v2, v4, color1);
    draw_triangle(v1, v3, v2, color2);

    // // Draw border
    Square square = (Square){pos.x, pos.y, size};
    draw_square_rounded_line_ex(square, roundness, segments, real_border_thickness, border_color);
    draw_rounded_line(v1, v2, real_border_thickness, border_color);
}

/**
 * Draw a tile
 * This can be a player token or a card
 */
static void draw_tile(TileType tile_type, Vec2f pos, i32 size, i32 default_board_tile_size, b32 is_darken)
{
    if (tile_type == TOKEN_PLAYER1) {
        draw_cross_token(pos, size, default_board_tile_size, is_darken);
    }
    else if (tile_type == TOKEN_PLAYER2) {
        draw_circle_token(pos, size, default_board_tile_size, is_darken);
    }
    else {
        // TODO: ASSERT(tile is a card)
        draw_card(tile_type, pos, size, default_board_tile_size, is_darken);
    }
}

static void draw_pressed_tile(TileType tile_type, Vec2f pos, i32 size, i32 default_board_tile_size)
{
    // < CONFIGURABLE
    const i32 size_difference = 5;
    // >

    const Vec2f new_pos = {pos.x + size_difference, pos.y + size_difference};
    const i32 new_size = size - 2 * size_difference;

    draw_tile(tile_type, new_pos, new_size, default_board_tile_size, false);
}

/**
 * Draw board tiles
 */
static b32 is_part_of_winning_combination(const Tile board[][BOARD_COLUMNS_NB], int row, int col)
{
    TileType type = board[row][col].type;

    // Checking the line
    b32 horizontal_win = true;
    for (int j = 0; j < BOARD_COLUMNS_NB; j++) {
        if (board[row][j].type != type) {
            horizontal_win = false;
            break;
        }
    }

    // Checking the column
    b32 vertical_win = true;
    for (int i = 0; i < BOARD_ROWS_NB; i++) {
        if (board[i][col].type != type) {
            vertical_win = false;
            break;
        }
    }

    // Checking 2x2 squares, if applicable
    b32 square_win = false;
    for (int dr = 0; dr <= 1; dr++) {
        for (int dc = 0; dc <= 1; dc++) {
            int square_row = row - dr;
            int square_col = col - dc;
            // Check that indices are valid for a 2x2 square
            if (square_row >= 0 && square_row < BOARD_ROWS_NB - 1 && square_col >= 0 && square_col < BOARD_COLUMNS_NB - 1) {
                square_win = (board[square_row][square_col].type == type &&
                              board[square_row][square_col + 1].type == type &&
                              board[square_row + 1][square_col].type == type &&
                              board[square_row + 1][square_col + 1].type == type);
                if (square_win) {
                    break; // Early exit if a winning combination is found
                }
            }
        }
        if (square_win)
            break;
    }

    // Checking diagonals, if applicable
    b32 diagonal1_win = true, diagonal2_win = true;
    if (row == col) { // Main diagonal
        for (int i = 0; i < BOARD_ROWS_NB; i++) {
            if (board[i][i].type != type) {
                diagonal1_win = false;
                break;
            }
        }
    }
    else {
        diagonal1_win = false;
    }

    if (row == BOARD_ROWS_NB - 1 - col) { // Secondary diagonal
        for (int i = 0; i < BOARD_ROWS_NB; i++) {
            if (board[i][BOARD_ROWS_NB - 1 - i].type != type) {
                diagonal2_win = false;
                break;
            }
        }
    }
    else {
        diagonal2_win = false;
    }

    return horizontal_win || vertical_win || square_win || diagonal1_win || diagonal2_win;
}

static void draw_board_tiles_end_game(const Tile board[][BOARD_COLUMNS_NB], const BoardGlobalRenderingData *board_rendering_data)
{
    const Vec2f first_card_pos = {board_rendering_data->pos.x + board_rendering_data->padding, board_rendering_data->pos.y + board_rendering_data->padding};
    for (i32 i = 0; i < 4; i++) {
        for (i32 j = 0; j < 4; j++) {
            Vec2f pos = {
                pos.x = first_card_pos.x + i * (board_rendering_data->tile_size + board_rendering_data->tile_spacing),
                pos.y = first_card_pos.y + j * (board_rendering_data->tile_size + board_rendering_data->tile_spacing)};

            b32 is_darken = true;
            if (is_part_of_winning_combination(board, i, j)) {
                is_darken = false;
            }

            draw_tile(board[i][j].type, pos, board_rendering_data->tile_size, board_rendering_data->tile_size, is_darken);
        }
    }
}

static void draw_board_tiles_playing(const Tile board[][BOARD_COLUMNS_NB], const BoardGlobalRenderingData *board_rendering_data)
{
    const Vec2f first_card_pos = {board_rendering_data->pos.x + board_rendering_data->padding, board_rendering_data->pos.y + board_rendering_data->padding};
    for (i32 i = 0; i < 4; i++) {
        for (i32 j = 0; j < 4; j++) {
            Vec2f pos;
            if (is_token_placement_animation_running()) {
                i32 tile_x = (i32)((game_global_animations_data->player_stack_anim_data->end_square.x - rendering_data->board.game_pos_x) / (rendering_data->board.tile_size + rendering_data->board.tile_spacing));
                i32 tile_y = (i32)((game_global_animations_data->player_stack_anim_data->end_square.y - rendering_data->board.pos.y) / (rendering_data->board.tile_size + rendering_data->board.tile_spacing));
                if (tile_x == i && tile_y == j) {
                    continue;
                }
                goto draw;
            }

        draw:
            pos.x = first_card_pos.x + i * (board_rendering_data->tile_size + board_rendering_data->tile_spacing);
            pos.y = first_card_pos.y + j * (board_rendering_data->tile_size + board_rendering_data->tile_spacing);
            if (board[i][j].is_pressed) {
                draw_pressed_tile(board[i][j].type, pos, board_rendering_data->tile_size, board_rendering_data->tile_size);
            }
            else {
                draw_tile(board[i][j].type, pos, board_rendering_data->tile_size, board_rendering_data->tile_size, false);
            }
        }
    }
}

static void draw_board_tiles(const GameLogicData *game, const BoardGlobalRenderingData *board_rendering_data)
{

    if ((game->game_state == GAME_STATE_WIN || game->game_state == GAME_STATE_DRAW) && !is_token_placement_animation_running()) {
        draw_board_tiles_end_game(game->board, board_rendering_data);
    }
    else {
        draw_board_tiles_playing(game->board, board_rendering_data);
    }
}

/**
 * Draw board
 * You don´t need to pass the board position or width as theses are stored as global variables
 */
static void draw_board(const GameLogicData *game, const BoardGlobalRenderingData *board_rendering_data)
{
    // < CONFIGURABLE
    const Vec2f pos = board_rendering_data->pos;
    const i32 width = board_rendering_data->size;
    const f32 roundness = 0.1;
    const i32 segments = 20;
    const i32 border_thickness = DEFAULT_BORDER_THICKNESS;
    const Color color = (Color){37, 37, 37, 255};
    const Color border_color = BLACK;
    // >

    // draw board background
    Rect rec = (Rect){pos.x, pos.y, width, width};
    draw_rectangle_rounded(rec, roundness, segments, color);
    draw_rectangle_rounded_line_ex(rec, roundness, segments, border_thickness, border_color);

    // draw board tiles
    draw_board_tiles(game, board_rendering_data);
}

/**
 * Draw card stack
 */
static void draw_card_stack(const BoardGlobalRenderingData *board_rendering_data, const CardStackGlobalRenderingData *card_stack_rendering_data, const TileType stack_previous_top_card)
{
    // < CONFIGURABLE
    Vec2f pos = card_stack_rendering_data->pos;
    i32 width = board_rendering_data->tile_size;
    // >

    if (stack_previous_top_card != EMPTY_TILE) {
        draw_tile(stack_previous_top_card, pos, width, board_rendering_data->tile_size, false);
    }
}

static void draw_player_area_background(const PlayersStackGlobalRenderingData *player_stack_rendering_data, const PlayerTextGlobalRenderingData *player_text_rendering_data)
{
    // < CONFIGURABLE
    const f32 roundness = 0.1;
    const i32 segments = 20;
    const i32 border_thickness = 6;
    const f32 background_padding = 30;

    const Color background_color = (Color){50, 50, 50, 255};
    const Color border_color = (Color){15, 15, 15, 255};
    // >

    const i32 start_pos_x = player_stack_rendering_data->pos_x;

    // draw board background player 1
    Rect rec1 = (Rect){
        start_pos_x - background_padding,
        player_text_rendering_data->p1_pos_y - player_text_rendering_data->p1_zoom_value - 16,
        player_stack_rendering_data->stack_width + 2 * background_padding,
        player_text_rendering_data->font_size + player_text_rendering_data->p1_zoom_value + player_stack_rendering_data->token_size + 60};
    draw_rectangle_rounded(rec1, roundness, segments, background_color);
    draw_rectangle_rounded_line_ex(rec1, roundness, segments, border_thickness, border_color);

    // draw board background player 2
    Rect rec2 = (Rect){
        start_pos_x - background_padding,
        player_stack_rendering_data->p2_pos_y - 22,
        player_stack_rendering_data->stack_width + 2 * background_padding,
        player_text_rendering_data->font_size + player_text_rendering_data->p2_zoom_value + player_stack_rendering_data->token_size + 60};
    draw_rectangle_rounded(rec2, roundness, segments, background_color);
    draw_rectangle_rounded_line_ex(rec2, roundness, segments, border_thickness, border_color);
}

/**
 * Draw player token stack
 */
static void draw_players_tokens_stack(const PlayersStackGlobalRenderingData *player_stack_rendering_data, const i32 player1_remaining_tokens, const i32 player2_remaining_tokens, const i32 default_board_tile_size)
{
    // < CONFIGURABLE
    const i32 token_size = player_stack_rendering_data->token_size;
    const i32 tokens_spacing = player_stack_rendering_data->token_spacing;
    const f32 p1_start_pos_y = player_stack_rendering_data->p1_pos_y;
    const f32 p2_start_pos_y = player_stack_rendering_data->p2_pos_y;
    // >

    const i32 start_pos_x = player_stack_rendering_data->pos_x;
    // draw player1 stack
    for (i32 i = 0; i < player1_remaining_tokens; i++) {
        Vec2f pos = {start_pos_x + i * (token_size + tokens_spacing),
                     p1_start_pos_y};

        draw_tile(TOKEN_PLAYER1, pos, token_size, default_board_tile_size, false);
    }
    // draw player2 stack
    for (i32 i = 0; i < player2_remaining_tokens; i++) {
        Vec2f pos = {start_pos_x + i * (token_size + tokens_spacing),
                     p2_start_pos_y};

        draw_tile(TOKEN_PLAYER2, pos, token_size, default_board_tile_size, false);
    }
}

/**
 * Draw player id text
 */
static void draw_players_text(const PlayerTextGlobalRenderingData *player_text_rendering_data, const GameMode game_mode)
{
    // < CONFIGURABLE
    const i32 font_size = player_text_rendering_data->font_size;
    const i32 p1_zoom_value = player_text_rendering_data->p1_zoom_value;
    const i32 p2_zoom_value = player_text_rendering_data->p2_zoom_value;
    const Color color = WHITE;
    const f32 pos_x = WINDOW_WIDTH / 2;
    // >

    const f32 p1_pos_y = player_text_rendering_data->p1_pos_y;
    const f32 p2_pos_y = player_text_rendering_data->p2_pos_y;

    draw_text(get_localized_text("ui_player_1"), pos_x, p1_pos_y - p1_zoom_value, font_size + p1_zoom_value, color, ALIGN_CENTER);
    if (game_mode == MODE_ONE_PLAYER) {
        draw_text(get_localized_text("ui_bot"), pos_x, p2_pos_y, font_size + p2_zoom_value, color, ALIGN_CENTER);
    }
    else if (game_mode == MODE_TWO_PLAYER) {
        draw_inverted_text(get_localized_text("ui_player_2"), pos_x, p2_pos_y, font_size + p2_zoom_value, color, ALIGN_CENTER);
    }
    else {
        UNREACHABLE();
    }
}

/**
 * Draw ui message
 */
static void draw_ui_message(const BoardGlobalRenderingData *board_rendering_data, const InfoMessage *info_message_p1, const InfoMessage *info_message_p2)
{
    // CONFIGURABLE
    const i32 distance_from_board = 60;
    const i32 font_size = 40;
    const Color color = RED;
    // >

    if (!is_token_placement_animation_running()) {
        if (info_message_p1->display_time > 0.0f) {
            draw_text(info_message_p1->message, WINDOW_WIDTH / 2, board_rendering_data->pos.y + board_rendering_data->size + distance_from_board, font_size, color, ALIGN_CENTER);
        }
        if (info_message_p2->display_time > 0.0f) {
            draw_inverted_text(info_message_p2->message, WINDOW_WIDTH / 2, board_rendering_data->pos.y - distance_from_board - font_size, font_size, color, ALIGN_CENTER);
        }
    }
}

/**
 * Draw gameover text
 */
static void draw_game_over_text(const BoardGlobalRenderingData *board_rendering_data, const GameLogicData *game)
{
    // CONFIGURABLE
    const i32 distance_from_board = 110;
    const i32 font_size = 100;
    const Color color = WHITE;
    // >

    const char *victory_message = get_localized_text("victory_message");
    const char *defeat_message = get_localized_text("defeat_message");
    const char *draw_message = get_localized_text("draw_message");
    const f32 message_pos_x = WINDOW_WIDTH / 2;
    const f32 message_pos_y_p1 = board_rendering_data->pos.y + board_rendering_data->size + distance_from_board;
    const f32 message_pos_y_p2 = board_rendering_data->pos.y - font_size - distance_from_board;

    if (game->game_state == GAME_STATE_WIN) {
        const Player winner = game->current_player;

        if (winner == PLAYER1) {
            draw_text(victory_message, message_pos_x, message_pos_y_p1, font_size, color, ALIGN_CENTER);
            if (game->mode == MODE_TWO_PLAYER) {
                draw_inverted_text(defeat_message, message_pos_x, message_pos_y_p2, font_size, color, ALIGN_CENTER);
            }
        }
        else if (winner == PLAYER2) {
            draw_text(defeat_message, message_pos_x, message_pos_y_p1, font_size, color, ALIGN_CENTER);
            if (game->mode == MODE_TWO_PLAYER) {
                draw_inverted_text(victory_message, message_pos_x, message_pos_y_p2, font_size, color, ALIGN_CENTER);
            }
        }
    }
    else if (game->game_state == GAME_STATE_DRAW) {
        draw_text(draw_message, message_pos_x, message_pos_y_p1, font_size, color, ALIGN_CENTER);
        if (game->mode == MODE_TWO_PLAYER) {
            draw_inverted_text(draw_message, message_pos_x, message_pos_y_p2, font_size, color, ALIGN_CENTER);
        }
    }
    else {
        UNREACHABLE();
    }
}

/**
 * Draw button
 */
static void draw_restart_button(const Button *restart_button)
{
    if (restart_button->is_pressed) {
        draw_pressed_button(*restart_button);
    }
    else {
        draw_button(*restart_button);
    }
}

static void draw_home_button(const Button *home_button)
{
    if (home_button->is_pressed) {
        draw_pressed_button(*home_button);
    }
    else {
        draw_button(*home_button);
    }
}

static void update_zoom_value_player_text(PlayerTextGlobalRenderingData *player_text_rendering_data, const GameState game_state, const Player current_player)
{
    // < CUSTOMIZABLE
    const i32 default_zoom_value = 30;
    // >

    if (game_state == GAME_STATE_WIN || game_state == GAME_STATE_DRAW) {
        player_text_rendering_data->p1_zoom_value = 0;
        player_text_rendering_data->p2_zoom_value = 0;
    }
    else if (current_player == PLAYER1 && is_token_placement_animation_running() == false) {
        player_text_rendering_data->p1_zoom_value = default_zoom_value;
        player_text_rendering_data->p2_zoom_value = 0;
    }
    else if (current_player == PLAYER2 && is_token_placement_animation_running() == false) {
        player_text_rendering_data->p1_zoom_value = 0;
        player_text_rendering_data->p2_zoom_value = default_zoom_value;
    }
}

void update_game_rendering(GameAnimationsData *game_animations_data, const GameLogicData *game)
{
    // zoom
    update_zoom_value_player_text(&rendering_data->players_text, game->game_state, game->current_player);

    // animations
    if (is_token_placement_animation_running() && game_animations_data->card_stack_anim_data->is_done == false) {
        update_animation(game_animations_data->card_stack_anim_data);
    }
    if (is_token_placement_animation_running() && game_animations_data->player_stack_anim_data->is_done == false) {
        update_animation(game_animations_data->player_stack_anim_data);
    }

    if (is_token_placement_animation_running() && is_token_placement_animation_running() && game_animations_data->card_stack_anim_data->is_done && game_animations_data->player_stack_anim_data->is_done) {
        end_token_placement_animation();
    }

    if (game->first_turn == false && rendering_data->board.pos.x != rendering_data->board.game_pos_x) {
        update_board_pos();
    }
}

void draw_game(const GameLogicData *game, const GameAnimationsData *game_animations_data)
{
    begin_drawing();

    clear_background(BACKGROUND_COLOR);
    draw_board(game, &rendering_data->board);
    draw_ui_message(&rendering_data->board, &game->info_message_p1, &game->info_message_p2);

    draw_player_area_background(&rendering_data->players_stack, &rendering_data->players_text);
    draw_players_tokens_stack(&rendering_data->players_stack, game->player1_remaining_tokens, game->player2_remaining_tokens, rendering_data->board.tile_size);
    draw_players_text(&rendering_data->players_text, game->mode);
    draw_card_stack(&rendering_data->board, &rendering_data->card_stack, game->stack_top_card.type);

    if ((game->game_state == GAME_STATE_WIN || game->game_state == GAME_STATE_DRAW) && is_token_placement_animation_running() == false) {
        draw_game_over_text(&rendering_data->board, game);
        draw_home_button(&game->home_button);
        draw_restart_button(&game->restart_button);
    }

    // Animations
    if (game_animations_data->card_stack_anim_data != NULL) {
        Vec2f pos = (Vec2f){game_animations_data->card_stack_anim_data->square.x, game_animations_data->card_stack_anim_data->square.y};
        draw_tile(
            game_animations_data->card_stack_anim_data->tile_type,
            pos,
            game_animations_data->card_stack_anim_data->square.size,
            rendering_data->board.tile_size,
            false);
    }
    if (game_animations_data->player_stack_anim_data != NULL) {
        Vec2f pos = (Vec2f){game_animations_data->player_stack_anim_data->square.x, game_animations_data->player_stack_anim_data->square.y};
        draw_tile(
            game_animations_data->player_stack_anim_data->tile_type,
            pos,
            game_animations_data->player_stack_anim_data->square.size,
            rendering_data->board.tile_size,
            false);
    }

    end_drawing();
}
