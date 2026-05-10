#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "Map.h"
#include <cmath> 

// ---------------------------------------------------------
// BASE PLAYER CLASS
// ---------------------------------------------------------
class Player {
public:
    Vector2 worldPos;
    float speed;
    float rotation;
    Color color;
    float walkTimer; 

    Player(float x, float y, float s, Color c) {
        worldPos = {x, y};
        speed = s;
        color = c;
        rotation = 0.0f;
        walkTimer = 0.0f;
    }

    virtual void Draw() = 0; 
    virtual ~Player() {}
};

// ---------------------------------------------------------
// ROBBER CLASS (Stripey "Real" Look)
// ---------------------------------------------------------
class Robber : public Player {
public:
    Robber(float x, float y) : Player(x, y, 6.0f, PURPLE) {}

    void update(const CityMap& map) {
        Vector2 nextPos = worldPos;
        bool isMoving = false;

        // Smooth Keyboard Movement
        if (IsKeyDown(KEY_W)) { nextPos.y -= speed; rotation = 270; isMoving = true; }
        if (IsKeyDown(KEY_S)) { nextPos.y += speed; rotation = 90;  isMoving = true; }
        if (IsKeyDown(KEY_A)) { nextPos.x -= speed; rotation = 180; isMoving = true; }
        if (IsKeyDown(KEY_D)) { nextPos.x += speed; rotation = 0;   isMoving = true; }

        if (isMoving) walkTimer += GetFrameTime() * 10;

        // Collision Check (Centered for 96px tiles)
        int gridX = (int)((nextPos.x + 15) / map.tileSize);
        int gridY = (int)((nextPos.y + 8) / map.tileSize);

        if (map.isWalkable(gridX, gridY)) {
            worldPos = nextPos;
        }
    }

    void Draw() override {
        float sway = sin(walkTimer) * 4; 

        // Shadow under feet for depth
        DrawCircle(worldPos.x, worldPos.y + 30, 15, Fade(BLACK, 0.2f));

        // Shirt (RAYWHITE with Stripes) - 40px wide
        DrawRectangleV({worldPos.x - 20, worldPos.y - 10 + sway}, {40, 36}, RAYWHITE); 
        DrawRectangleV({worldPos.x - 20, worldPos.y - 2 + sway}, {40, 8}, BLACK);    
        DrawRectangleV({worldPos.x - 20, worldPos.y + 14 + sway}, {40, 8}, BLACK);   
        
        // Head / Beanie
        DrawRectangleV({worldPos.x - 16, worldPos.y - 24 + sway}, {32, 24}, BLACK);  
        
        // Eyes
        DrawCircle(worldPos.x - 8, worldPos.y - 12 + sway, 4, WHITE);                
        DrawCircle(worldPos.x + 8, worldPos.y - 12 + sway, 4, WHITE);                
    }
};

// ---------------------------------------------------------
// POLICE CLASS (Big Uniform + Siren)
// ---------------------------------------------------------
enum AIState { PATROL, CHASE };

class Police : public Player {
    AIState state;
    float patrolTimer;
    float patrolDx, patrolDy;

public:
    Police(float x, float y) : Player(x, y, 2.5f, BLUE) {
        state = PATROL;
        patrolTimer = 0;
        patrolDx = 0;
        patrolDy = 0;
    }

    void update(Vector2 robberPos, bool alarmActive, const CityMap& map) {
        if (alarmActive) state = CHASE;
        else state = PATROL;

        Vector2 nextPos = worldPos;
        walkTimer += GetFrameTime() * 8;

        if (state == CHASE) {
            float dx = robberPos.x - worldPos.x;
            float dy = robberPos.y - worldPos.y;
            if (abs(dx) > abs(dy)) {
                nextPos.x += (dx > 0) ? speed : -speed;
            } else {
                nextPos.y += (dy > 0) ? speed : -speed;
            }
        } else if (state == PATROL) {
            patrolTimer -= GetFrameTime();
            if (patrolTimer <= 0) {
                int dir = GetRandomValue(0, 3);
                patrolDx = (dir == 0) ? speed : (dir == 1) ? -speed : 0;
                patrolDy = (dir == 2) ? speed : (dir == 3) ? -speed : 0;
                patrolTimer = 2.0f;
            }
            nextPos.x += patrolDx * GetFrameTime() * 60;
            nextPos.y += patrolDy * GetFrameTime() * 60;
        }

        int gridX = (int)((nextPos.x + 15) / map.tileSize);
        int gridY = (int)((nextPos.y + 8) / map.tileSize);

        if (map.isWalkable(gridX, gridY)) {
            worldPos = nextPos;
        } else {
            patrolTimer = 0; // Pick new direction if hitting a wall
        }
    }

    void Draw() override {
        float sway = sin(walkTimer) * 3;
        
        // Shadow
        DrawCircle(worldPos.x, worldPos.y + 40, 18, Fade(BLACK, 0.2f));

        // Uniform Body - 45px wide
        DrawRectangleV({worldPos.x - 22, worldPos.y - 10 + sway}, {45, 45}, BLUE);
        
        // Police Hat
        DrawRectangle(worldPos.x - 26, worldPos.y - 18 + sway, 52, 12, DARKBLUE); 
        
        // Face area
        DrawRectangleV({worldPos.x - 15, worldPos.y + 5 + sway}, {30, 20}, PINK);

        // Flashing Siren (Only during Chase)
        if (state == CHASE) {
            Color sirenColor = ((int)(GetTime() * 10) % 2 == 0) ? RED : BLUE;
            DrawCircle(worldPos.x, worldPos.y - 25 + sway, 8, sirenColor);
            DrawCircleGradient(worldPos.x, worldPos.y - 25 + sway, 30, Fade(sirenColor, 0.4f), BLANK);
        }
    }
};

#endif