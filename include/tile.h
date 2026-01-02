#include "raylib.h"

#ifndef TILE_H
#define TILE_H

// 0 always represents a GROUND tile
#define TILE_GROUND 0

typedef struct maptile* Tile;

// Creates a tile object given its info.
Tile TileCreateTextured(char* name, int maptile, const char* imgname, bool is_transparent);
Tile TileCreateColored(char* name, int maptile, Color color);

// Destroys a tile object.
void TileDestroy(Tile* tilep);


// The name of a tile.
char* TileGetName(Tile tile);

// The number (MapTiles) that this tile represents.
int TileGetMapTiles(Tile tile);

// The texture associated with this tile.
Texture TileGetTexture(Tile tile);

// True if the tile is transparent
bool TileIsTransparent(Tile tile);

#endif