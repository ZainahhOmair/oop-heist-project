#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "Map.h"
#include <cmath> 

class Player {
public:
    Vector2 worldPos;
    float speed;
    float rotation;
    Color color;
    float walkTimer; // Added for the "sway" animation

    Player(float x, float y, float s, Color c) {
        worldPos = {x, y};
        speed = s;
        color = c;
        rotation = 0.0f;
        walkTimer = 0.0f;
    }

    // Base Draw handles the "Body" logic
    virtual void Draw() = 0; 
    virtual ~Player() {}
};

class Robber : public Player {
public:
    Robber(float x, float y) : Player(x, y, 4.0f, PURPLE) {}

    void update(const CityMap& map) {
        Vector2 nextPos = worldPos;
        bool isMoving = false;

        if (IsKeyDown(KEY_W)) { nextPos.y -= speed; rotation = 270; isMoving = true; }
        if (IsKeyDown(KEY_S)) { nextPos.y += speed; rotation = 90;  isMoving = true; }
        if (IsKeyDown(KEY_A)) { nextPos.x -= speed; rotation = 180; isMoving = true; }
        if (IsKeyDown(KEY_D)) { nextPos.x += speed; rotation = 0;   isMoving = true; }

        if (isMoving) walkTimer += GetFrameTime() * 10; // Increase timer for sway

        int gridX = (int)((nextPos.x + 15) / map.tileSize);
        int gridY = (int)((nextPos.y + 8) / map.tileSize);

        if (map.isWalkable(gridX, gridY)) {
            worldPos = nextPos;
        }
    }

    void Draw() override {
        float sway = sin(walkTimer) * 3; // The "bobbing" effect

        // Draw Robber - Stripey Shirt look
        // We use worldPos.x/y as the center
        DrawRectangleV({worldPos.x - 10, worldPos.y - 5 + sway}, {20, 18}, RAYWHITE); // Shirt
        DrawRectangleV({worldPos.x - 10, worldPos.y - 1 + sway}, {20, 4}, BLACK);    // Stripe 1
        DrawRectangleV({worldPos.x - 10, worldPos.y + 7 + sway}, {20, 4}, BLACK);    // Stripe 2
        
        // Head / Beanie
        DrawRectangleV({worldPos.x - 8, worldPos.y - 12 + sway}, {16, 12}, BLACK);   // Beanie
        DrawCircle(worldPos.x - 4, worldPos.y - 6 + sway, 2, WHITE);                 // Left Eye
        DrawCircle(worldPos.x + 4, worldPos.y - 6 + sway, 2, WHITE);                 // Right Eye
    }
};

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
        walkTimer += GetFrameTime() * 8; // Police always have a slight idle sway

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
            patrolTimer = 0;
        }
    }

    void Draw() override {
        float sway = sin(walkTimer) * 2;
        
        // Body (Uniform)
        DrawRectangleV({worldPos.x - 10, worldPos.y - 5 + sway}, {20, 20}, BLUE);
        
        // Hat
        DrawRectangle(worldPos.x - 13, worldPos.y - 8 + sway, 26, 6, DARKBLUE); 
        
        // Flashing Siren (Only when Chasing!)
        if (state == CHASE) {
            Color sirenColor = ((int)(GetTime() * 10) % 2 == 0) ? RED : BLUE;
            DrawCircle(worldPos.x, worldPos.y - 12 + sway, 5, sirenColor);
            DrawCircleGradient(worldPos.x, worldPos.y - 12 + sway, 15, Fade(sirenColor, 0.3f), BLANK);
        }
        
        // Face
        DrawCircle(worldPos.x, worldPos.y + sway, 6, PINK); 
    }
};

#endif