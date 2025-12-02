#include "menu.h"

static const MenuAction kMenuActions[] = {
    MENU_ACTION_NEW_GAME,
    MENU_ACTION_LOAD,
    MENU_ACTION_SAVE,
    MENU_ACTION_QUIT,
    MENU_ACTION_RESUME
};

static int menu_item_count(void) {
    return (int)(sizeof(kMenuActions) / sizeof(kMenuActions[0]));
}

static MenuAction action_from_index(int index) {
    if (index < 0) index = 0;
    int count = menu_item_count();
    if (index >= count) index = count - 1;
    return kMenuActions[index];
}

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
    int count = menu_item_count();
    if (menu->selectedIndex < count - 1) {
        menu->selectedIndex++;
    }
}

void menu_prev(MenuState* menu) {
    if (menu->selectedIndex > 0) menu->selectedIndex--;
}

void menu_commit(MenuState* menu) {
    menu->pendingAction = action_from_index(menu->selectedIndex);
}
