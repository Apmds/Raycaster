#include <stdlib.h>
#include <assert.h>
#include "map.h"
#include "raylib.h"

struct map {
    int numRows;
    int numCols;
    int gridSize;                   // Tamanho de cada grelha
    MapTile** grid;
};

Map MapCreate(int numRows, int numCols) {
    Map map = malloc(sizeof(struct map));
    assert(map != NULL);
    assert(numRows > 0);
    assert(numCols > 0);

    // Inicializar grid
    map->grid = malloc(sizeof(MapTile*)*numRows);
    assert(map->grid != NULL);
    for (int i = 0; i < numRows; i++) {
        map->grid[i] = calloc(numCols, sizeof(MapTile));
        assert(map->grid[i] != NULL);
    }

    map->numCols = numCols;
    map->numRows = numRows;
    map->gridSize = 30;

    return map;
}

void MapDestroy(Map* mp) {
    assert(mp != NULL);
    assert(*mp != NULL);

    Map map = *mp;

    // Destruir grid
    for (int i = 0; i < map->numRows; i++) {
        free(map->grid[i]);
    }
    free(map->grid);

    free(map);

    *mp = NULL;
}

void MapSetTile(Map map, int row, int col, MapTile tile) {
    assert(map != NULL);
    assert(row >= 0);
    assert(col >= 0);
    
    map->grid[row][col] = tile;
}

MapTile MapGetTile(Map map, int row, int col) {
    assert(map != NULL);

    return map->grid[row][col];
}

void MapDraw2D(Map map) {
    assert(map != NULL);

    for (int row = 0; row < map->numRows; row++) {
        for (int col = 0; col < map->numCols; col++) {
            Color color;
            if (map->grid[row][col] == WALL) {
                color = (Color) {255, 255, 255, 255};
            } else {
                color = (Color) {0, 0, 0, 255};
            }
            DrawRectangle(row*map->gridSize, col*map->gridSize, map->gridSize, map->gridSize, color);
        }
    }
}
