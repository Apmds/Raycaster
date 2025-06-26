#include "raylib.h"
#include "tile.h"

#ifndef MAP_H
#define MAP_H

typedef struct map* Map;

Map MapCreate(int numRows, int numCols, int tileSize);
Map MapCreateFromFile(const char* filename);
void MapDestroy(Map* mp);

void MapSetTile(Map map, int row, int col, MapTiles tile);
MapTiles MapGetTile(Map map, int row, int col);
Tile MapGetTileObject(Map map, MapTiles tile);

int MapGetTileSize(Map map);
int MapGetNumRows(Map map);
int MapGetNumCols(Map map);

Texture MapGetTextureAt(Map map, int row, int col);

void MapDraw2D(Map map);

#endif