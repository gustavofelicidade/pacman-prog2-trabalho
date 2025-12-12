#include "game.h"
#include "render.h"
#include "save.h"
#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#define SAVE_FILE_PATH "savegame.sav"
#define RANKING_FILE_PATH "ranking.dat"

static void return_to_title(GameState* game);
static void open_ranking_screen(GameState* game);

static void set_hud_message(GameState* game, const char* text, float duration) {
    if (!text) return;
    snprintf(game->hudMessage, sizeof(game->hudMessage), "%s", text);
    game->hudMessageTimer = duration;
}

static void update_hud_message(GameState* game, float dt) {
    if (game->hudMessageTimer > 0.0f) {
        game->hudMessageTimer -= dt;
        if (game->hudMessageTimer < 0.0f) {
            game->hudMessageTimer = 0.0f;
            game->hudMessage[0] = '\0';
        }
    }
}

static int manhattan(Position a, Position b) {
    int dr = a.row - b.row;
    if (dr < 0) dr = -dr;
    int dc = a.col - b.col;
    if (dc < 0) dc = -dc;
    return dr + dc;
}

static Direction opposite(Direction dir) {
    switch (dir) {
        case DIR_UP: return DIR_DOWN;
        case DIR_DOWN: return DIR_UP;
        case DIR_LEFT: return DIR_RIGHT;
        case DIR_RIGHT: return DIR_LEFT;
        default: return DIR_NONE;
    }
}

static Position next_position(Position pos, Direction dir) {
    switch (dir) {
        case DIR_UP: pos.row -= 1; break;
        case DIR_DOWN: pos.row += 1; break;
        case DIR_LEFT: pos.col -= 1; break;
        case DIR_RIGHT: pos.col += 1; break;
        default: break;
    }
    return pos;
}

static bool can_move_to(const Map* map, Position pos, Direction dir) {
    if (dir == DIR_NONE) return false;
    Position next = next_position(pos, dir);
    if (!map_in_bounds(map, next.row, next.col)) return false;
    char cell = map_get(map, next.row, next.col);
    return cell != '#';
}

static bool same_position(Position a, Position b) {
    return a.row == b.row && a.col == b.col;
}

static void teleport_if_portal(Position* pos, const Map* map) {
    if (map_get(map, pos->row, pos->col) != 'T' || map->portalCount < 2) return;
    Position current = *pos;
    Position fallback = current;
    for (int i = 0; i < map->portalCount; i++) {
        Position portal = map->portals[i];
        if (same_position(portal, current)) continue;
        if (portal.row == current.row || portal.col == current.col) {
            *pos = portal;
            return;
        }
        fallback = portal;
    }
    if (!same_position(fallback, current)) {
        *pos = fallback;
    }
}

static void trigger_end_state(GameState* game, bool victory) {
    game->paused = true;
    game->pacman.dir = DIR_NONE;
    game->pacman.pendingDir = DIR_NONE;
    const char* msg = victory ? "Todos os niveis concluidos!" : "Game Over!";
    int rankPos = ranking_position_for_score(&game->ranking, game->score);
    game->postPhase = victory ? GAME_PHASE_VICTORY : GAME_PHASE_GAMEOVER;
    if (rankPos >= 0) {
        game->phase = GAME_PHASE_ENTER_SCORE;
        game->pendingRankingScore = game->score;
        game->pendingRankingIndex = rankPos;
        game->nameEntryLen = 0;
        game->nameEntry[0] = '\0';
        set_hud_message(game, "Novo recorde! Digite seu nome e ENTER.", HUD_MESSAGE_TIME);
    } else {
        game->phase = game->postPhase;
        set_hud_message(game, msg, HUD_MESSAGE_TIME);
    }
}

static void activate_power_mode(GameState* game) {
    game->pacman.powered = true;
    game->pacman.powerTimeLeft = POWER_MODE_DURATION;
    if (!game->ghosts) return;
    for (int i = 0; i < game->ghostCount; i++) {
        game->ghosts[i].vulnerable = true;
        game->ghosts[i].vulnerableTimeLeft = POWER_MODE_DURATION;
    }
}

static void update_power_mode(GameState* game, float dt) {
    if (game->pacman.powered) {
        game->pacman.powerTimeLeft -= dt;
        if (game->pacman.powerTimeLeft <= 0.0f) {
            game->pacman.powered = false;
            game->pacman.powerTimeLeft = 0.0f;
        }
    }
    if (!game->ghosts) return;
    for (int i = 0; i < game->ghostCount; i++) {
        Ghost* ghost = &game->ghosts[i];
        if (!ghost->vulnerable) continue;
        ghost->vulnerableTimeLeft -= dt;
        if (ghost->vulnerableTimeLeft <= 0.0f) {
            ghost->vulnerable = false;
            ghost->vulnerableTimeLeft = 0.0f;
        }
    }
}

static void handle_tile(GameState* game) {
    Map* map = &game->map;
    Pacman* pac = &game->pacman;
    char cell = map_get(map, pac->pos.row, pac->pos.col);
    switch (cell) {
        case '.':
            map_set(map, pac->pos.row, pac->pos.col, ' ');
            game->score += 10;
            if (game->pelletsRemaining > 0) game->pelletsRemaining--;
            if (map->pelletsRemaining > 0) map->pelletsRemaining--;
            break;
        case 'o':
            map_set(map, pac->pos.row, pac->pos.col, ' ');
            game->score += 50;
            if (game->pelletsRemaining > 0) game->pelletsRemaining--;
            if (map->pelletsRemaining > 0) map->pelletsRemaining--;
            activate_power_mode(game);
            break;
        default:
            break;
    }
}

static void handle_pacman_hit(GameState* game) {
    if (game->score >= 200) {
        game->score -= 200;
    } else {
        game->score = 0;
    }
    if (game->lives > 0) game->lives--;
    if (game->lives <= 0) {
        trigger_end_state(game, false);
        return;
    }
    game->pacman.pos = game->map.pacmanStart;
    game->pacman.dir = DIR_NONE;
    game->pacman.pendingDir = DIR_NONE;
    game->pacman.powered = false;
    game->pacman.powerTimeLeft = 0.0f;
    game->pacman.moveTimer = 0.0f;
    if (!game->ghosts || !game->map.ghostStarts) return;
    for (int i = 0; i < game->ghostCount; i++) {
        Ghost* ghost = &game->ghosts[i];
        if (!ghost->alive) continue;
        ghost->pos = game->map.ghostStarts[i];
        ghost->dir = DIR_NONE;
        ghost->vulnerable = false;
        ghost->vulnerableTimeLeft = 0.0f;
        ghost->moveTimer = 0.0f;
    }
}

static void handle_collisions(GameState* game) {
    if (!game->ghosts) return;
    for (int i = 0; i < game->ghostCount; i++) {
        Ghost* ghost = &game->ghosts[i];
        if (!ghost->alive) continue;
        if (!same_position(ghost->pos, game->pacman.pos)) continue;
        if (game->pacman.powered && ghost->vulnerable) {
            ghost->alive = false;
            ghost->vulnerable = false;
            ghost->vulnerableTimeLeft = 0.0f;
            game->score += 100;
        } else {
            handle_pacman_hit(game);
            return;
        }
    }
}

static Direction read_pacman_input(void) {
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) return DIR_RIGHT;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) return DIR_LEFT;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) return DIR_UP;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) return DIR_DOWN;
    return DIR_NONE;
}

static bool advance_pacman_step(GameState* game) {
    Pacman* pac = &game->pacman;
    if (pac->pendingDir != DIR_NONE && can_move_to(&game->map, pac->pos, pac->pendingDir)) {
        pac->dir = pac->pendingDir;
    }
    if (pac->dir == DIR_NONE) return false;
    if (!can_move_to(&game->map, pac->pos, pac->dir)) {
        pac->dir = DIR_NONE;
        return false;
    }
    pac->pos = next_position(pac->pos, pac->dir);
    teleport_if_portal(&pac->pos, &game->map);
    handle_tile(game);
    handle_collisions(game);
    return true;
}

static void update_pacman(GameState* game, float dt) {
    Direction input = read_pacman_input();
    if (input != DIR_NONE) {
        game->pacman.pendingDir = input;
    }
    game->pacman.moveTimer += dt;
    while (game->pacman.moveTimer >= PACMAN_STEP_INTERVAL) {
        if (!advance_pacman_step(game)) break;
        game->pacman.moveTimer -= PACMAN_STEP_INTERVAL;
    }
}

static Direction choose_ghost_direction(const GameState* game, const Ghost* ghost) {
    const Map* map = &game->map;
    Direction options[4];
    int count = 0;
    Direction all[4] = {DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};
    for (int i = 0; i < 4; i++) {
        if (can_move_to(map, ghost->pos, all[i])) {
            options[count++] = all[i];
        }
    }
    if (count == 0) return DIR_NONE;
    Direction current = ghost->dir;
    bool canContinue = (current != DIR_NONE) && can_move_to(map, ghost->pos, current);
    if (canContinue && count <= 2) {
        return current;
    }
    Direction oppositeDir = opposite(current);
    Direction filtered[4];
    int filteredCount = 0;
    for (int i = 0; i < count; i++) {
        Direction dir = options[i];
        if (count > 1 && dir == oppositeDir) continue;
        filtered[filteredCount++] = dir;
    }
    if (filteredCount == 0) {
        filteredCount = count;
        for (int i = 0; i < count; i++) filtered[i] = options[i];
    }

    bool chase = !game->pacman.powered;
    Direction bestDir = DIR_NONE;
    int bestScore = chase ? INT_MAX : INT_MIN;

    for (int i = 0; i < filteredCount; i++) {
        Direction dir = filtered[i];
        Position next = next_position(ghost->pos, dir);
        int dist = manhattan(next, game->pacman.pos);
        int score = chase ? dist : -dist;
        if ((chase && score < bestScore) || (!chase && score > bestScore)) {
            bestScore = score;
            bestDir = dir;
        }
    }

    if (bestDir == DIR_NONE) {
        int idx = GetRandomValue(0, filteredCount - 1);
        bestDir = filtered[idx];
    }
    return bestDir;
}

static void update_ghosts(GameState* game, float dt) {
    if (!game->ghosts) return;
    for (int i = 0; i < game->ghostCount; i++) {
        Ghost* ghost = &game->ghosts[i];
        if (!ghost->alive) continue;
        float interval = ghost->vulnerable ? GHOST_STEP_INTERVAL_VULNERABLE : GHOST_STEP_INTERVAL;
        ghost->moveTimer += dt;
        while (ghost->moveTimer >= interval) {
            ghost->moveTimer -= interval;
            Direction nextDir = choose_ghost_direction(game, ghost);
            if (nextDir == DIR_NONE) break;
            ghost->dir = nextDir;
            ghost->pos = next_position(ghost->pos, ghost->dir);
            teleport_if_portal(&ghost->pos, &game->map);
            handle_collisions(game);
        }
    }
}

static bool load_level_number(GameState* game, int level) {
    char path[64];
    snprintf(path, sizeof(path), "assets/maps/mapa%d.txt", level);
    if (!game_load_level(game, path)) {
        return false;
    }
    game->level = level;
    return true;
}

static void check_level_transition(GameState* game) {
    if (game->pelletsRemaining > 0) return;
    int nextLevel = game->level + 1;
    if (!load_level_number(game, nextLevel)) {
        trigger_end_state(game, true);
    }
}

static void start_new_game(GameState* game) {
    game->score = 0;
    game->lives = PACMAN_START_LIVES;
    if (!load_level_number(game, 1)) {
        game->running = false;
    } else {
        game->running = true;
        game->phase = GAME_PHASE_PLAYING;
        game->paused = false;
        set_hud_message(game, "Novo jogo iniciado!", HUD_MESSAGE_TIME);
    }
}

static void close_menu(GameState* game) {
    menu_close(&game->menu);
    game->paused = false;
}

static void execute_menu_action(GameState* game, MenuAction action) {
    switch (action) {
        case MENU_ACTION_NEW_GAME:
            start_new_game(game);
            close_menu(game);
            break;
        case MENU_ACTION_LOAD:
            if (load_game(game, SAVE_FILE_PATH)) {
                set_hud_message(game, "Jogo carregado.", HUD_MESSAGE_TIME);
                close_menu(game);
            } else {
                set_hud_message(game, "Falha ao carregar jogo.", HUD_MESSAGE_TIME);
            }
            break;
        case MENU_ACTION_SAVE:
            if (save_game(game, SAVE_FILE_PATH)) {
                set_hud_message(game, "Jogo salvo.", HUD_MESSAGE_TIME);
            } else {
                set_hud_message(game, "Erro ao salvar jogo.", HUD_MESSAGE_TIME);
            }
            break;
        case MENU_ACTION_QUIT:
            game->running = false;
            break;
        case MENU_ACTION_RESUME:
            close_menu(game);
            break;
        case MENU_ACTION_NONE:
        default:
            break;
    }
}

static void handle_menu_input(GameState* game) {
    MenuState* menu = &game->menu;
    if (IsKeyPressed(KEY_DOWN)) menu_next(menu);
    if (IsKeyPressed(KEY_UP)) menu_prev(menu);
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) menu_commit(menu);
    if (IsKeyPressed(KEY_N)) menu->pendingAction = MENU_ACTION_NEW_GAME;
    if (IsKeyPressed(KEY_C)) menu->pendingAction = MENU_ACTION_LOAD;
    if (IsKeyPressed(KEY_S)) menu->pendingAction = MENU_ACTION_SAVE;
    if (IsKeyPressed(KEY_Q)) menu->pendingAction = MENU_ACTION_QUIT;
    if (IsKeyPressed(KEY_V)) menu->pendingAction = MENU_ACTION_RESUME;
    if (menu->pendingAction != MENU_ACTION_NONE) {
        execute_menu_action(game, menu->pendingAction);
        menu->pendingAction = MENU_ACTION_NONE;
    }
}

static void handle_end_screen_input(GameState* game) {
    if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ENTER)) {
        start_new_game(game);
        menu_close(&game->menu);
    }
    if (IsKeyPressed(KEY_Q)) {
        game->running = false;
    }
    if (IsKeyPressed(KEY_R)) {
        open_ranking_screen(game);
    }
    if (IsKeyPressed(KEY_T)) {
        return_to_title(game);
    }
}

static void return_to_title(GameState* game) {
    menu_close(&game->menu);
    game->phase = GAME_PHASE_TITLE;
    game->paused = true;
}

static void open_ranking_screen(GameState* game) {
    game->phase = GAME_PHASE_RANKING;
    game->paused = true;
}

static void finalize_name_entry(GameState* game, bool commit) {
    if (commit) {
        const char* name = (game->nameEntryLen > 0) ? game->nameEntry : "PLAYER";
        ranking_insert(&game->ranking, name, game->pendingRankingScore);
        if (ranking_save(&game->ranking, RANKING_FILE_PATH)) {
            set_hud_message(game, "Ranking atualizado!", HUD_MESSAGE_TIME);
        } else {
            set_hud_message(game, "Falha ao salvar ranking.", HUD_MESSAGE_TIME);
        }
    } else {
        set_hud_message(game, "Registro ignorado.", HUD_MESSAGE_TIME);
    }
    game->pendingRankingScore = 0;
    game->pendingRankingIndex = -1;
    game->nameEntryLen = 0;
    game->nameEntry[0] = '\0';
    game->phase = game->postPhase;
}

static void handle_name_entry(GameState* game) {
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 126) {
            if (game->nameEntryLen < RANKING_NAME_LEN - 1) {
                game->nameEntry[game->nameEntryLen++] = (char)key;
                game->nameEntry[game->nameEntryLen] = '\0';
            }
        }
        key = GetCharPressed();
    }
    if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_DELETE)) && game->nameEntryLen > 0) {
        game->nameEntryLen--;
        game->nameEntry[game->nameEntryLen] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER)) {
        finalize_name_entry(game, true);
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        finalize_name_entry(game, false);
    }
}

static void handle_title_input(GameState* game) {
    if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ENTER)) {
        start_new_game(game);
        return;
    }
    if (IsKeyPressed(KEY_C)) {
        if (load_game(game, SAVE_FILE_PATH)) {
            set_hud_message(game, "Jogo carregado.", HUD_MESSAGE_TIME);
            game->phase = GAME_PHASE_PLAYING;
        } else {
            set_hud_message(game, "Nenhum save encontrado.", HUD_MESSAGE_TIME);
        }
        return;
    }
    if (IsKeyPressed(KEY_R)) {
        open_ranking_screen(game);
        return;
    }
    if (IsKeyPressed(KEY_Q)) {
        game->running = false;
    }
}

static void handle_ranking_input(GameState* game) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE) || IsKeyPressed(KEY_ENTER)) {
        return_to_title(game);
    }
    if (IsKeyPressed(KEY_N)) {
        start_new_game(game);
    }
    if (IsKeyPressed(KEY_Q)) {
        game->running = false;
    }
}

bool game_load_level(GameState* game, const char* mapPath) {
    map_free(&game->map);
    if (!map_load(&game->map, mapPath)) return false;

    game->pelletsRemaining = game->map.pelletsRemaining;
    game->powerTimer = 0.0f;
    snprintf(game->currentMapPath, sizeof(game->currentMapPath), "%s", mapPath);

    game->pacman.pos = game->map.pacmanStart;
    game->pacman.dir = DIR_NONE;
    game->pacman.pendingDir = DIR_NONE;
    game->pacman.powered = false;
    game->pacman.powerTimeLeft = 0.0f;
    game->pacman.moveTimer = 0.0f;

    free(game->ghosts);
    game->ghostCount = game->map.ghostCount;
    if (game->ghostCount > 0) {
        game->ghosts = (Ghost*)malloc(sizeof(Ghost) * game->ghostCount);
        for (int i = 0; i < game->ghostCount; i++) {
            game->ghosts[i].pos = game->map.ghostStarts[i];
            game->ghosts[i].dir = DIR_NONE;
            game->ghosts[i].vulnerable = false;
            game->ghosts[i].vulnerableTimeLeft = 0.0f;
            game->ghosts[i].alive = true;
            game->ghosts[i].moveTimer = 0.0f;
        }
    } else {
        game->ghosts = NULL;
    }

    game->phase = GAME_PHASE_PLAYING;
    game->paused = false;
    return true;
}

bool game_init(GameState* game, const char* firstMapPath, int ghostCount) {
    (void)ghostCount;
    game->level = 1;
    game->score = 0;
    game->lives = PACMAN_START_LIVES;
    game->paused = true;
    game->running = true;
    game->menu.status = MENU_HIDDEN;
    game->menu.pendingAction = MENU_ACTION_NONE;
    game->menu.selectedIndex = 0;
    game->ghosts = NULL;
    game->ghostCount = 0;
    game->pelletsRemaining = 0;
    game->powerTimer = 0.0f;
    memset(game->currentMapPath, 0, sizeof(game->currentMapPath));
    game->phase = GAME_PHASE_TITLE;
    game->postPhase = GAME_PHASE_TITLE;
    game->hudMessage[0] = '\0';
    game->hudMessageTimer = 0.0f;
    game->pendingRankingScore = 0;
    game->pendingRankingIndex = -1;
    game->nameEntryLen = 0;
    game->nameEntry[0] = '\0';

    ranking_init(&game->ranking);
    ranking_load(&game->ranking, RANKING_FILE_PATH);

    game->map = (Map){0};
    (void)firstMapPath;
    return true;
}

void game_shutdown(GameState* game) {
    free(game->ghosts);
    game->ghosts = NULL;
    map_free(&game->map);
}

void game_update(GameState* game, float dt) {
    update_hud_message(game, dt);

    switch (game->phase) {
        case GAME_PHASE_TITLE:
            handle_title_input(game);
            return;
        case GAME_PHASE_RANKING:
            handle_ranking_input(game);
            return;
        case GAME_PHASE_ENTER_SCORE:
            handle_name_entry(game);
            return;
        case GAME_PHASE_VICTORY:
        case GAME_PHASE_GAMEOVER:
            handle_end_screen_input(game);
            return;
        case GAME_PHASE_PLAYING:
        default:
            break;
    }

    if (IsKeyPressed(KEY_TAB)) {
        if (game->menu.status == MENU_OPEN) {
            close_menu(game);
        } else {
            menu_open(&game->menu);
            game->paused = true;
        }
    }

    if (game->menu.status == MENU_OPEN) {
        handle_menu_input(game);
        return;
    }

    update_power_mode(game, dt);
    update_pacman(game, dt);
    update_ghosts(game, dt);
    handle_collisions(game);
    check_level_transition(game);
}

void game_draw(const GameState* game) {
    switch (game->phase) {
        case GAME_PHASE_TITLE:
            render_title_screen(game);
            break;
        case GAME_PHASE_RANKING:
            render_ranking_screen(game);
            break;
        default:
            render_game(game);
            if (game->phase == GAME_PHASE_PLAYING) {
                render_menu(game);
            }
            break;
    }
}
