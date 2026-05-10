#include "GameManager.h"
#include <iostream>
#include <cmath>
#include <utility>
using namespace std;

// screen constants - Surface Pro 2880x1920
const int SCREEN_W = 2880;
const int SCREEN_H = 1920;
const int TILE_SIZE = 96;
const int MAP_PX = 1920;
const int SIDEBAR_X = 1920;
const int SIDEBAR_W = 960;

// constructor - sets starting values
GameManager::GameManager() : robber(18 * TILE_SIZE, 1 * TILE_SIZE) {
    level = 1;
    gameOver = false;
    gameWon = false;
    alarmActive = false;

    // sound will also be added in the constructor for game manager
    InitAudioDevice();
    bgMusic = LoadMusicStream("assets/background.mp3");
    alarmSound = LoadMusicStream("assets/alarm.mp3");
    collectSound = LoadSound("assets/collect.mp3");
    gameOverSound = LoadSound("assets/gameover.mp3");

    // start bgm foran se
    PlayMusicStream(bgMusic);

    // camera setup - follows robber, robber stays centered
    camera.target = robber.worldPos;
    camera.offset = {SCREEN_W / 2.0f, SCREEN_H / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

// sets up each new level
void GameManager::startLevel() {
    policeUnits.clear();
    for (auto t : treasures) delete t;
    treasures.clear();

    alarmActive = false;
    AlarmSystem::getInstance().reset();

    map.regenerate();

    // find safe street tile starting from top right going left
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

// spawns 5 random treasures near BANK and PRESIDENT buildings only
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
        int idx = GetRandomValue(0, validSpots.size() - 1);
        treasures.push_back(new Treasure(validSpots[idx].first, validSpots[idx].second));
        validSpots.erase(validSpots.begin() + idx);
    }
}

// spawns police on random street tiles far from robber
void GameManager::spawnPolice() {
    int policeCount = min(level, 7);
    policeUnits.clear();

    int robberTileX = (int)(robber.worldPos.x / map.tileSize);
    int robberTileY = (int)(robber.worldPos.y / map.tileSize);

    for (int i = 0; i < policeCount; i++) {
        int px, py;
        int attempts = 0;
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

// called every frame - updates all game objects
void GameManager::update() {
    if (gameOver || gameWon) return;

    // keep background music going
    UpdateMusicStream(bgMusic);

    // declare wasActive FIRST before anything else
    bool wasActive = alarmActive;

    // now update the alarm timer
    AlarmSystem::getInstance().update(GetFrameTime());
    alarmActive = AlarmSystem::getInstance().isActive();

    // start or stop alarm sound based on state change
    if (alarmActive && !wasActive) {
        PlayMusicStream(alarmSound);
    } else if (!alarmActive && wasActive) {
        StopMusicStream(alarmSound);
    }

    // keep alarm music going if active
    if (alarmActive) {
        UpdateMusicStream(alarmSound);
    }

    robber.update(map);

    for (auto& p : policeUnits) {
        p.update(robber.worldPos, alarmActive, map);
    }

    // camera smoothly follows robber
    camera.target = robber.worldPos;

    for (auto& p : policeUnits) {
        p.update(robber.worldPos, alarmActive, map);
    }

    checkCollisions();
}

// checks all collisions every frame
void GameManager::checkCollisions() {
    // police catches robber = game over
    for (auto& p : policeUnits) {
        float dx = p.worldPos.x - robber.worldPos.x;
        float dy = p.worldPos.y - robber.worldPos.y;
        float distance = sqrt(dx*dx + dy*dy);
        if (distance < 40.0f) {
            gameOver = true;
            PlaySound(gameOverSound);
            return;
        }
    }

    // robber touches treasure = collect it
    for (auto t : treasures) {
        if (!t->collected && t->isNear(robber.worldPos, map.tileSize)) {
            t->collected = true;
            inventory.addItem(new Item("Diamond", 100));
            PlaySound(collectSound); // phle collect sound ayega phir alarm bajega
            AlarmSystem::getInstance().trigger();
            alarmActive = true;
            cout << "Diamond collected! Alarm triggered!" << endl;
        }
    }

    // only check level completion if treasures actually exist
    if (!treasures.empty()) {
        bool allCollected = true;
        for (auto t : treasures) {
            if (!t->collected) {
                allCollected = false;
                break;
            }
        }
        if (allCollected) {
            level++;
            if (level > 10) {
                gameWon = true;
            } else {
                startLevel();
            }
        }
    }
}

// draws everything on screen
void GameManager::draw() {

    // everything inside BeginMode2D scrolls with camera
    BeginMode2D(camera);

        map.Draw(alarmActive);

        for (auto t : treasures) {
            t->Draw(map.tileSize);
        }

        robber.Draw();

        for (auto& p : policeUnits) {
            p.Draw();
        }

    EndMode2D();

    // everything below here is fixed on screen - does not scroll

    // red flash when alarm active
    if (alarmActive) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {255, 0, 0, 30});
        DrawText("!! ALARM !!", SCREEN_W/2 - 200, 20, 80, RED);
    }

    // sidebar UI - always fixed on right side
    drawUI();

    // game over screen
    if (gameOver) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 180});
        DrawText("GAME OVER", SCREEN_W/2 - 200, SCREEN_H/2 - 60, 100, RED);
        DrawText("Press R to restart", SCREEN_W/2 - 180, SCREEN_H/2 + 60, 50, WHITE);
    }

    // win screen
    if (gameWon) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 180});
        DrawText("YOU WIN!", SCREEN_W/2 - 150, SCREEN_H/2 - 60, 100, GREEN);
        DrawText("All heists complete!", SCREEN_W/2 - 220, SCREEN_H/2 + 60, 50, WHITE);
    }
}

// draws the sidebar UI
void GameManager::drawUI() {
    DrawRectangle(SIDEBAR_X, 0, SIDEBAR_W, SCREEN_H, BLACK);
    DrawText("NEON HEIST", SIDEBAR_X + 80, 80, 60, YELLOW);
    DrawText(TextFormat("Level: %d / 10", level), SIDEBAR_X + 80, 220, 45, WHITE);

    if (alarmActive) {
        DrawText("ALARM ACTIVE!", SIDEBAR_X + 80, 320, 45, RED);
    } else {
        DrawText("All clear", SIDEBAR_X + 80, 320, 45, GREEN);
    }

    DrawText(TextFormat("Diamonds: %d", inventory.getItemCount()), SIDEBAR_X + 80, 420, 45, SKYBLUE);
    DrawText(TextFormat("Score: %d", inventory.getTotalValue()), SIDEBAR_X + 80, 520, 45, WHITE);
    DrawText("Arrow keys to move", SIDEBAR_X + 80, SCREEN_H - 200, 35, GRAY);
    DrawText("Steal from Bank &", SIDEBAR_X + 80, SCREEN_H - 150, 35, GRAY);
    DrawText("President buildings", SIDEBAR_X + 80, SCREEN_H - 100, 35, GRAY);
}

// destructor - cleans up memory
GameManager::~GameManager() {
    for (auto t : treasures) delete t;
    treasures.clear();

    // sound bhi destroy hoga
    UnloadMusicStream(bgMusic);
    UnloadMusicStream(alarmSound);
    UnloadSound(collectSound);
    UnloadSound(gameOverSound);
    CloseAudioDevice();
}