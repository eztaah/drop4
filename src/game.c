#include "game.h"

GameLogicData *game_logic_data = NULL;
GameGlobalRenderingData *game_global_rendering_data = NULL;
GameAnimationsData *game_global_animations_data = NULL;

static void init_game_rendering_data(GameGlobalRenderingData *game_global_rendering_data)
{
    // Tile rendering data
    game_global_rendering_data->board.tile_size = 124;
    game_global_rendering_data->board.tile_spacing = 30;

    // Board rendering data
    game_global_rendering_data->board.padding = 34;
    game_global_rendering_data->board.size = game_global_rendering_data->board.tile_size * 4 + 3 * game_global_rendering_data->board.tile_spacing + 2 * game_global_rendering_data->board.padding;
    game_global_rendering_data->board.game_pos_x = 20;
    game_global_rendering_data->board.pos = (Vec2f){
        WINDOW_WIDTH / 2 - game_global_rendering_data->board.size / 2,
        WINDOW_HEIGHT / 2 - game_global_rendering_data->board.size / 2};

    // Card stack data
    game_global_rendering_data->card_stack.pos = (Vec2f){
        20 + game_global_rendering_data->board.size + 54,
        WINDOW_HEIGHT / 2 - game_global_rendering_data->board.tile_size / 2};

    // Remaining token rendering data
    game_global_rendering_data->players_stack.token_size = 60;
    game_global_rendering_data->players_stack.token_spacing = 20;
    game_global_rendering_data->players_stack.stack_width = (game_global_rendering_data->players_stack.token_size * PLAYER_STACK_TOKENS_SLOTS + game_global_rendering_data->players_stack.token_spacing * (PLAYER_STACK_TOKENS_SLOTS - 1));
    game_global_rendering_data->players_stack.pos_x = WINDOW_WIDTH / 2 - game_global_rendering_data->players_stack.stack_width / 2;
    game_global_rendering_data->players_stack.p1_pos_y = WINDOW_HEIGHT - game_global_rendering_data->players_stack.token_size - 50;
    game_global_rendering_data->players_stack.p2_pos_y = 50;

    // Player id text
    game_global_rendering_data->players_text.font_size = 60;
    game_global_rendering_data->players_text.p1_zoom_value = 0;
    game_global_rendering_data->players_text.p2_zoom_value = 0;
    game_global_rendering_data->players_text.distance_from_player_stack = 20;
    game_global_rendering_data->players_text.p1_pos_y = game_global_rendering_data->players_stack.p1_pos_y - game_global_rendering_data->players_text.font_size - game_global_rendering_data->players_text.distance_from_player_stack;
    game_global_rendering_data->players_text.p2_pos_y = game_global_rendering_data->players_stack.p2_pos_y + game_global_rendering_data->players_stack.token_size + game_global_rendering_data->players_text.distance_from_player_stack;

    rendering_data->stack_top_card_ui.type = EMPTY_TILE;
    rendering_data->stack_top_card_ui.is_pressed = false;
}

static void init_board(Tile board[][BOARD_COLUMNS_NB])
{
    // Create an array with all the cards
    const i32 board_tiles_number = BOARD_ROWS_NB * BOARD_COLUMNS_NB;
    TileType cards[board_tiles_number];
    for (i32 i = 0; i < board_tiles_number; i++) {
        cards[i] = (TileType)i;
    }

    // Shuffle the array with the cards
    for (i32 i = board_tiles_number - 1; i > 0; i--) {
        i32 j = rand() % (i + 1);
        TileType temp = cards[i];
        cards[i] = cards[j];
        cards[j] = temp;
    }

    // Put all the shuffled cards in the board
    i32 k = 0;
    for (i32 i = 0; i < 4; i++) {
        for (i32 j = 0; j < 4; j++) {
            board[i][j].type = cards[k++];
            board[i][j].is_pressed = false;
        }
    }
}

static void init_game_logic_data(GameLogicData *game, const BoardGlobalRenderingData board_rendering_data, const GameMode mode)
{
    // init game struct values
    game->mode = mode;

#ifdef DEV_SET_GAMEOVER
    game->game_state = GAME_STATE_DRAW;
#else
    game->game_state = GAME_STATE_PLAYING;
#endif

    game->order = ORDER_NONE;

    init_board(game->board);

    game->stack_top_card.type = EMPTY_TILE;
    game->stack_top_card.is_pressed = false;

    game->current_player = PLAYER1;
    game->first_turn = true;
    game->player1_remaining_tokens = PLAYER_STACK_TOKENS_SLOTS;
    game->player2_remaining_tokens = PLAYER_STACK_TOKENS_SLOTS;

    game->ai_thinking_duration = 0.0f;

    sprintf(game->info_message_p1.message, " ");
    game->info_message_p1.display_time = 0.0f;

    sprintf(game->info_message_p2.message, " ");
    game->info_message_p2.display_time = 0.0f;

    game->home_button.square = (Square){
        board_rendering_data.game_pos_x + board_rendering_data.size + 46,
        WINDOW_HEIGHT / 2 - 160 - 167,
        160};
    game->home_button.texture = load_texture("./src/assets/home.png");
    game->home_button.texture_padding = 34;
    game->home_button.is_pressed = false;

    game->restart_button.square = (Square){
        board_rendering_data.game_pos_x + board_rendering_data.size + 46,
        WINDOW_HEIGHT / 2 + 167,
        160,
    };
    game->restart_button.texture = load_texture("./src/assets/restart.png");
    game->restart_button.texture_padding = 34;
    game->restart_button.is_pressed = false;
}

static void init_game_animations_data(GameAnimationsData *game_animations_data)
{
    game_animations_data->card_stack_anim_data = NULL;
    game_animations_data->player_stack_anim_data = NULL;
}

void instanciate_game(const GameMode mode)
{
    // Init game global rendering data
    ALLOC_VAR(game_global_rendering_data, GameGlobalRenderingData);
    init_game_rendering_data(game_global_rendering_data);

    // Init game logic data
    ALLOC_VAR(game_logic_data, GameLogicData);
    init_game_logic_data(game_logic_data, game_global_rendering_data->board, mode);

    // Init game animations data
    ALLOC_VAR(game_global_animations_data, GameAnimationsData);
    init_game_animations_data(game_global_animations_data);
}

void exit_game(void)
{
    free(game_logic_data);
    free(game_global_rendering_data);
    free(game_global_animations_data);
}
