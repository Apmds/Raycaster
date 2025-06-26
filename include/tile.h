#include "raylib.h"

#ifndef TILE_H
#define TILE_H

typedef enum {
    GROUND,
    WALL1,
    WALL2,
    WALL_TRANSPARENT,
    TOTALTILES,
} MapTiles;

typedef struct maptile* Tile;

// Creates a tile object given its info.
Tile TileCreate(const char* name, MapTiles maptile, const char* imgname, bool is_transparent);

// Destroys a tile object.
void TileDestroy(Tile* tilep);


// The name of a tile.
const char* TileGetName(Tile tile);

// The number (MapTiles) that this tile represents.
MapTiles TileGetMapTiles(Tile tile);

// The texture associated with this tile.
Texture TileGetTexture(Tile tile);

// True if the tile is transparent
bool TileIsTransparent(Tile tile);

#endif