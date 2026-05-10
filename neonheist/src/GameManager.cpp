
#include "GameManager.h"
#include <iostream>
#include <cmath>
#include <utility>
#include <algorithm>
using namespace std;

const int SCREEN_W = 2880;
const int SCREEN_H = 1920;

// constructor
GameManager::GameManager() : robber(18 * 144, 1 * 144) {
    level = 1;
    gameOver = false;
    gameWon = false;
    alarmActive = false;

    InitAudioDevice();
    bgMusic     = LoadMusicStream("assets/background.mp3");
    alarmSound  = LoadMusicStream("assets/alarm.mp3");
    collectSound  = LoadSound("assets/collect.mp3");
    gameOverSound = LoadSound("assets/gameover.mp3");
    PlayMusicStream(bgMusic);

    // camera centers robber on full screen
    camera.target   = robber.worldPos;
    camera.offset   = {SCREEN_W / 2.0f, SCREEN_H / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom     = 1.0f;
}

// sets up each new level
void GameManager::startLevel() {
    policeUnits.clear();
    for (auto t : treasures) delete t;
    treasures.clear();

    alarmActive = false;
    AlarmSystem::getInstance().reset();  // resets alarm timer

    map.regenerate();

    // spawn robber safely top right
    bool spawned = false;
    for (int x = CityMap::MAP_SIZE - 2; x > 0 && !spawned; x--) {
        for (int y = 1; y < 4 && !spawned; y++) {
            if (map.grid[y][x] == STREET) {
                robber.worldPos = {(float)(x * map.tileSize),
                                  (float)(y * map.tileSize)};
                spawned = true;
            }
        }
    }

    spawnTreasures();
    spawnPolice();
}

// spawns 5 random treasures near BANK and PRESIDENT only
void GameManager::spawnTreasures() {
    vector<pair<int,int>> validSpots;
    for (int y = 1; y < CityMap::MAP_SIZE; y++) {
        for (int x = 0; x < CityMap::MAP_SIZE; x++) {
            if (map.grid[y][x] == DOOR) {
                int buildingType = map.grid[y-1][x];
                if (buildingType == BANK || buildingType == PRESIDENT) {
                    validSpots.push_back({x, y});
                }
            }
        }
    }

    int count = min((int)validSpots.size(), 5);
    for (int i = 0; i < count; i++) {
        int idx = GetRandomValue(0, (int)validSpots.size() - 1);
        treasures.push_back(new Treasure(validSpots[idx].first,
                                         validSpots[idx].second));
        validSpots.erase(validSpots.begin() + idx);
    }
}

// spawns police far from robber
void GameManager::spawnPolice() {
    int policeCount = min(level, 7);
    policeUnits.clear();

    int robberTileX = (int)(robber.worldPos.x / map.tileSize);
    int robberTileY = (int)(robber.worldPos.y / map.tileSize);

    for (int i = 0; i < policeCount; i++) {
        int px, py, attempts = 0;
        do {
            px = GetRandomValue(1, CityMap::MAP_SIZE - 2);
            py = GetRandomValue(1, CityMap::MAP_SIZE - 2);
            attempts++;
        } while (attempts < 200 && (
            map.grid[py][px] != STREET ||
            (abs(px - robberTileX) < 8 && abs(py - robberTileY) < 8)
        ));

        policeUnits.push_back(Police(px * map.tileSize, py * map.tileSize));
    }
}

// called every frame
void GameManager::update() {
    if (gameOver || gameWon) {
        if (IsKeyPressed(KEY_R)) {
            level = 1;
            gameOver = false;
            gameWon = false;
            startLevel();
        }
        return;
    }

    UpdateMusicStream(bgMusic);

    // alarm timer - THIS IS THE KEY FIX
    bool wasActive = alarmActive;
    AlarmSystem::getInstance().update(GetFrameTime());  // counts down timer
    alarmActive = AlarmSystem::getInstance().isActive(); // false when timer runs out

    if (alarmActive && !wasActive)  PlayMusicStream(alarmSound);
    else if (!alarmActive && wasActive) StopMusicStream(alarmSound);
    if (alarmActive) UpdateMusicStream(alarmSound);

    robber.update(map);

    // camera smoothly follows robber
    camera.target = robber.worldPos;

    for (auto& p : policeUnits) {
        p.update(robber.worldPos, alarmActive, map);
    }

    checkCollisions();
}

// checks all collisions
void GameManager::checkCollisions() {
    // police catches robber
    for (auto& p : policeUnits) {
        float dx = p.worldPos.x - robber.worldPos.x;
        float dy = p.worldPos.y - robber.worldPos.y;
        float distance = sqrt(dx*dx + dy*dy);
        if (distance < 50.0f) {
            gameOver = true;
            PlaySound(gameOverSound);
            return;
        }
    }

    // robber touches treasure
    for (auto t : treasures) {
        if (!t->collected && t->isNear(robber.worldPos, map.tileSize)) {
            t->collected = true;
            inventory.addItem(new Item("Diamond", 100));
            PlaySound(collectSound);
            AlarmSystem::getInstance().trigger();
            alarmActive = true;
            cout << "Diamond collected! Alarm triggered!" << endl;
        }
    }

    // check level complete
    if (!treasures.empty()) {
        bool allCollected = true;
        for (auto t : treasures) {
            if (!t->collected) { allCollected = false; break; }
        }
        if (allCollected) {
            level++;
            if (level > 10) gameWon = true;
            else startLevel();
        }
    }
}

// draws everything
void GameManager::draw() {
    // infinite ground color matching streets
    ClearBackground({45, 45, 48, 255});

    BeginMode2D(camera);
        map.Draw(alarmActive);
        for (auto t : treasures) t->Draw(map.tileSize);
        robber.Draw();
        for (auto& p : policeUnits) p.Draw();
    EndMode2D();

    // alarm red flash - fixed on screen
    if (alarmActive) {
        float alpha = (sin(GetTime() * 8) + 1.0f) / 2.0f * 0.15f;
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(RED, alpha));
        DrawText("!! ALARM !!", SCREEN_W/2 - 200, 20, 80, RED);
    }

    // UI overlay
    drawUI();

    // game over screen
    if (gameOver) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 200});
        DrawText("WASTED", SCREEN_W/2 - 180, SCREEN_H/2 - 50, 100, RED);
        DrawText("Press R to Restart", SCREEN_W/2 - 160, SCREEN_H/2 + 60, 40, RAYWHITE);
    }

    // win screen
    if (gameWon) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 200});
        DrawText("MASTER THIEF", SCREEN_W/2 - 250, SCREEN_H/2 - 50, 100, GOLD);
        DrawText("Press R to Play Again", SCREEN_W/2 - 200, SCREEN_H/2 + 60, 40, WHITE);
    }
}

// draws UI overlay in top left corner
void GameManager::drawUI() {
    // small dark panel top left
    DrawRectangle(0, 0, 500, 220, {0, 0, 0, 170});
    DrawText("NEON HEIST", 20, 15, 40, YELLOW);
    DrawText(TextFormat("Level: %d / 10", level), 20, 65, 30, WHITE);

    if (alarmActive) {
        DrawText("ALARM ACTIVE!", 20, 105, 30, RED);
    } else {
        DrawText("All clear", 20, 105, 30, GREEN);
    }

    DrawText(TextFormat("Diamonds: %d  Score: $%d",
             inventory.getItemCount(),
             inventory.getTotalValue()), 20, 145, 25, SKYBLUE);
}

// destructor
GameManager::~GameManager() {
    for (auto t : treasures) delete t;
    treasures.clear();

    UnloadMusicStream(bgMusic);
    UnloadMusicStream(alarmSound);
    UnloadSound(collectSound);
    UnloadSound(gameOverSound);
    CloseAudioDevice();
}