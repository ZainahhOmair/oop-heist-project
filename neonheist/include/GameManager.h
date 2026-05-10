#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "raylib.h"
#include "Map.h"
#include "Player.h"       // has Robber and Police classes inside
#include "Treasure.h"
#include "AlarmSystem.h"
#include "InventoryManager.h"
#include <vector>         // needed for storing multiple police and treasures

class GameManager {
public:

    // ------------------------------------------------
    // GAME STATE
    // these variables track what is happening in the game
    // ------------------------------------------------
    int level;            // current level (1 to 10)
    bool gameOver;        // true when police catches robber
    bool gameWon;         // true when all 10 levels are done
    bool alarmActive;     // true when robber steals treasure

    // ------------------------------------------------
    // GAME OBJECTS
    // these are the actual things that exist in the game world
    // ------------------------------------------------
    CityMap map;                        // the city grid
    Robber robber;                      // the player controlled character
    std::vector<Police> policeUnits;    // all police on current level
    std::vector<Treasure*> treasures;   // all treasures on current level


    //sound effects declared
    Music bgMusic;
    Music alarmSound;
    Sound collectSound;
    Sound gameOverSound;


    Camera2D camera;


    // ------------------------------------------------
    // INVENTORY
    // tracks all diamonds the robber has collected
    // ------------------------------------------------
    InventoryManager inventory;

    // ------------------------------------------------
    // CONSTRUCTOR
    // called once when game starts
    // sets everything to starting values
    // ------------------------------------------------
    GameManager();

    // ------------------------------------------------
    // STARTLEVEL
    // called at the beginning of each level
    // spawns police, spawns treasures, resets alarm
    // police count = level number, max 7
    // ------------------------------------------------
    void startLevel();

    // ------------------------------------------------
    // UPDATE
    // called every single frame
    // moves robber, moves police, checks collisions
    // ------------------------------------------------
    void update();

    // ------------------------------------------------
    // DRAW
    // called every frame inside BeginDrawing()
    // draws map, robber, police, treasures, UI
    // ------------------------------------------------
    void draw();

    // ------------------------------------------------
    // CHECKCOLLISIONS
    // checks if robber touched police = game over
    // checks if robber touched treasure = collect it
    // ------------------------------------------------
    void checkCollisions();

    // ------------------------------------------------
    // SPAWNTREASSURES
    // loops through map grid looking for DOOR tiles
    // only spawns treasure near BANK and PRESIDENT buildings
    // ------------------------------------------------
    void spawnTreasures();

    // ------------------------------------------------
    // SPAWNPOLICE
    // creates police objects and pushes them into policeUnits vector
    // number of police = level number but max 7
    // police spawn on random STREET tiles
    // ------------------------------------------------
    void spawnPolice();

    // ------------------------------------------------
    // DRAWUI
    // draws the sidebar on the right side of screen
    // shows level number, alarm status, diamonds collected
    // ------------------------------------------------
    void drawUI();

    // ------------------------------------------------
    // DESTRUCTOR
    // cleans up treasure pointers from memory
    // called automatically when game closes
    // ------------------------------------------------
    ~GameManager();
};

#endif