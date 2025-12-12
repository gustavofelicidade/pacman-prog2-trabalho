#include "game.h"
#include "raylib.h"

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pac-Man Prog II");
    InitAudioDevice();
    SetTargetFPS(60);

    GameState game;
    if (!game_init(&game, "assets/maps/mapa1.txt", 4)) {
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }

    while (!WindowShouldClose() && game.running) {
        if (IsKeyPressed(KEY_F)) {
            ToggleFullscreen();
            if (!IsWindowFullscreen()) {
                SetWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
            }
        }

        float dt = GetFrameTime();
        game_update(&game, dt);

        BeginDrawing();
        ClearBackground(BLACK);
        game_draw(&game);
        EndDrawing();
    }

    game_shutdown(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
