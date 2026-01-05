#include "raylib.h"
#include "tile.h"
#include "billboard.h"
#include "list.h"

#ifndef MAP_H
#define MAP_H

typedef struct map* Map;
typedef const struct map* CMap;

Map MapCreate(int numRows, int numCols, int tileSize);
Map MapCreateFromFile(const char* filename);
void MapDestroy(Map* mp);

void MapSetTile(CMap map, int row, int col, int tile);
int MapGetTile(CMap map, int row, int col);
Tile MapGetTileObject(Map map, int tile);

int MapGetTileSize(CMap map);
int MapGetNumRows(CMap map);
int MapGetNumCols(CMap map);

List MapGetBillboardsAt(Map map, int col, int row);

Texture MapGetTextureAt(Map map, int row, int col);

void MapDraw2D(Map map);
void MapDraw3D(Map map, int screenWidth, int screenHeight);

#endif