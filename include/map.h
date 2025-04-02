#ifndef MAP_H
#define MAP_H

typedef enum MapTile {
    GROUND,
    WALL,
} MapTile;

typedef struct map* Map;

Map MapCreate(int numRows, int numCols);
void MapDestroy(Map* mp);

void MapSetTile(Map map, int row, int col, MapTile tile);
MapTile MapGetTile(Map map, int row, int col);

void MapDraw2D(Map map);

#endif