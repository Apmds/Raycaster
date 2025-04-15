#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "map.h"
#include "raylib.h"

struct map {
    int numRows;
    int numCols;
    int tileSize;                   // Tamanho de cada quadrado
    Texture wall_img;                 // TEMPORARY
    MapTile** grid;
};

Map MapCreate(int numRows, int numCols) {
    Map map = malloc(sizeof(struct map));
    assert(map != NULL);
    assert(numRows > 0);
    assert(numCols > 0);

    // Initialize grid
    map->grid = malloc(sizeof(MapTile*)*numRows);
    assert(map->grid != NULL);
    for (int i = 0; i < numRows; i++) {
        map->grid[i] = calloc(numCols, sizeof(MapTile));
        assert(map->grid[i] != NULL);
    }

    map->numCols = numCols;
    map->numRows = numRows;
    map->tileSize = 30;

    // TEMPORARY
    map->wall_img = LoadTexture("map.png");

    return map;
}

Map MapCreateFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file!");
        exit(EXIT_FAILURE);
    }
    
    Map map = malloc(sizeof(struct map));
    assert(map != NULL);

    int numRows = 0;
    int numCols = 0;
    int tileSize = 0;

    char line[500];
    int lineIdx = 0;
    
    while (fgets(line, sizeof(line), file) != NULL) {
        // Ignore empty lines
        size_t line_size = strlen(line);
        if (line_size == 0 || line[0] == '\n' || line_size == strspn(line, " \r\n\t")) {
            continue;
        }
        
        // Format first line
        if (lineIdx == 0) {
            if (sscanf(line, "%d,%d,%d", &numRows, &numCols, &tileSize) != 3) {
                printf("Map properties not formatted correctly: the first line must be: <numRows>,<numCols>,<tileSize>");
                exit(EXIT_FAILURE);
            }

            // Initialize grid
            map->grid = malloc(sizeof(MapTile*)*numRows);
            assert(map->grid != NULL);
            for (int i = 0; i < numRows; i++) {
                map->grid[i] = calloc(numCols, sizeof(MapTile));
                assert(map->grid[i] != NULL);
            }
        
            map->numCols = numCols;
            map->numRows = numRows;
            map->tileSize = tileSize;

        } else {
            // Read tiles
            int tileX = 0;
            int tileY = 0;
            char tileName[50];
            if (sscanf(line, "%d,%d,%s", &tileX, &tileY, tileName) != 3) {
                printf("Map properties not formatted correctly: the line must be: <tileX>,<tileY>,<tileName>");
                exit(EXIT_FAILURE);
            }
            
            // Set tiles
            if (strcmp(tileName, "WALL") == 0) {
                MapSetTile(map, tileX, tileY, WALL);
            }
            if (strcmp(tileName, "GROUND") == 0) {
                MapSetTile(map, tileX, tileY, GROUND);
            }
        }
            
        lineIdx++;
    }
    fclose(file);

    // TEMPORARY
    map->wall_img = LoadTexture("map.png");

    return map;
}

void MapDestroy(Map* mp) {
    assert(mp != NULL);
    assert(*mp != NULL);

    Map map = *mp;

    // Destroy grid
    for (int i = 0; i < map->numRows; i++) {
        free(map->grid[i]);
    }
    free(map->grid);

    UnloadTexture(map->wall_img);
    
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
    if (row >= map->numRows || row < 0 || col >= map->numCols || col < 0) {
        return GROUND;
    }

    return map->grid[row][col];
}

int MapGetTileSize(Map map) {
    assert(map != NULL);
    
    return map->tileSize;
}

int MapGetNumRows(Map map) {
    assert(map != NULL);

    return map->numRows;
}

int MapGetNumCols(Map map) {
    assert(map != NULL);

    return map->numCols;
}

Texture MapGetTextureAt(Map map, int row, int col) {
    assert(map != NULL);

    // TEMPORARY
    return map->wall_img;
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
            DrawRectangle(row*map->tileSize, col*map->tileSize, map->tileSize, map->tileSize, color);
        }
    }
}
