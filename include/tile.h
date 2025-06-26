#include "raylib.h"

#ifndef TILE_H
#define TILE_H

typedef enum {
    GROUND,
    WALL1,
    WALL2,
    WALL3,
    TOTALTILES,
} MapTiles;

typedef struct maptile* Tile;

Tile TileCreate(const char* name, MapTiles maptile, const char* imgname);
void TileDestroy(Tile* tilep);

const char* TileGetName(Tile tile);
MapTiles TileGetMapTiles(Tile tile);
Texture TileGetTexture(Tile tile);

#endif