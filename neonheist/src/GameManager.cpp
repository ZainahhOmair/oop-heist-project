#include "GameManager.h"
#include <iostream>
#include <cmath>
#include <utility>
#include <algorithm>
using namespace std;

int SCREEN_W = GetScreenWidth();
int SCREEN_H = GetScreenHeight();

// constructor
GameManager::GameManager() : robber(18 * 144, 1 * 144) {
    level = 1;
    gameOver = false;
    gameWon = false;
    alarmActive = false;
    levelComplete = false;

    InitAudioDevice();
    bgMusic       = LoadMusicStream("assets/background.mp3");
    alarmSound    = LoadMusicStream("assets/alarm.mp3");
    collectSound  = LoadSound("assets/collect.mp3");
    gameOverSound = LoadSound("assets/gameover.mp3");

    SetMusicVolume(bgMusic, 0.4f);
    SetMusicVolume(alarmSound, 3.0f);
    SetSoundVolume(collectSound, 1.0f);
    SetSoundVolume(gameOverSound, 1.0f);

    PlayMusicStream(bgMusic);

    // actual screen size after wndw
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    camera.target   = robber.worldPos;
    camera.offset   = {sw * 0.70f / 2.0f, sh / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom     = 1.0f;
}

//level starttt
void GameManager::startLevel() {
    policeUnits.clear();
    for (auto t : treasures) delete t;
    treasures.clear();
    bystanders.clear();

    alarmActive = false;
    levelComplete = false;
    AlarmSystem::getInstance().reset();

    map.regenerate();

    //safe spwning, building k ander nhi
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
    spawnBystanders();
}

// spawns 5 treasures near BANK and PRESIDENT only
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

// spawns police faraway from robber
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
        } while (attempts < 50 && (
            map.grid[py][px] != STREET ||
            (abs(px - robberTileX) < 5 && abs(py - robberTileY) < 5)
        ));

        policeUnits.push_back(Police(px * map.tileSize, py * map.tileSize));
    }
}

// 20 ppl
void GameManager::spawnBystanders() {
    bystanders.clear();
    const int BYSTANDER_COUNT = 20;

    int robberTileX = (int)(robber.worldPos.x / map.tileSize);
    int robberTileY = (int)(robber.worldPos.y / map.tileSize);

    for (int i = 0; i < BYSTANDER_COUNT; i++) {
        int bx, by, attempts = 0;
        do {
            bx = GetRandomValue(1, CityMap::MAP_SIZE - 2);
            by = GetRandomValue(1, CityMap::MAP_SIZE - 2);
            attempts++;
        } while (attempts < 50 && (
            map.grid[by][bx] != STREET ||
            (abs(bx - robberTileX) < 2 && abs(by - robberTileY) < 2)
        ));

        bystanders.push_back(Bystander(bx * map.tileSize, by * map.tileSize));
    }
}

// called every frame
void GameManager::update() {
    if (gameOver || gameWon) {
        if (IsKeyPressed(KEY_R)) {
            level = 1;
            gameOver = false;
            gameWon = false;
            levelComplete = false;
            startLevel();
        }
        return;
    }

    if (levelComplete) {
        levelComplete = false;
        level++;
        if (level > 10) {
            gameWon = true;
        } else {
            startLevel();
        }
        return;
    }

    if (IsMusicStreamPlaying(bgMusic)) {
        UpdateMusicStream(bgMusic);
    }

    bool wasActive = alarmActive;
    AlarmSystem::getInstance().update(GetFrameTime());
    alarmActive = AlarmSystem::getInstance().isActive();

    if (alarmActive && !wasActive) {
        SetMusicVolume(bgMusic, 0.05f);
        PlayMusicStream(alarmSound);
    } else if (!alarmActive && wasActive) {
        SetMusicVolume(bgMusic, 0.3f);
        StopMusicStream(alarmSound);
    }

    if (alarmActive && IsMusicStreamPlaying(alarmSound)) {
        UpdateMusicStream(alarmSound);
    }

    robber.update(map);
    camera.target = robber.worldPos;

    for (auto& p : policeUnits) {
        p.update(robber.worldPos, alarmActive, map);
    }

    for (auto& b : bystanders) {
        b.update(robber.worldPos, alarmActive, map);
    }

    checkCollisions();
}

// checks all collisions
void GameManager::checkCollisions() {
    for (auto& p : policeUnits) {
        if (CheckCollisionCircles(robber.worldPos, 25, p.worldPos, 25)) {
            gameOver = true;
            PlaySound(gameOverSound);
            return;
        }
    }

    for (auto& b : bystanders) {
        if (CheckCollisionCircles(robber.worldPos, 20, b.worldPos, 20)) {
            float dx = robber.worldPos.x - b.worldPos.x;
            float dy = robber.worldPos.y - b.worldPos.y;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist > 0) {
                robber.worldPos.x = b.worldPos.x + (dx / dist) * 42;
                robber.worldPos.y = b.worldPos.y + (dy / dist) * 42;
            }
        }
    }

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

    if (!treasures.empty()) {
        bool allCollected = true;
        for (auto t : treasures) {
            if (!t->collected) { allCollected = false; break; }
        }
        if (allCollected) {
            levelComplete = true;
        }
    }
}

// draws everything
void GameManager::draw() {
    ClearBackground({45, 45, 48, 255});

    BeginMode2D(camera);
        map.Draw(alarmActive);
        for (auto t : treasures) t->Draw(map.tileSize);
        for (auto& b : bystanders) b.Draw();
        robber.Draw();
        for (auto& p : policeUnits) p.Draw();
    EndMode2D();

    if (alarmActive) {
        float alpha = (sin(GetTime() * 8) + 1.0f) / 2.0f * 0.15f;
        int mapAreaW = (int)(GetScreenWidth() * 0.70f);
        DrawRectangle(0, 0, mapAreaW, GetScreenHeight(), Fade(RED, alpha));
        DrawText("!! RUN FOR YOUR LIFE !!", mapAreaW/2 - 200, 30, 80, RED);
    }

    drawUI();

    if (gameOver) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 200});
        DrawText("ARRESTED", GetScreenWidth()/2 - 180, GetScreenHeight()/2 - 100, 100, RED);
        DrawText("Press R to Restart", GetScreenWidth()/2 - 200, GetScreenHeight()/2 + 40, 50, RAYWHITE);
    }

    if (gameWon) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), {0, 0, 0, 200});
        DrawText("MASTER THIEF", GetScreenWidth()/2 - 300, GetScreenHeight()/2 - 100, 100, GOLD);
        DrawText("Press R to Play Again", GetScreenWidth()/2 - 220, GetScreenHeight()/2 + 40, 50, WHITE);
    }
}

// draws sidebar UI
void GameManager::drawUI() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    int sidebarX = (int)(sw * 0.70f);
    int sidebarW = (int)(sw * 0.30f);

    DrawRectangle(sidebarX, 0, sidebarW, sh, {15, 15, 20, 255});
    DrawRectangleLinesEx({(float)sidebarX, 0, (float)sidebarW,
                         (float)sh}, 8, SKYBLUE);

    DrawText("NEON HEIST", sidebarX + 70, 120, 75, YELLOW);
    DrawLine(sidebarX + 60, 220, sidebarX + sidebarW - 60, 220, DARKGRAY);
    DrawText(TextFormat("PHASE: %02d / 10", level), sidebarX + 70, 260, 55, RAYWHITE);
    DrawLine(sidebarX + 60, 340, sidebarX + sidebarW - 60, 340, DARKGRAY);

    if (alarmActive) {
        DrawRectangle(sidebarX + 50, 370, sidebarW - 100, 90, RED);
        DrawText("!! ALARM ACTIVE !!", sidebarX + 70, 395, 40, WHITE);
    } else {
        DrawRectangle(sidebarX + 50, 370, sidebarW - 100, 90, DARKGREEN);
        DrawText("STEALTH MODE", sidebarX + 70, 395, 40, WHITE);
    }

    DrawLine(sidebarX + 60, 490, sidebarX + sidebarW - 60, 490, DARKGRAY);
    DrawText("LOOT SECURED", sidebarX + 70, 520, 38, GRAY);
    DrawText(TextFormat("%d DIAMONDS", inventory.getItemCount()),
             sidebarX + 70, 575, 65, SKYBLUE);
    DrawText(TextFormat("TOTAL: $%d", inventory.getTotalValue()),
             sidebarX + 70, 660, 50, GOLD);

    DrawLine(sidebarX + 60, 740, sidebarX + sidebarW - 60, 740, DARKGRAY);
    DrawText(TextFormat("POLICE ON DUTY: %d", (int)policeUnits.size()),
             sidebarX + 70, 770, 42, RED);
    DrawText("increases each level",
             sidebarX + 70, 825, 32, DARKGRAY);

    DrawLine(sidebarX + 60, 890, sidebarX + sidebarW - 60, 890, DARKGRAY);
    DrawText("HOW TO PLAY", sidebarX + 70, 920, 45, GRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 985,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Arrow Keys  -  Move", sidebarX + 70, 1003, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1075,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Walk over diamond  -  Steal", sidebarX + 70, 1093, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1165,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Avoid the police!", sidebarX + 70, 1183, 38, RED);

    DrawRectangleRounded({(float)sidebarX + 50, 1255,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Steal from BANK &", sidebarX + 70, 1273, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1345,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("PRESIDENT buildings only", sidebarX + 70, 1363, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1435,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Collect all diamonds", sidebarX + 70, 1453, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1525,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("to advance the level", sidebarX + 70, 1543, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1615,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Beware of civilians!", sidebarX + 70, 1633, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1705,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("They panic on alarm", sidebarX + 70, 1723, 38, LIGHTGRAY);

    DrawLine(sidebarX + 60, 1800, sidebarX + sidebarW - 60, 1800, DARKGRAY);
    DrawText("R  -  Restart Game", sidebarX + 70, 1830, 40, DARKGRAY);
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