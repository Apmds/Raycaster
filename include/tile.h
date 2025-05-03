#include "raylib.h"

#ifndef TILE_H
#define TILE_H

typedef enum {
    GROUND,
    WALL,
} MapTiles;

typedef struct maptile* Tile;

Tile TileCreate();
void TileDestroy(Tile* tilep);

void TileGetName(Tile tile);
MapTiles TileGetMapTiles(Tile tile);
Image TileGetImage(Tile tile);

#endif