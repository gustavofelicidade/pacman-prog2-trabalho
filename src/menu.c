#include "menu.h"

void menu_open(MenuState* menu) {
    menu->status = MENU_OPEN;
    menu->selectedIndex = 0;
    menu->pendingAction = MENU_ACTION_NONE;
}

void menu_close(MenuState* menu) {
    menu->status = MENU_HIDDEN;
    menu->pendingAction = MENU_ACTION_NONE;
}

void menu_next(MenuState* menu) {
    menu->selectedIndex++;
}

void menu_prev(MenuState* menu) {
    if (menu->selectedIndex > 0) menu->selectedIndex--;
}

void menu_commit(MenuState* menu) {
    // TODO: map selectedIndex to MenuAction.
    menu->pendingAction = MENU_ACTION_NONE;
}
