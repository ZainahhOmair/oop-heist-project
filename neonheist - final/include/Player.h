#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "Map.h"
#include <cmath>

// base class
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

// robber class
class Robber : public Player {
public:
    Robber(float x, float y) : Player(x, y, 6.0f, PURPLE) {}

    void update(const CityMap& map) {
        Vector2 nextPos = worldPos;
        bool isMoving = false;

        // arrow keys for movement
        if (IsKeyDown(KEY_UP))    { nextPos.y -= speed; rotation = 270; isMoving = true; }
        if (IsKeyDown(KEY_DOWN))  { nextPos.y += speed; rotation = 90;  isMoving = true; }
        if (IsKeyDown(KEY_LEFT))  { nextPos.x -= speed; rotation = 180; isMoving = true; }
        if (IsKeyDown(KEY_RIGHT)) { nextPos.x += speed; rotation = 0;   isMoving = true; }

        if (isMoving) walkTimer += GetFrameTime() * 10;

        int gridX = (int)((nextPos.x + 15) / map.tileSize);
        int gridY = (int)((nextPos.y + 8)  / map.tileSize);

        if (map.isWalkable(gridX, gridY)) {
            worldPos = nextPos;
        }
    }

    void Draw() override {
        // no sway - clean stable drawing
        DrawCircle(worldPos.x, worldPos.y + 30, 15, Fade(BLACK, 0.2f));

        // striped shirt
        DrawRectangleV({worldPos.x - 20, worldPos.y - 10}, {40, 36}, RAYWHITE);
        DrawRectangleV({worldPos.x - 20, worldPos.y - 2},  {40, 8},  BLACK);
        DrawRectangleV({worldPos.x - 20, worldPos.y + 14}, {40, 8},  BLACK);

        // head and beanie
        DrawRectangleV({worldPos.x - 16, worldPos.y - 24}, {32, 24}, BLACK);

        // eyes
        DrawCircle(worldPos.x - 8, worldPos.y - 12, 4, WHITE);
        DrawCircle(worldPos.x + 8, worldPos.y - 12, 4, WHITE);
    }
};

// police AI states
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
        int gridY = (int)((nextPos.y + 8)  / map.tileSize);

        if (map.isWalkable(gridX, gridY)) {
            worldPos = nextPos;
        } else {
            patrolTimer = 0;
        }
    }

    void Draw() override {
        // no sway
        DrawCircle(worldPos.x, worldPos.y + 40, 18, Fade(BLACK, 0.2f));

        // uniform body
        DrawRectangleV({worldPos.x - 22, worldPos.y - 10}, {45, 45}, BLUE);

        // police hat
        DrawRectangle(worldPos.x - 26, worldPos.y - 18, 52, 12, DARKBLUE);

        // face
        DrawRectangleV({worldPos.x - 15, worldPos.y + 5}, {30, 20}, PINK);

        // flashing siren only during chase
        if (state == CHASE) {
            Color sirenColor = ((int)(GetTime() * 10) % 2 == 0) ? RED : BLUE;
            DrawCircle(worldPos.x, worldPos.y - 25, 8, sirenColor);
        }
    }
};


// bystander class - civilians walking around
class Bystander : public Player {
    float moveTimer;
    float moveDx, moveDy;

public:
    Bystander(float x, float y) : Player(x, y, 2.0f, BEIGE) {
        moveTimer = 0;
        moveDx = 0;
        moveDy = 0;
    }

    void update(Vector2 robberPos, bool alarmActive, const CityMap& map) {
        Vector2 nextPos = worldPos;
        moveTimer -= GetFrameTime();

        if (alarmActive) {
            // run away from robber in opposite direction
            if (moveTimer <= 0) {
                float dx = worldPos.x - robberPos.x;
                float dy = worldPos.y - robberPos.y;

                // move away from robber
                if (abs(dx) > abs(dy)) {
                    moveDx = (dx > 0) ? speed * 2 : -speed * 2;
                    moveDy = 0;
                } else {
                    moveDx = 0;
                    moveDy = (dy > 0) ? speed * 2 : -speed * 2;
                }
                moveTimer = 0.5f; // change direction every 0.5 seconds
            }
        } else {
            // normal state - walk randomly
            if (moveTimer <= 0) {
                int dir = GetRandomValue(0, 4);
                moveDx = 0;
                moveDy = 0;
                if (dir == 0) moveDx =  speed;
                if (dir == 1) moveDx = -speed;
                if (dir == 2) moveDy =  speed;
                if (dir == 3) moveDy = -speed;
                // dir == 4 means stand still for a moment
                moveTimer = GetRandomValue(1, 3); // random wait 1-3 seconds
            }
        }

        nextPos.x += moveDx * GetFrameTime() * 60;
        nextPos.y += moveDy * GetFrameTime() * 60;

        // collision check same as police and robber
        int gridX = (int)((nextPos.x + 15) / map.tileSize);
        int gridY = (int)((nextPos.y + 8)  / map.tileSize);

        if (map.isWalkable(gridX, gridY)) {
            worldPos = nextPos;
        } else {
            moveTimer = 0; // hit wall, pick new direction
        }
    }

    void Draw() override {
        // simple civilian look - different from robber and police
        // shadow
        DrawCircle(worldPos.x, worldPos.y + 30, 12, Fade(BLACK, 0.15f));

        // body - casual clothes
        DrawRectangleV({worldPos.x - 15, worldPos.y - 8}, {30, 32}, BEIGE);

        // head
        DrawCircle(worldPos.x, worldPos.y - 16, 14, BEIGE);

        // hair
        DrawRectangle(worldPos.x - 14, worldPos.y - 30, 28, 10, BROWN);
    }
};

#endif