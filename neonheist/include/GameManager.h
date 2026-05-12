#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "raylib.h"
#include "Map.h"
#include "Player.h"       
#include "Treasure.h"
#include "AlarmSystem.h"
#include "InventoryManager.h"
#include <vector>         

class GameManager {
public:

    
    int level;            
    bool gameOver;        
    bool gameWon;         
    bool alarmActive;     
    bool levelComplete; 

    // game objs

    CityMap map;                        
    Robber robber;                      
    std::vector<Police> policeUnits; 
    std::vector<Bystander> bystanders;    
    std::vector<Treasure*> treasures;   


    Music bgMusic;
    Music alarmSound;
    Sound collectSound;
    Sound gameOverSound;


    Camera2D camera;



    InventoryManager inventory;


    GameManager();

    
    void startLevel(); //sets random spwans 

    void update();

    
    void draw();

   
    void checkCollisions();

    
    void spawnTreasures();

    void spawnPolice();

    void spawnBystanders(); 

    
    void drawUI();

    
    ~GameManager();
};

#endif