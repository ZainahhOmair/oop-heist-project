#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include <vector>

enum TileType {
    STREET    = 0,
    WALL      = 1,
    TREES     = 2,
    NORMAL    = 4,
    DOOR      = 5,
    MALL      = 6,
    BANK      = 7,
    PRESIDENT = 8
};

class CityMap {
public:
    static const int MAP_SIZE = 30;
    int grid[MAP_SIZE][MAP_SIZE];
    int tileSize;

    CityMap() {
        tileSize = 2880 / MAP_SIZE;

        for (int i = 0; i < MAP_SIZE; i++) {
            for (int j = 0; j < MAP_SIZE; j++) {
                if (i == 0 || i == MAP_SIZE - 1 || j == 0 || j == MAP_SIZE - 1)
                    grid[i][j] = WALL;
                else
                    grid[i][j] = STREET;
            }
        }
        spawnBuilding(PRESIDENT, 1);
        spawnBuilding(BANK, 2);
        spawnBuilding(MALL, 2);
        spawnBuilding(NORMAL, 6);
    }

    void regenerate() {
        for (int i = 0; i < MAP_SIZE; i++) {
            for (int j = 0; j < MAP_SIZE; j++) {
                if (i == 0 || i == MAP_SIZE - 1 || j == 0 || j == MAP_SIZE - 1)
                    grid[i][j] = WALL;
                else
                    grid[i][j] = STREET;
            }
        }
        spawnBuilding(PRESIDENT, 1);
        spawnBuilding(BANK, 2);
        spawnBuilding(MALL, 2);
        spawnBuilding(NORMAL, 6);
    }

    bool isWalkable(int x, int y) const {
        if (x < 0 || x >= MAP_SIZE || y < 0 || y >= MAP_SIZE) return false;
        return grid[y][x] == STREET || grid[y][x] == DOOR;
    }

    void spawnBuilding(int type, int count) {
        for (int k = 0; k < count; k++) {
            bool placed = false;
            int attempts = 0;
            while (!placed && attempts < 100) {
                attempts++;
                int r = GetRandomValue(2, MAP_SIZE - 4);
                int c = GetRandomValue(2, MAP_SIZE - 4);
                if (grid[r][c] == STREET &&
                    grid[r-1][c] == STREET &&
                    grid[r][c+1] == STREET &&
                    grid[r-1][c+1] == STREET &&
                    grid[r+1][c] == STREET) {
                    grid[r][c]     = type;
                    grid[r-1][c]   = type;
                    grid[r][c+1]   = type;
                    grid[r-1][c+1] = type;
                    grid[r+1][c]   = DOOR;
                    placed = true;
                }
            }
        }
    }

    void Draw(bool alarmActive) {
        for (int y = 0; y < MAP_SIZE; y++) {
            for (int x = 0; x < MAP_SIZE; x++) {
                Vector2 pos = {(float)x * tileSize, (float)y * tileSize};

                // base tile
                if (grid[y][x] == STREET || grid[y][x] == DOOR) {
                    DrawRectangleV(pos, {(float)tileSize, (float)tileSize}, DARKGRAY);
                    if (x % 2 == 0)
                        DrawRectangle(pos.x + tileSize/2 - 2, pos.y + 4, 4, tileSize - 8, RAYWHITE);
                } else {
                    DrawRectangleV(pos, {(float)tileSize, (float)tileSize}, DARKGREEN);
                }

                // objects on tiles
                if (grid[y][x] == WALL)
                    DrawRectangleRec({pos.x, pos.y, (float)tileSize, (float)tileSize}, BLACK);
                else if (grid[y][x] == BANK)
                    DrawBuilding(pos, GOLD);
                else if (grid[y][x] == MALL)
                    DrawBuilding(pos, ORANGE);
                else if (grid[y][x] == PRESIDENT)
                    DrawBuilding(pos, PURPLE);
                else if (grid[y][x] == NORMAL)
                    DrawBuilding(pos, DARKBLUE);
                else if (grid[y][x] == DOOR) {
                    // draw door attached to bottom of building above it
                    Color doorColor = alarmActive ? RED : MAROON;
                    // full width door at top of tile to connect with building
                    DrawRectangleV(pos, {(float)tileSize, (float)tileSize/2}, doorColor);
                    // door frame
                    DrawRectangle(pos.x + tileSize/3, pos.y, tileSize/3, tileSize/2, DARKBROWN);
                    // door handle
                    DrawCircle(pos.x + tileSize/3 + tileSize/5, pos.y + tileSize/4, 4, GOLD);
                }
            }
        }
    }

    void DrawBuilding(Vector2 pos, Color col) {
        DrawRectangleV(pos, {(float)tileSize - 2, (float)tileSize - 2}, col);
        DrawRectangle(pos.x + 4,          pos.y + 4, tileSize/4, tileSize/4, SKYBLUE);
        DrawRectangle(pos.x + tileSize/2, pos.y + 4, tileSize/4, tileSize/4, SKYBLUE);
    }
};

#endif