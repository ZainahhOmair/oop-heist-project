#include "raylib.h"
#include "GameManager.h"

int main() {
    InitWindow(800, 600, "Neon Heist");

    int monitor = GetCurrentMonitor();
    int monitorW = GetMonitorWidth(monitor);
    int monitorH = GetMonitorHeight(monitor);

    SetWindowSize(monitorW, monitorH);
    ToggleFullscreen();
    SetTargetFPS(60);

    GameManager gm;
    gm.startLevel();

    while (!WindowShouldClose()) {
        gm.update();

        BeginDrawing();
            ClearBackground(BLACK);
            gm.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}