#ifndef PLAYERS_H
#define PLAYERS_H

#include "raylib.h"
#include "map.h"
#include <vector>

// base class lalalalalalalalalaallaallalaalalallaaaaaaaa
class character {
public:
    Vector2 worldpos;
    float speed;
    float rotation;
    Color color;

    character(float x, float y, float s, Color c) {
        worldpos = {x, y};
        speed = s;
        color = c;
        rotation = 0.0f;
    }

    virtual void Draw() {
        Rectangle shoulders = {worldpos.x, worldpos.y, 30, 15};
        Vector2 origin = {15, 7.5f};
        DrawRectanglePro(shoulders, origin, rotation, color);
        DrawCircleV(worldpos, 8, PINK);
    }
};


// robber classssssssssssss
class robber : public character {
public:
    robber(float x, float y) : character(x, y, 4.0f, PURPLE) {}

    void update(const CityMap& map) {
        Vector2 nextpos = worldpos;

        if (IsKeyDown(KEY_W)) { nextpos.y -= speed; rotation = 270; }
        if (IsKeyDown(KEY_S)) { nextpos.y += speed; rotation = 90; }
        if (IsKeyDown(KEY_A)) { nextpos.x -= speed; rotation = 180; }
        if (IsKeyDown(KEY_D)) { nextpos.x += speed; rotation = 0; }

        //bayanuu's map used

        int gridX = (int)(nextpos.x / map.tileSize);
        int gridY = (int)(nextpos.y / map.tileSize);

        if (gridX >= 0 && gridX < 20 && gridY >= 0 && gridY < 20) {
            // Only walk on STREET (0) or DOOR (5)
            if (map.grid[gridY][gridX] == 0 || map.grid[gridY][gridX] == 5) {
                worldpos = nextpos;
            }
        }
    }
};

// police classss beedoooobeeedoooobeedoooo
enum AIState { PATROL, CHASE };

class police : public character {
public:
    AIState state;

    police(float x, float y) : character(x, y, 3.2f, DARKBLUE) {
        state = PATROL;
    }

    void update(Vector2 robberPos, bool alarmActive) {
        
        if (alarmActive) state = CHASE;
        else state = PATROL;

        if (state == CHASE) {
            // how police moves beedooobeeeddoooo
            if (robberPos.x > worldpos.x) { worldpos.x += speed; rotation = 0; }
            if (robberPos.x < worldpos.x) { worldpos.x -= speed; rotation = 180; }
            if (robberPos.y > worldpos.y) { worldpos.y += speed; rotation = 90; }
            if (robberPos.y < worldpos.y) { worldpos.y -= speed; rotation = 270; }
        } 
        else if (state == PATROL) {
           
        }
    }
   
};
#endif


