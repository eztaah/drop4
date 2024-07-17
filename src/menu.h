#ifndef MENU_H
#define MENU_H

#include "application.h"

typedef enum {
    PAGE_MAIN_MENU,
    PAGE_CHOOSE_MODE,
} Page;

typedef enum {
    START_ONE_PLAYER_GAME,
    START_TWO_PLAYER_GAME,
    NONE,
} MenuOrder;

typedef struct {
    Page page;
    MenuOrder order;

    Texture rules_image;
    Texture game_overview_image;

    Button play_button;
    Button one_player_button;
    Button two_players_button;
} Menu;

extern Menu *menu;

void update_menu(Menu *menu);
void draw_menu(const Menu *menu);
void instanciate_menu(void);
void exit_menu(void);

#endif
