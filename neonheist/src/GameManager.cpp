#include "GameManager.h"
#include <iostream>
#include <cmath>
#include <utility>
#include <algorithm>
using namespace std;

const int SCREEN_W = 2880;
const int SCREEN_H = 1920;

// ------------------------------------------------
// CONSTRUCTOR
// called once when game first starts
// sets all starting values and loads sounds
// ------------------------------------------------
GameManager::GameManager() : robber(18 * 144, 1 * 144) {
    level = 1;
    gameOver = false;
    gameWon = false;
    alarmActive = false;

    // load all sound files from assets folder
    InitAudioDevice();
    bgMusic       = LoadMusicStream("assets/background.mp3");
    alarmSound    = LoadMusicStream("assets/alarm.mp3");
    collectSound  = LoadSound("assets/collect.mp3");
    gameOverSound = LoadSound("assets/gameover.mp3");


    //diff in vols 
    // set volumes - 0.0 = silent, 1.0 = full volume
    SetMusicVolume(bgMusic, 0.3f);      // background music quieter
    SetMusicVolume(alarmSound, 3.0f);   // alarm at full volume
    SetSoundVolume(collectSound, 0.8f); // collect sound loud
    SetSoundVolume(gameOverSound, 1.0f);// game over at full volume

PlayMusicStream(bgMusic);

    // camera setup - robber stays centered in 70% of screen
    // 30% is taken by sidebar on the right
    camera.target   = robber.worldPos;
    camera.offset   = {SCREEN_W * 0.70f / 2.0f, SCREEN_H / 2.0f};
    camera.rotation = 0.0f;
    camera.zoom     = 1.0f;
}

// ------------------------------------------------
// STARTLEVEL
// called at beginning of each level
// clears old objects, generates new map, spawns everything
// ------------------------------------------------
void GameManager::startLevel() {
    // clear police from previous level
    policeUnits.clear();

    // delete treasure pointers and clear vector
    for (auto t : treasures) delete t;
    treasures.clear();

    // bystanders also clear and respawn with new positions
    bystanders.clear();

    // reset alarm
    alarmActive = false;
    AlarmSystem::getInstance().reset();

    // generate fresh random map for this level
    map.regenerate();

    // find safe street tile for robber starting from top right
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

    // spawn all game objects
    spawnTreasures();
    spawnPolice();
    spawnBystanders(); // always 20 bystanders, positions change each level
}

// ------------------------------------------------
// SPAWNTREASSURES
// finds DOOR tiles near BANK and PRESIDENT buildings
// picks 5 random ones to place diamonds on
// ------------------------------------------------
void GameManager::spawnTreasures() {
    vector<pair<int,int>> validSpots;

    for (int y = 1; y < CityMap::MAP_SIZE; y++) {
        for (int x = 0; x < CityMap::MAP_SIZE; x++) {
            if (map.grid[y][x] == DOOR) {
                // check tile above door to find building type
                int buildingType = map.grid[y-1][x];
                if (buildingType == BANK || buildingType == PRESIDENT) {
                    validSpots.push_back({x, y});
                }
            }
        }
    }

    // pick 5 random spots from valid ones
    int count = min((int)validSpots.size(), 5);
    for (int i = 0; i < count; i++) {
        int idx = GetRandomValue(0, (int)validSpots.size() - 1);
        treasures.push_back(new Treasure(validSpots[idx].first,
                                         validSpots[idx].second));
        validSpots.erase(validSpots.begin() + idx);
    }
}

// ------------------------------------------------
// SPAWNPOLICE
// spawns police on random street tiles
// count = level number, max 7
// keeps them far from robber spawn
// ------------------------------------------------
void GameManager::spawnPolice() {
    int policeCount = min(level, 7);
    policeUnits.clear();

    // find robber tile position to keep police away
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
            // must be at least 8 tiles away from robber
            (abs(px - robberTileX) < 8 && abs(py - robberTileY) < 8)
        ));

        policeUnits.push_back(Police(px * map.tileSize, py * map.tileSize));
    }
}

// ------------------------------------------------
// SPAWNBYSTANDERS
// always spawns exactly 20 bystanders
// positions change every level but count stays 20
// they walk randomly normally, run away when alarm rings
// ------------------------------------------------
void GameManager::spawnBystanders() {
    bystanders.clear();

    // always exactly 20 bystanders every level
    const int BYSTANDER_COUNT = 20;

    int robberTileX = (int)(robber.worldPos.x / map.tileSize);
    int robberTileY = (int)(robber.worldPos.y / map.tileSize);

    for (int i = 0; i < BYSTANDER_COUNT; i++) {
        int bx, by, attempts = 0;
        do {
            bx = GetRandomValue(1, CityMap::MAP_SIZE - 2);
            by = GetRandomValue(1, CityMap::MAP_SIZE - 2);
            attempts++;
        } while (attempts < 200 && (
            map.grid[by][bx] != STREET ||
            // keep bystanders at least 3 tiles from robber spawn
            (abs(bx - robberTileX) < 3 && abs(by - robberTileY) < 3)
        ));

        bystanders.push_back(Bystander(bx * map.tileSize, by * map.tileSize));
    }
}

// ------------------------------------------------
// UPDATE
// called every single frame
// updates all game objects and checks collisions
// ------------------------------------------------
void GameManager::update() {
    // if game ended, only check for restart key
    if (gameOver || gameWon) {
        if (IsKeyPressed(KEY_R)) {
            level = 1;
            gameOver = false;
            gameWon = false;
            startLevel();
        }
        return;
    }

    // keep background music playing every frame
    UpdateMusicStream(bgMusic);

    // save previous alarm state to detect changes
    bool wasActive = alarmActive;

    // update alarm countdown timer
    AlarmSystem::getInstance().update(GetFrameTime());
    // check if alarm is still active after timer update
    alarmActive = AlarmSystem::getInstance().isActive();

    // play alarm sound when alarm turns on
if (alarmActive && !wasActive) {
    SetMusicVolume(bgMusic, 0.05f);   // drop bg music very low
    PlayMusicStream(alarmSound);
}
// stop alarm sound when alarm turns off
else if (!alarmActive && wasActive) {
    SetMusicVolume(bgMusic, 0.3f);    // restore bg music
    StopMusicStream(alarmSound);
}
    // stop alarm sound when alarm turns off
    else if (!alarmActive && wasActive) {
        StopMusicStream(alarmSound);
    }

    // keep alarm music streaming while active
    if (alarmActive) {
        UpdateMusicStream(alarmSound);
    }

    // move robber based on arrow keys
    robber.update(map);

    // camera follows robber every frame
    camera.target = robber.worldPos;

    // move all police - chase or patrol based on alarm
    for (auto& p : policeUnits) {
        p.update(robber.worldPos, alarmActive, map);
    }

    // move all bystanders - walk normally or run away based on alarm
    for (auto& b : bystanders) {
        b.update(robber.worldPos, alarmActive, map);
    }

    // check all collisions last
    checkCollisions();
}

// ------------------------------------------------
// CHECKCOLLISIONS
// uses Raylib built in CheckCollisionCircles
// checks robber vs police and robber vs treasure
// ------------------------------------------------
void GameManager::checkCollisions() {
    // check if any police caught the robber
    // using Raylib's built in circle collision function
    for (auto& p : policeUnits) {
        if (CheckCollisionCircles(robber.worldPos, 25, p.worldPos, 25)) {
            gameOver = true;
            PlaySound(gameOverSound);
            return; // stop checking once game is over
        }
    }

    // check if robber stepped on a treasure tile
    for (auto t : treasures) {
        if (!t->collected && t->isNear(robber.worldPos, map.tileSize)) {
            t->collected = true;
            // add diamond to inventory
            inventory.addItem(new Item("Diamond", 100));
            // play collect sound first
            PlaySound(collectSound);
            // then trigger alarm
            AlarmSystem::getInstance().trigger();
            alarmActive = true;
            cout << "Diamond collected! Alarm triggered!" << endl;
        }
    }

    // check if all treasures collected to advance level
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
                gameWon = true; // all 10 levels done
            } else {
                startLevel(); // go to next level
            }
        }
    }
}

// ------------------------------------------------
// DRAW
// called every frame
// everything inside BeginMode2D scrolls with camera
// everything outside BeginMode2D is fixed on screen
// ------------------------------------------------
void GameManager::draw() {
    // background color matches road color
    ClearBackground({45, 45, 48, 255});

    // --- WORLD SPACE (scrolls with camera) ---
    BeginMode2D(camera);
        // draw map tiles first (bottom layer)
        map.Draw(alarmActive);

        // draw treasures on top of map
        for (auto t : treasures) t->Draw(map.tileSize);

        // draw bystanders
        for (auto& b : bystanders) b.Draw();

        // draw robber
        robber.Draw();

        // draw police on top of everything
        for (auto& p : policeUnits) p.Draw();

    EndMode2D();

    // --- SCREEN SPACE (fixed, does not scroll) ---

    // pulsing red overlay when alarm is active
    if (alarmActive) {
        float alpha = (sin(GetTime() * 8) + 1.0f) / 2.0f * 0.15f;
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(RED, alpha));
        DrawText("!! ALARM !!", SCREEN_W * 0.35f - 200, 30, 80, RED);
    }

    // draw sidebar UI
    drawUI();

    // game over overlay
    if (gameOver) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 200});
        DrawText("WASTED",
                 SCREEN_W/2 - 180, SCREEN_H/2 - 100, 100, RED);
        DrawText("Press R to Restart",
                 SCREEN_W/2 - 200, SCREEN_H/2 + 40, 50, RAYWHITE);
    }

    // win overlay
    if (gameWon) {
        DrawRectangle(0, 0, SCREEN_W, SCREEN_H, {0, 0, 0, 200});
        DrawText("MASTER THIEF",
                 SCREEN_W/2 - 300, SCREEN_H/2 - 100, 100, GOLD);
        DrawText("Press R to Play Again",
                 SCREEN_W/2 - 220, SCREEN_H/2 + 40, 50, WHITE);
    }
}

// ------------------------------------------------
// DRAWUI
// draws the right sidebar with all game info
// sidebar = 30% of screen width
// ------------------------------------------------
void GameManager::drawUI() {
    int sidebarX = (int)(SCREEN_W * 0.70f);
    int sidebarW = (int)(SCREEN_W * 0.30f);

    // sidebar background with neon blue border
    DrawRectangle(sidebarX, 0, sidebarW, SCREEN_H, {15, 15, 20, 255});
    DrawRectangleLinesEx({(float)sidebarX, 0, (float)sidebarW,
                         (float)SCREEN_H}, 8, SKYBLUE);

    // game title
    DrawText("NEON HEIST", sidebarX + 70, 60, 75, YELLOW);
    DrawLine(sidebarX + 60, 160, sidebarX + sidebarW - 60, 160, DARKGRAY);

    // current level
    DrawText(TextFormat("PHASE: %02d / 10", level),
             sidebarX + 70, 200, 55, RAYWHITE);
    DrawLine(sidebarX + 60, 280, sidebarX + sidebarW - 60, 280, DARKGRAY);

    // alarm status - red box if active, green if not
    if (alarmActive) {
        DrawRectangle(sidebarX + 50, 310, sidebarW - 100, 90, RED);
        DrawText("!! ALARM ACTIVE !!", sidebarX + 70, 335, 40, WHITE);
    } else {
        DrawRectangle(sidebarX + 50, 310, sidebarW - 100, 90, DARKGREEN);
        DrawText("STEALTH MODE", sidebarX + 70, 335, 40, WHITE);
    }

    // inventory info
    DrawLine(sidebarX + 60, 430, sidebarX + sidebarW - 60, 430, DARKGRAY);
    DrawText("LOOT SECURED", sidebarX + 70, 460, 38, GRAY);
    DrawText(TextFormat("%d DIAMONDS", inventory.getItemCount()),
             sidebarX + 70, 515, 65, SKYBLUE);
    DrawText(TextFormat("TOTAL: $%d", inventory.getTotalValue()),
             sidebarX + 70, 600, 50, GOLD);

    // police count - increases each level
    DrawLine(sidebarX + 60, 680, sidebarX + sidebarW - 60, 680, DARKGRAY);
    DrawText(TextFormat("POLICE ON DUTY: %d", (int)policeUnits.size()),
             sidebarX + 70, 710, 42, RED);
    DrawText("(increases each level)",
             sidebarX + 70, 765, 32, DARKGRAY);

    // how to play section
    DrawLine(sidebarX + 60, 830, sidebarX + sidebarW - 60, 830, DARKGRAY);
    DrawText("HOW TO PLAY", sidebarX + 70, 860, 45, GRAY);

    // instruction boxes with rounded corners
    DrawRectangleRounded({(float)sidebarX + 50, 925,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Arrow Keys  -  Move",
             sidebarX + 70, 943, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1015,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Walk over diamond  -  Steal",
             sidebarX + 70, 1033, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1105,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Avoid the police!",
             sidebarX + 70, 1123, 38, RED);

    DrawRectangleRounded({(float)sidebarX + 50, 1195,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Steal from BANK &",
             sidebarX + 70, 1213, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1285,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("PRESIDENT buildings only",
             sidebarX + 70, 1303, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1375,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Collect all 5 diamonds",
             sidebarX + 70, 1393, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1465,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("to advance the level",
             sidebarX + 70, 1483, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1555,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("Beware of civilians!",
             sidebarX + 70, 1573, 38, LIGHTGRAY);

    DrawRectangleRounded({(float)sidebarX + 50, 1645,
                         (float)sidebarW - 100, 70}, 0.3f, 10, {30, 30, 40, 255});
    DrawText("They panic on alarm",
             sidebarX + 70, 1663, 38, LIGHTGRAY);

    // restart reminder at bottom
    DrawLine(sidebarX + 60, 1740, sidebarX + sidebarW - 60, 1740, DARKGRAY);
    DrawText("R  -  Restart Game",
             sidebarX + 70, 1770, 40, DARKGRAY);
}

// ------------------------------------------------
// DESTRUCTOR
// cleans up all memory when game closes
// ------------------------------------------------
GameManager::~GameManager() {
    // delete all treasure pointers
    for (auto t : treasures) delete t;
    treasures.clear();

    // unload all audio
    UnloadMusicStream(bgMusic);
    UnloadMusicStream(alarmSound);
    UnloadSound(collectSound);
    UnloadSound(gameOverSound);
    CloseAudioDevice();
}