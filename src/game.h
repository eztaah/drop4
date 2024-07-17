#ifndef GAME_H
#define GAME_H

#include "application.h"

#define BOARD_ROWS_NB 4
#define BOARD_COLUMNS_NB 4
#define PLAYER_STACK_TOKENS_SLOTS 8

#define rendering_data game_global_rendering_data

typedef enum {
    CARD_BLUE_RED,
    CARD_BLUE_PURPLE,
    CARD_BLUE_GREEN,
    CARD_BLUE_BROWN,

    CARD_YELLOW_RED,
    CARD_YELLOW_PURPLE,
    CARD_YELLOW_GREEN,
    CARD_YELLOW_BROWN,

    CARD_ORANGE_RED,
    CARD_ORANGE_PURPLE,
    CARD_ORANGE_GREEN,
    CARD_ORANGE_BROWN,

    CARD_SKYBLUE_RED,
    CARD_SKYBLUE_PURPLE,
    CARD_SKYBLUE_GREEN,
    CARD_SKYBLUE_BROWN,

    TOKEN_PLAYER1,
    TOKEN_PLAYER2,

    EMPTY_TILE,
} TileType;

typedef struct {
    TileType type;
    b32 is_pressed;
} Tile;

typedef enum {
    PLAYER1 = 1,
    PLAYER2 = 2
} Player;

typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_WIN,
    GAME_STATE_DRAW,
} GameState;

typedef enum {
    RESTART_GAME,
    GO_TO_MENU,
    ORDER_NONE,
} GameOrder;

typedef struct {
    char message[256];
    f32 display_time;
} InfoMessage;

typedef struct {
    GameMode mode;
    GameState game_state;
    GameOrder order;

    Tile board[BOARD_ROWS_NB][BOARD_COLUMNS_NB];
    Tile stack_top_card;

    Player current_player;
    b32 first_turn;
    i32 player1_remaining_tokens;
    i32 player2_remaining_tokens;

    f32 ai_thinking_duration;

    InfoMessage info_message_p1;
    InfoMessage info_message_p2;

    Button home_button;
    Button restart_button;
} GameLogicData;

typedef struct {
    TileType tile_type;
    Square square;
    Square end_square;

    Vec2f end_speed;
    b32 end_speed_calculated;

    b32 is_done;
    // b32 is_active;
} AnimationData;

/**
 * This struct allows us to put rendering data at a global scope, we need to use that for animations or relative positions
 */
typedef struct {
    i32 tile_size;
    i32 tile_spacing;
    i32 padding;
    i32 size;
    f32 game_pos_x;
    Vec2f pos;
} BoardGlobalRenderingData;
typedef struct {
    Vec2f pos;
} CardStackGlobalRenderingData;
typedef struct {
    i32 token_size;
    i32 token_spacing;
    i32 stack_width;
    f32 pos_x;
    f32 p1_pos_y;
    f32 p2_pos_y;
} PlayersStackGlobalRenderingData;
typedef struct {
    i32 font_size;
    i32 p1_zoom_value;
    i32 p2_zoom_value;
    i32 distance_from_player_stack;
    f32 p1_pos_y;
    f32 p2_pos_y;
} PlayerTextGlobalRenderingData;
typedef struct {
    // Board rendering data
    BoardGlobalRenderingData board;

    // Card stack pos
    CardStackGlobalRenderingData card_stack;

    // Remaining token rendering data
    PlayersStackGlobalRenderingData players_stack;

    // Player text
    PlayerTextGlobalRenderingData players_text;

    Tile stack_top_card_ui;

} GameGlobalRenderingData;

typedef struct {
    AnimationData *card_stack_anim_data;
    AnimationData *player_stack_anim_data;
} GameAnimationsData;

extern GameLogicData *game_logic_data;
extern GameGlobalRenderingData *game_global_rendering_data;
extern GameAnimationsData *game_global_animations_data;

Color get_tile_color(const TileType tile_type, const i32 color_number);
Vec2i get_ai_pressed_tile(Tile board[][BOARD_COLUMNS_NB], const Tile stack_top_card);
b32 have_common_color(const TileType tile1_type, const TileType tile2_type);
b32 is_board_full(const Tile board[][BOARD_COLUMNS_NB]);

// animations
void update_animation(AnimationData *animation_data);
void end_token_placement_animation(void);
void start_card_stack_anim(GameAnimationsData *game_animations_data, TileType card, Vec2i tile_board_pos);
void start_player_stack_anim(GameAnimationsData *game_animations_data, const GameLogicData *game, Player player, Vec2i tile_board_pos);
b32 is_token_placement_animation_running(void);
void update_board_pos(void);

void update_game_logic(GameLogicData *game, GameAnimationsData *game_animations_data);
void update_game_rendering(GameAnimationsData *game_animations_data, const GameLogicData *game);
void draw_game(const GameLogicData *game, const GameAnimationsData *game_animations_data);
void instanciate_game(const GameMode mode);
void exit_game(void);

#endif
