#ifndef TREASURE_H
#define TREASURE_H

#include "raylib.h"
#include "Map.h"

class Treasure {
public:
    int tileX, tileY;       // diamnd ki position world map per
    bool collected;          // picked or not

    Treasure(int x, int y) {
        tileX = x;
        tileY = y;
        collected = false;
    }

    void Draw(int tileSize) {
        if (!collected) {
            float centerX = tileX * tileSize + tileSize / 2;
            float centerY = tileY * tileSize + tileSize / 2;
    
            // bigger diamond using multiple rotated rectangles
            Rectangle diamond = {centerX, centerY, 50, 50};
            Vector2 origin = {25, 25};
            DrawRectanglePro(diamond, origin, 45, SKYBLUE);   // outer diamond
            Rectangle inner = {centerX, centerY, 30, 30};
            Vector2 innerOrigin = {15, 15};
            DrawRectanglePro(inner, innerOrigin, 45, BLUE);    // inner shine
            DrawRectanglePro({centerX, centerY, 10, 10}, {5, 5}, 45, WHITE);  // sparkle
        }
    }

    bool isNear(Vector2 robberPos, int tileSize) {
        int robberTileX = (int)((robberPos.x + 15) / tileSize);
        int robberTileY = (int)((robberPos.y + 8)  / tileSize);
        return (robberTileX == tileX && robberTileY == tileY);
    }
};

#endif