#include "raylib.h"
#include <string.h>

int main(void) {
    const int largura = 800;
    const int altura = 450;
    char texto[64] = "Pressione uma seta";

    InitWindow(largura, altura, "Teste Raylib");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_RIGHT)) strcpy(texto, "Direita");
        if (IsKeyPressed(KEY_LEFT)) strcpy(texto, "Esquerda");
        if (IsKeyPressed(KEY_UP)) strcpy(texto, "Cima");
        if (IsKeyPressed(KEY_DOWN)) strcpy(texto, "Baixo");

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText(texto, 240, 200, 40, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
