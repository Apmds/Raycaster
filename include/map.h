#ifndef MAP_H
#define MAP_H

typedef enum MapTile {
    GROUND,
    WALL,
} MapTile;

typedef struct map* Map;

Map MapCreate(int numRows, int numCols);
Map MapCreateFromFile(const char* filename);
void MapDestroy(Map* mp);

void MapSetTile(Map map, int row, int col, MapTile tile);
MapTile MapGetTile(Map map, int row, int col);

int MapGetTileSize(Map map);
int MapGetNumRows(Map map);
int MapGetNumCols(Map map);


void MapDraw2D(Map map);

#endif