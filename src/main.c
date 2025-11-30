#include "game.h"
#include "raylib.h"

int main(void) {
    GameState game;
    if (!game_init(&game, "assets/maps/mapa1.txt", 4)) {
        return 1;
    }

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pac-Man Prog II");
    SetTargetFPS(60);

    while (!WindowShouldClose() && game.running) {
        float dt = GetFrameTime();
        game_update(&game, dt);

        BeginDrawing();
        ClearBackground(BLACK);
        game_draw(&game);
        EndDrawing();
    }

    CloseWindow();
    game_shutdown(&game);
    return 0;
}
