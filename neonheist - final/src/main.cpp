#include "raylib.h"
#include "GameManager.h"

int main() {
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(2880, 1920, "Neon Heist");
    SetWindowSize(1920, 1080);
    SetTargetFPS(60);

    GameManager gm;
    gm.startLevel();

    while (!WindowShouldClose()) {

        gm.update();

        if ((gm.gameOver || gm.gameWon) && IsKeyPressed(KEY_R)) {
            gm.level = 1;
            gm.gameOver = false;
            gm.gameWon = false;
            gm.startLevel();
        }

        BeginDrawing();
            ClearBackground(BLACK);
            gm.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}