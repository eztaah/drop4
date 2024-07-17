#include "game.h"

b32 have_common_color(const TileType tile1_type, const TileType tile2_type)
{
    Color color1_card1 = get_tile_color(tile1_type, 1);
    Color color2_card1 = get_tile_color(tile1_type, 2);
    Color color1_card2 = get_tile_color(tile2_type, 1);
    Color color2_card2 = get_tile_color(tile2_type, 2);

    return (color1_card1.r == color1_card2.r && color1_card1.g == color1_card2.g && color1_card1.b == color1_card2.b) ||
           (color1_card1.r == color2_card2.r && color1_card1.g == color2_card2.g && color1_card1.b == color2_card2.b) ||
           (color2_card1.r == color1_card2.r && color2_card1.g == color1_card2.g && color2_card1.b == color1_card2.b) ||
           (color2_card1.r == color2_card2.r && color2_card1.g == color2_card2.g && color2_card1.b == color2_card2.b);
}

b32 is_board_full(const Tile board[][BOARD_COLUMNS_NB])
{
    b32 is_full = true;
    for (i32 i = 0; i < 4; i++) {
        for (i32 j = 0; j < 4; j++) {
            if (board[i][j].type != TOKEN_PLAYER1 && board[i][j].type != TOKEN_PLAYER2) {
                is_full = false;
            }
        }
    }
    return is_full;
}

static b32 is_winner(const Player player, GameLogicData *game)
{
    TileType player_token_type;
    if (player == PLAYER1) {
        player_token_type = TOKEN_PLAYER1;
    }
    else if (player == PLAYER2) {
        player_token_type = TOKEN_PLAYER2;
    }
    else {
        UNREACHABLE();
    }

    // if board full, do not return winner
    if (is_board_full(game->board)) {
        return false;
    }

    // Check horizontal and vertical lines
    for (i32 i = 0; i < BOARD_ROWS_NB; i++) {
        b32 horizontal_win = true;
        b32 vertical_win = true;
        for (i32 j = 0; j < BOARD_COLUMNS_NB; j++) {
            if (game->board[i][j].type != player_token_type) {
                horizontal_win = false;
            }
            if (game->board[j][i].type != player_token_type) {
                vertical_win = false;
            }
        }
        if (horizontal_win || vertical_win) {
            return true;
        }
    }

    // Check 2x2 squares
    for (i32 i = 0; i < BOARD_ROWS_NB - 1; i++) {
        for (i32 j = 0; j < BOARD_COLUMNS_NB - 1; j++) {
            if (game->board[i][j].type == player_token_type && game->board[i][j + 1].type == player_token_type &&
                game->board[i + 1][j].type == player_token_type && game->board[i + 1][j + 1].type == player_token_type) {
                return true;
            }
        }
    }

    // Check diagonals
    b32 diagonal1_win = true;
    b32 diagonal2_win = true;
    for (i32 i = 0; i < BOARD_ROWS_NB; i++) {
        if (game->board[i][i].type != player_token_type) {
            diagonal1_win = false;
        }
        if (game->board[i][BOARD_ROWS_NB - 1 - i].type != player_token_type) {
            diagonal2_win = false;
        }
    }
    if (diagonal1_win || diagonal2_win) {
        return true;
    }

    // Check if the next player can play
    for (i32 i = 0; i < BOARD_ROWS_NB; i++) {
        for (i32 j = 0; j < BOARD_COLUMNS_NB; j++) {
            const b32 tile_is_a_card = game->board[i][j].type != TOKEN_PLAYER1 && game->board[i][j].type != TOKEN_PLAYER2;
            if (game->stack_top_card.type != EMPTY_TILE) {
                if (tile_is_a_card && have_common_color(game->board[i][j].type, rendering_data->stack_top_card_ui.type)) {
                    return false;
                }
                else {
                    continue;
                }
            }
            else {
                return false;
            }
        }
    }

    if (game->current_player == PLAYER1) {
        sprintf(game->info_message_p1.message, "%s", get_localized_text("opponent_cannot_play_message"));
        game->info_message_p1.display_time = 100;

        if (game->mode == MODE_TWO_PLAYER) {
            sprintf(game->info_message_p2.message, "%s", get_localized_text("cannot_play_message"));
            game->info_message_p2.display_time = 100;
        }
    }
    else if (game->current_player == PLAYER2) {
        sprintf(game->info_message_p1.message, "%s", get_localized_text("cannot_play_message"));
        game->info_message_p1.display_time = 100;

        if (game->mode == MODE_TWO_PLAYER) {
            sprintf(game->info_message_p2.message, "%s", get_localized_text("opponent_cannot_play_message"));
            game->info_message_p2.display_time = 100;
        }
    }
    return true;
}

static b32 is_token_placement_valid(const Vec2i token_pos, const GameLogicData *game, InfoMessage *info_message)
{
    b32 is_valid = true;
    TileType board_tile_type = game->board[token_pos.x][token_pos.y].type;

    // if token is not on the board
    if (token_pos.x < 0 || token_pos.x > 3 || token_pos.y < 0 || token_pos.y > 3) {
        is_valid = false;
    }

    // If it is first turn and player want to place in the middle
    if (game->first_turn && (token_pos.x == 1 || token_pos.x == 2) && (token_pos.y == 1 || token_pos.y == 2)) {
        trace_log(LOG_INFO, "You can't place token in the middle if first turn!");
        sprintf(info_message->message, "%s", get_localized_text("info_first_turn"));
        info_message->display_time = 2.0f;
        is_valid = false;
    }

    // If the cell is already occupied
    else if (board_tile_type == TOKEN_PLAYER1 || board_tile_type == TOKEN_PLAYER2) {
        trace_log(LOG_INFO, "Cell already occupied!");
        sprintf(info_message->message, "%s", get_localized_text("info_occupied"));
        info_message->display_time = 2.0f;
        is_valid = false;
    }

    // If the board card can't be removed
    else if (game->first_turn == false && !have_common_color(board_tile_type, game->stack_top_card.type)) {
        trace_log(LOG_INFO, "The card does not share a color with the last discarded card!");
        sprintf(info_message->message, "%s", get_localized_text("info_no_color_share"));
        info_message->display_time = 2.0f;
        is_valid = false;
    }

    return is_valid;
}

static Vec2i get_next_token_placement(GameLogicData *game)
{
    // One player game
    if (game->mode == MODE_ONE_PLAYER) {
        if (game->current_player == PLAYER1) {
            for (i32 i = 0; i < 4; i++) {
                for (i32 j = 0; j < 4; j++) {
                    Vec2i tile_coord = (Vec2i){i, j};
                    if (is_released(game->board[i][j].is_pressed) && is_token_placement_animation_running() == false) {
                        if (is_token_placement_valid(tile_coord, game, &game->info_message_p1)) {
                            return (Vec2i){i, j};
                        }
                    }
                }
            }
            return (Vec2i){-1, -1};
        }
        else if (game->current_player == PLAYER2 && game->ai_thinking_duration <= 0.0f && is_token_placement_animation_running() == false) {
            game->ai_thinking_duration = 0.0f;
            Vec2i pressed_tile = get_ai_pressed_tile(game->board, game->stack_top_card);
            if (is_token_placement_valid(pressed_tile, game, &game->info_message_p2)) {
                return pressed_tile;
            }
            else {
                application_panic(__FILE__, __LINE__, "ai made an inccorect move");
            }
        }
        else {
            return (Vec2i){-1, -1};
        }
    }

    // Two player game
    else if (game->mode == MODE_TWO_PLAYER) {
        for (i32 i = 0; i < 4; i++) {
            for (i32 j = 0; j < 4; j++) {
                Vec2i tile_coord = (Vec2i){i, j};
                if (is_released(game->board[i][j].is_pressed) && is_token_placement_animation_running() == false) {
                    if (game->current_player == PLAYER1) {
                        if (is_token_placement_valid(tile_coord, game, &game->info_message_p1)) {
                            return (Vec2i){i, j};
                        }
                    }
                    else if (game->current_player == PLAYER2) {
                        if (is_token_placement_valid(tile_coord, game, &game->info_message_p2)) {
                            return (Vec2i){i, j};
                        }
                    }
                    else {
                        UNREACHABLE();
                    }
                }
            }
        }
        return (Vec2i){-1, -1};
    }

    else {
        UNREACHABLE();
    }
    return (Vec2i){-1, -1};
}

static void update_tiles_pressed_state(Tile board[][BOARD_COLUMNS_NB])
{
    const Vec2f first_card_pos = {rendering_data->board.pos.x + rendering_data->board.padding, rendering_data->board.pos.y + rendering_data->board.padding};
    for (i32 i = 0; i < 4; i++) {
        for (i32 j = 0; j < 4; j++) {
            // create a square for the current cell
            Square tile_square;
            tile_square.x = first_card_pos.x + i * (rendering_data->board.tile_size + rendering_data->board.tile_spacing);
            tile_square.y = first_card_pos.y + j * (rendering_data->board.tile_size + rendering_data->board.tile_spacing);
            tile_square.size = rendering_data->board.tile_size;

            // verifiy is the current cell is pressed
            update_pressable_object_state(tile_square, &board[i][j].is_pressed);
        }
    }
}

Color get_tile_color(const TileType tile_type, const i32 color_number)
{
    if (color_number == 1) {
        switch (tile_type) {
            case CARD_BLUE_RED:
                return BLUE;
            case CARD_BLUE_PURPLE:
                return BLUE;
            case CARD_BLUE_GREEN:
                return BLUE;
            case CARD_BLUE_BROWN:
                return BLUE;
            case CARD_YELLOW_RED:
                return YELLOW;
            case CARD_YELLOW_PURPLE:
                return YELLOW;
            case CARD_YELLOW_GREEN:
                return YELLOW;
            case CARD_YELLOW_BROWN:
                return YELLOW;
            case CARD_ORANGE_RED:
                return ORANGE;
            case CARD_ORANGE_PURPLE:
                return ORANGE;
            case CARD_ORANGE_GREEN:
                return ORANGE;
            case CARD_ORANGE_BROWN:
                return ORANGE;
            case CARD_SKYBLUE_RED:
                return SKYBLUE;
            case CARD_SKYBLUE_PURPLE:
                return SKYBLUE;
            case CARD_SKYBLUE_GREEN:
                return SKYBLUE;
            case CARD_SKYBLUE_BROWN:
                return SKYBLUE;
            default:
                UNREACHABLE();
        }
    }
    else if (color_number == 2) {
        switch (tile_type) {
            case CARD_BLUE_RED:
                return RED;
            case CARD_BLUE_PURPLE:
                return PURPLE;
            case CARD_BLUE_GREEN:
                return GREEN;
            case CARD_BLUE_BROWN:
                return BROWN;
            case CARD_YELLOW_RED:
                return RED;
            case CARD_YELLOW_PURPLE:
                return PURPLE;
            case CARD_YELLOW_GREEN:
                return GREEN;
            case CARD_YELLOW_BROWN:
                return BROWN;
            case CARD_ORANGE_RED:
                return RED;
            case CARD_ORANGE_PURPLE:
                return PURPLE;
            case CARD_ORANGE_GREEN:
                return GREEN;
            case CARD_ORANGE_BROWN:
                return BROWN;
            case CARD_SKYBLUE_RED:
                return RED;
            case CARD_SKYBLUE_PURPLE:
                return PURPLE;
            case CARD_SKYBLUE_GREEN:
                return GREEN;
            case CARD_SKYBLUE_BROWN:
                return BROWN;
            default:
                UNREACHABLE();
        }
    }
    UNREACHABLE();
    return WHITE;   // Here to remove warnings
}

void update_game_logic(GameLogicData *game, GameAnimationsData *game_animations_data)
{
    if (is_token_placement_animation_running() == false || game_animations_data->card_stack_anim_data->is_done || game_animations_data->player_stack_anim_data->is_done) {
        game->stack_top_card = rendering_data->stack_top_card_ui;
    }

    if (game->game_state == GAME_STATE_PLAYING) {
        ASSERT(game != NULL, "GameLogicData should be initialized");

        Vec2i next_token_placement = get_next_token_placement(game);
        b32 sbd_want_to_play = next_token_placement.x != -1 && next_token_placement.y != -1;

        if (sbd_want_to_play) {
            Tile board_card = game->board[next_token_placement.x][next_token_placement.y];

            // Add the clicked card to the pile
            rendering_data->stack_top_card_ui = board_card;

            // Update the board
            if (game->current_player == PLAYER1) {
                game->board[next_token_placement.x][next_token_placement.y].type = TOKEN_PLAYER1;
            }
            else if (game->current_player == PLAYER2) {
                game->board[next_token_placement.x][next_token_placement.y].type = TOKEN_PLAYER2;
            }
            else {
                UNREACHABLE();
            }

            trace_log(LOG_INFO, "Player %d put his token on {row: %d, col: %d}", game->current_player, next_token_placement.y + 1, next_token_placement.x + 1);

            start_card_stack_anim(game_animations_data, board_card.type, (Vec2i){next_token_placement.x, next_token_placement.y});
            start_player_stack_anim(game_animations_data, game, game->current_player, (Vec2i){next_token_placement.x, next_token_placement.y});

            // rendering_data->board.pos.x = rendering_data->board.game_pos_x;
            game->first_turn = false;
            game->info_message_p1.display_time = 0.0f;
            game->info_message_p2.display_time = 0.0f;

            // update player stack
            if (game->current_player == PLAYER1) {
                game->player1_remaining_tokens--;
            }
            else if (game->current_player == PLAYER2) {
                game->player2_remaining_tokens--;
            }
            else {
                UNREACHABLE();
            }

            // Check for a winner
            if (is_winner(game->current_player, game)) {
                trace_log(LOG_INFO, "Player %d wins!", game->current_player);
                game->game_state = GAME_STATE_WIN;
                return;
            }
            // check for draw
            if (is_board_full(game->board)) {
                trace_log(LOG_INFO, "It is a draw");
                game->game_state = GAME_STATE_DRAW;
                return;
            }

            // Switch player
            if (game->current_player == PLAYER1) {
                if (game->mode == MODE_ONE_PLAYER) {
                    game->ai_thinking_duration = 4.0f;
                }
                game->current_player = PLAYER2;
            }
            else if (game->current_player == PLAYER2) {
                game->current_player = PLAYER1;
            }
            else {
                UNREACHABLE();
            }
        }

        // Reduce ai waiting duration
        if (game->ai_thinking_duration > 0.0f) {
            game->ai_thinking_duration -= get_frame_time();
        }
        if (game->info_message_p1.display_time > 0.0f) {
            game->info_message_p1.display_time -= get_frame_time();
        }
        if (game->info_message_p2.display_time > 0.0f) {
            game->info_message_p2.display_time -= get_frame_time();
        }

        update_tiles_pressed_state(game->board);
    }
    else if (game->game_state == GAME_STATE_WIN || game->game_state == GAME_STATE_DRAW) {
        if (is_released(game->home_button.is_pressed)) {
            game->order = GO_TO_MENU;
            game->home_button.is_pressed = false;
        }
        else if (is_released(game->restart_button.is_pressed)) {
            game->order = RESTART_GAME;
            game->restart_button.is_pressed = false;
        }

        update_pressable_object_state(game->restart_button.square, &game->restart_button.is_pressed);
        update_pressable_object_state(game->home_button.square, &game->home_button.is_pressed);
    }
    else {
        UNREACHABLE();
    }
}
