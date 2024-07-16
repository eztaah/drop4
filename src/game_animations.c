#include "game.h"

static AnimationData *start_animation(TileType tile_type, Square start_square, Square end_square)
{
    // we create an animation_object
    AnimationData *animation_data = (AnimationData *)calloc(1, sizeof(AnimationData));
    if ((animation_data) == NULL) {
        application_panic(__FILE__, __LINE__, "Memory allocation failed");
    }

    // initialize the values
    animation_data->tile_type = tile_type;
    animation_data->square = start_square;
    animation_data->end_square = end_square;

    animation_data->end_speed = (Vec2f){0, 0};
    animation_data->end_speed_calculated = false;

    animation_data->is_done = false;

    return animation_data;
}

void update_board_pos(void)
{
    f32 token_pos_center = rendering_data->board.pos.x + rendering_data->board.size / 2;
    f32 end_pos_center = rendering_data->board.game_pos_x + rendering_data->board.size / 2;

    f32 dist_x = end_pos_center - token_pos_center;

    rendering_data->board.pos.x += dist_x / 15.0f;

    // stop condition
    if (fabs(dist_x) < 1) {
        rendering_data->board.pos.x = rendering_data->board.game_pos_x;
    }
}

void update_animation(AnimationData *animation_data)
{
    Vec2f token_pos_center = {
        animation_data->square.x + animation_data->square.size / 2,
        animation_data->square.y + animation_data->square.size / 2};

    Vec2f end_pos_center = {
        animation_data->end_square.x + animation_data->square.size / 2,
        animation_data->end_square.y + animation_data->square.size / 2};

    f32 dist_x = end_pos_center.x - token_pos_center.x;
    f32 dist_y = end_pos_center.y - token_pos_center.y;

    i32 size_mismatch = animation_data->end_square.size - animation_data->square.size;

    // Calculation of the estimated number of remaining frames to reach the destination
    f32 frames_to_end = fmax(fabs(dist_x), fabs(dist_y)) / (animation_data->end_square.size / 15.0f);
    frames_to_end = fmax(frames_to_end, 1); // Éviter la division par zéro

    if (fabs(dist_x) > 10 || fabs(dist_y) > 10) {
        animation_data->square.x += dist_x / 15.0f;
        animation_data->square.y += dist_y / 15.0f;
    }
    else {
        if (!animation_data->end_speed_calculated) {
            animation_data->end_speed_calculated = true;
            animation_data->end_speed.x = dist_x / 15.0f;
            animation_data->end_speed.y = dist_y / 15.0f;
        }
        animation_data->square.x += animation_data->end_speed.x;
        animation_data->square.y += animation_data->end_speed.y;
    }

    // Adjust the token size so that it ends at the same time as the movement
    if (animation_data->square.size < animation_data->end_square.size) {
        animation_data->square.size += size_mismatch / (frames_to_end * 0.25);
    }

    // stop condition
    if (fabs(dist_x) < 1 && fabs(dist_y) < 1) {
        animation_data->is_done = true;
    }
}

b32 is_token_placement_animation_running(void)
{
    return game_global_animations_data->card_stack_anim_data != NULL || game_global_animations_data->player_stack_anim_data != NULL;
}

void end_token_placement_animation(void)
{
    free(game_global_animations_data->card_stack_anim_data);
    free(game_global_animations_data->player_stack_anim_data);
    game_global_animations_data->card_stack_anim_data = NULL;
    game_global_animations_data->player_stack_anim_data = NULL;
}

void start_card_stack_anim(GameAnimationsData *game_animations_data, TileType card, Vec2i tile_board_pos)
{
    // get start pos
    const Vec2f first_card_pos = {rendering_data->board.pos.x + rendering_data->board.padding, rendering_data->board.pos.y + rendering_data->board.padding};
    Square start_square = (Square){
        first_card_pos.x + tile_board_pos.x * (rendering_data->board.tile_size + rendering_data->board.tile_spacing),
        first_card_pos.y + tile_board_pos.y * (rendering_data->board.tile_size + rendering_data->board.tile_spacing),
        rendering_data->board.tile_size};

    // calculate end pos
    Square end_square = (Square){
        rendering_data->card_stack.pos.x,
        rendering_data->card_stack.pos.y,
        rendering_data->board.tile_size};

    game_animations_data->card_stack_anim_data = start_animation(card, start_square, end_square);
}

void start_player_stack_anim(GameAnimationsData *game_animations_data, const GameLogicData *game, Player player, Vec2i tile_board_pos)
{
    TileType token;

    // calculate the start square
    Square start_square;
    if (player == PLAYER1) {
        start_square.x = rendering_data->players_stack.pos_x + (game->player1_remaining_tokens - 1) * (rendering_data->players_stack.token_size + rendering_data->players_stack.token_spacing);
        start_square.y = rendering_data->players_stack.p1_pos_y;
        token = TOKEN_PLAYER1;
    }
    else if (player == PLAYER2) {
        start_square.x = rendering_data->players_stack.pos_x + (game->player2_remaining_tokens - 1) * (rendering_data->players_stack.token_size + rendering_data->players_stack.token_spacing);
        start_square.y = rendering_data->players_stack.p2_pos_y;
        token = TOKEN_PLAYER2;
    }
    else {
        UNREACHABLE();
        token = TOKEN_PLAYER1;
    }
    start_square.size = rendering_data->players_stack.token_size;

    // calculate the end square
    f32 board_pos_x;
    if (game->first_turn) {
        board_pos_x = rendering_data->board.game_pos_x;
    }
    else {
        board_pos_x = rendering_data->board.pos.x;
    }
    const Vec2f first_card_pos = {board_pos_x + rendering_data->board.padding, rendering_data->board.pos.y + rendering_data->board.padding};
    Square end_square = (Square){
        first_card_pos.x + tile_board_pos.x * (rendering_data->board.tile_size + rendering_data->board.tile_spacing),
        first_card_pos.y + tile_board_pos.y * (rendering_data->board.tile_size + rendering_data->board.tile_spacing),
        rendering_data->board.tile_size};

    game_animations_data->player_stack_anim_data = start_animation(token, start_square, end_square);
}
