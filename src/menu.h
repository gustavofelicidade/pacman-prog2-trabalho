#pragma once

#include <stdbool.h>

typedef enum {
    MENU_HIDDEN = 0,
    MENU_OPEN
} MenuStatus;

typedef enum {
    MENU_ACTION_NONE = 0,
    MENU_ACTION_NEW_GAME,
    MENU_ACTION_LOAD,
    MENU_ACTION_SAVE,
    MENU_ACTION_QUIT,
    MENU_ACTION_RESUME
} MenuAction;

typedef struct {
    MenuStatus status;
    int selectedIndex;
    MenuAction pendingAction;
} MenuState;

void menu_open(MenuState* menu);
void menu_close(MenuState* menu);
void menu_next(MenuState* menu);
void menu_prev(MenuState* menu);
void menu_commit(MenuState* menu);
