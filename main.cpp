#include "raylib.h"
#include "GameOfLife.h"

int main() {
    const int screenWidth = 900;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Conway's Game of Life - IBA CS Edition");

    GameOfLife simulation;
    float updateTimer = 0;
    float updateInterval = 0.1f; 

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) simulation.isPaused = !simulation.isPaused;
        if (IsKeyPressed(KEY_R)) simulation.RandomizeGrid();
        if (IsKeyPressed(KEY_C)) simulation.ClearGrid();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mPos = GetMousePosition();
            int c = mPos.x / simulation.tileSize;
            int r = mPos.y / simulation.tileSize;
            if (r >= 0 && r < simulation.ROWS && c >= 0 && c < simulation.COLS) {
                simulation.grid[r][c] = !simulation.grid[r][c];
            }
        }

        updateTimer += GetFrameTime();
        if (updateTimer >= updateInterval) {
            simulation.UpdateSimulation();
            updateTimer = 0;
        }

        BeginDrawing();
            ClearBackground(BLACK);
            
            simulation.Draw();

            if (simulation.isPaused) DrawText("PAUSED - Click to toggle cells", 10, 570, 20, RAYWHITE);
            else DrawText("RUNNING", 10, 570, 20, GOLD);
            
            DrawText("R: Random | C: Clear | Space: Play/Pause", 500, 570, 20, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}\
