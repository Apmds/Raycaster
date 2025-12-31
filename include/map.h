#include "raylib.h"
#include "tile.h"
#include "billboard.h"
#include "list.h"

#ifndef MAP_H
#define MAP_H

typedef struct map* Map;

Map MapCreate(int numRows, int numCols, int tileSize);
Map MapCreateFromFile(const char* filename);
void MapDestroy(Map* mp);

void MapSetTile(Map map, int row, int col, int tile);
int MapGetTile(Map map, int row, int col);
Tile MapGetTileObject(Map map, int tile);

int MapGetTileSize(Map map);
int MapGetNumRows(Map map);
int MapGetNumCols(Map map);

List MapGetBillboardsAt(Map map, int col, int row);

Texture MapGetTextureAt(Map map, int row, int col);

void MapDraw2D(Map map);
void MapDraw3D(Map map, int screenWidth, int screenHeight);

#endif