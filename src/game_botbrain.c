
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

#include "game.h"

static i32 max(i32 a, i32 b)
{
    return (a > b) ? a : b;
}

static i32 min(i32 a, i32 b)
{
    return (a < b) ? a : b;
}

static i32 evaluate_line(const Tile board[][BOARD_COLUMNS_NB], i32 start_x, i32 start_y, i32 dx, i32 dy, TileType player)
{
    i32 count_player = 0;
    i32 count_opponent = 0;
    i32 count_empty = 0;

    // Determine who is the opponent
    TileType opponent = (player == TOKEN_PLAYER1) ? TOKEN_PLAYER2 : TOKEN_PLAYER1;

    // Count the number of empty tiles, player tokens, and AI tokens ON THE CURRENT LINE
    for (i32 i = 0; i < 4; ++i) {
        TileType type = board[start_x + i * dx][start_y + i * dy].type;
        if (type == player) {
            count_player++;
        }
        else if (type == opponent) {
            count_opponent++;
        }
        else if (type == EMPTY_TILE) {
            count_empty++;
        }
    }

    if (count_player == 4) {
        return 100; // Certain victory
    }
    else if (count_opponent == 4) {
        return -100; // Certain defeat
    }
    else if (count_player == 3 && count_empty == 1) {
        return 10;
    }
    else if (count_player == 2 && count_empty == 2) {
        return 5;
    }
    else if (count_opponent == 3 && count_empty == 1) {
        return -10; // Block the opponent
    }
    else if (count_opponent == 2 && count_empty == 2) {
        return -5; // Anticipate blocking the opponent
    }
    return 1;
}

static i32 evaluate_square(const Tile board[][BOARD_COLUMNS_NB], i32 start_x, i32 start_y, TileType player)
{
    i32 count_player = 0;
    i32 count_opponent = 0;
    i32 count_empty = 0;

    // Determine who is the opponent
    TileType opponent = (player == TOKEN_PLAYER1) ? TOKEN_PLAYER2 : TOKEN_PLAYER1;

    // Count the number of empty tiles, player tokens, and AI tokens ON THE CURRENT 2x2 SQUARE
    for (i32 dx = 0; dx < 2; ++dx) {
        for (i32 dy = 0; dy < 2; ++dy) {
            TileType type = board[start_x + dx][start_y + dy].type;
            if (type == player) {
                count_player++;
            }
            else if (type == opponent) {
                count_opponent++;
            }
            else if (type == EMPTY_TILE) {
                count_empty++;
            }
        }
    }

    if (count_player == 4) {
        return 100; // Make the AI prioritize winning moves
    }
    else if (count_opponent == 4) {
        return -100; // Make the AI avoid losing moves
    }
    else if (count_player == 3 && count_empty == 1) {
        return 10; // Encourage moves that set up future wins
    }
    else if (count_player == 2 && count_empty == 2) {
        return 5; // Encourage moves that make good positions
    }
    else if (count_opponent == 3 && count_empty == 1) {
        return -10; // Encourage blocking opponent's immediate win
    }
    else if (count_opponent == 2 && count_empty == 2) {
        return -5; // Encourage moves that block opponent's potential threats
    }
    return 1; // If we put 0, there is an error
}

/**
 * This function evaluates all the lines and columns of the board and adds or subtracts the score
 * The score thus corresponds to the "score of the board, is it a good board or not for the AI"
 * This function returns a score associated with a board
 */
static i32 evaluate_board(const Tile board[][BOARD_COLUMNS_NB], TileType player)
{
    i32 board_score = 0;

    // Check lines and columns
    for (i32 i = 0; i < BOARD_ROWS_NB; ++i) {
        board_score += evaluate_line(board, i, 0, 0, 1, player);
        board_score += evaluate_line(board, 0, i, 1, 0, player);
    }

    // Check diagonals
    board_score += evaluate_line(board, 0, 0, 1, 1, player);
    board_score += evaluate_line(board, 0, 3, 1, -1, player);

    // Check squares (4 possible configurations)
    for (i32 i = 0; i < BOARD_ROWS_NB - 1; ++i) {
        for (i32 j = 0; j < BOARD_COLUMNS_NB - 1; ++j) {
            board_score += evaluate_square(board, i, j, player);
        }
    }

    return board_score;
}

/**
 * Minimax function to evaluate the best move for the AI.
 * This function returns a score for the current situation of the game board.
 * It evaluates the score of the current board,
 */
#define MAX_DEPTH 16
static i32 minimax(Tile board[][BOARD_COLUMNS_NB], TileType stack_top_card, i32 depth, b32 is_maximizing, i32 alpha, i32 beta)
{
    i32 board_score_bot = evaluate_board(board, TOKEN_PLAYER2);
    i32 board_score_player1 = evaluate_board(board, TOKEN_PLAYER1);

    // CASE 1: The current game terrain is critical (immediate victory or defeat) or the board is full
    //      => We return, so we stop looking at all possible positions after

    // We modify the board score according to the number of moves to reach this board
    if (board_score_bot >= 100) {
        // Here we return a very good score if the depth is low
        return 100 - depth * 3; // Prefer quick victories
    }
    if (board_score_player1 >= 100) {
        return depth - 100 * 3; // Prefer slow defeats
    }

    // If the maximum depth is reached or if there are no more possible moves
    if (depth >= MAX_DEPTH || is_board_full(board)) {
        // Return the difference of scores to evaluate the current position.
        // This allows us to evaluate the quality of the position beyond terminal conditions.
        return board_score_bot - board_score_player1;
    }

    // CASE 2: The current terrain is not critical, so we will test all the following possible moves recursively

    // If it is the AI's turn, we go into positive mode
    if (is_maximizing) {
        // It's the AI's turn (maximizing player)
        i32 best = -INT_MAX; // Initialize the best score to the smallest possible value

        // Browse all the board squares to find the best move
        for (i32 i = 0; i < BOARD_ROWS_NB; i++) {
            for (i32 j = 0; j < BOARD_COLUMNS_NB; j++) {
                if (board[i][j].type != TOKEN_PLAYER1 && board[i][j].type != TOKEN_PLAYER2) {
                    if (have_common_color(board[i][j].type, stack_top_card)) {
                        TileType original = board[i][j].type;
                        board[i][j].type = TOKEN_PLAYER2;

                        // Recursive call to minimax to evaluate this position, changing the player
                        // We pass `false` to indicate that it will be the minimizing player's turn next.
                        best = max(best, minimax(board, original, depth + 1, false, alpha, beta));
                        board[i][j].type = original;

                        // Update alpha and perform an alpha-beta pruning if necessary
                        // If the best score obtained is higher than beta, we can stop considering other moves (pruning).
                        alpha = max(alpha, best);
                        if (beta <= alpha) {
                            // Perform an alpha-beta pruning
                            break;
                        }
                    }
                }
            }
        }
        return best;
    }

    // Otherwise, we go into negative mode
    else {
        // It's the opponent's turn (minimizing player)
        i32 best = INT_MAX; // Initialize the best score to the largest possible value

        // Browse all the board squares to find the best move
        for (i32 i = 0; i < BOARD_ROWS_NB; i++) {
            for (i32 j = 0; j < BOARD_COLUMNS_NB; j++) {
                if (board[i][j].type != TOKEN_PLAYER1 && board[i][j].type != TOKEN_PLAYER2) {
                    if (have_common_color(board[i][j].type, stack_top_card)) {
                        TileType original = board[i][j].type;
                        board[i][j].type = TOKEN_PLAYER1;

                        // Recursive call to minimax to evaluate this position, changing the player
                        // We pass `true` to indicate that it will be the maximizing player's turn next.
                        best = min(best, minimax(board, original, depth + 1, true, alpha, beta));
                        board[i][j].type = original;

                        // Update beta and perform an alpha-beta pruning if necessary
                        // If the best score obtained is lower than alpha, we can stop considering other moves (pruning).
                        beta = min(beta, best);
                        if (beta <= alpha)
                            break; // Perform an alpha-beta pruning
                    }
                }
            }
        }
        return best;
    }
}

/**
 * This function takes the game board and the stack as arguments, it returns the best position for the next move
 * This function returns the coordinates of a tile on the board
 */
static Vec2i find_best_move(Tile board[][BOARD_COLUMNS_NB], TileType stack_top_card)
{
    i32 best_value = -INT_MAX;
    Vec2i best_move = {-1, -1};

    // We browse each card of the board, if it is valid, then we launch the minmax function to know the score associated with this tile
    for (i32 i = 0; i < BOARD_ROWS_NB; i++) {
        for (i32 j = 0; j < BOARD_COLUMNS_NB; j++) {
            if (board[i][j].type != TOKEN_PLAYER1 && board[i][j].type != TOKEN_PLAYER2) {
                if (have_common_color(board[i][j].type, stack_top_card)) {
                    trace_log(LOG_DEBUG, "\nChecking tile {row: %d, col: %d} score...", j + 1, i + 1);
                    TileType original = board[i][j].type;

                    // Act as if the AI had played on this square, and analyze the situation with minimax
                    board[i][j].type = TOKEN_PLAYER2;

                    // This function will associate a score with a playable tile
                    i32 move_value = minimax(board, original, 0, false, -INT_MAX, INT_MAX); // Change stack_top_card to original
                    trace_log(LOG_DEBUG, "    > The move value for this tile is %d", move_value);
                    board[i][j].type = original;

                    if (move_value > best_value) {
                        best_move.x = i;
                        best_move.y = j;
                        best_value = move_value;
                    }
                }
            }
        }
    }

    return best_move;
}

// Function to get the best move for the AI
Vec2i get_ai_pressed_tile(Tile board[][BOARD_COLUMNS_NB], const Tile stack_top_card)
{
    Vec2i best_move = find_best_move(board, stack_top_card.type);
    trace_log(LOG_DEBUG, "best move : {%d, %d}", best_move.x, best_move.y);
    return (Vec2i){best_move.x, best_move.y};
}
