#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "map.h"
#include "raylib.h"
#include "hashmap.h"
#include <stdbool.h>
#include "tile.h"

// djb2 hash
static unsigned int djb2hash(void* key) {
    char* str = (char*) key;

    int hash = 5381;
    int c;

    while (c = *str++)
       hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

struct map {
    int numRows;
    int numCols;
    int tileSize;                       // Size of each tile (pixels)
    Texture wall_img;                   // TEMPORARY
    HashMap tileMap;
    char** tileNames;
    MapTiles** grid;
};

Map MapCreate(int numRows, int numCols, int tileSize) {
    Map map = malloc(sizeof(struct map));
    assert(map != NULL);
    assert(numRows > 0);
    assert(numCols > 0);

    // Initialize grid
    map->grid = malloc(sizeof(MapTiles*)*numRows);
    assert(map->grid != NULL);
    for (int i = 0; i < numRows; i++) {
        map->grid[i] = calloc(numCols, sizeof(MapTiles));
        assert(map->grid[i] != NULL);
    }

    map->numCols = numCols;
    map->numRows = numRows;
    map->tileSize = tileSize;

    // TEMPORARY
    map->wall_img = LoadTexture("test.png");

    map->tileMap = HashMapCreate(5, djb2hash);
    HashMapPut(map->tileMap, "GROUND", TileCreate("GROUND", GROUND, "test.png"));
    HashMapPut(map->tileMap, "WALL1", TileCreate("WALL1", WALL1, "map.png"));
    HashMapPut(map->tileMap, "WALL2", TileCreate("WALL2", WALL2, "map2.png"));
    HashMapPut(map->tileMap, "WALL3", TileCreate("WALL3", WALL3, "wabbit_alpha.png"));

    map->tileNames = (char**) malloc(TOTALTILES*sizeof(char*));
    map->tileNames[0] = "GROUND";
    map->tileNames[1] = "WALL1";
    map->tileNames[2] = "WALL2";
    map->tileNames[3] = "WALL3";

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
            map->grid = malloc(sizeof(MapTiles*)*numRows);
            assert(map->grid != NULL);
            for (int i = 0; i < numRows; i++) {
                map->grid[i] = calloc(numCols, sizeof(MapTiles));
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
                MapSetTile(map, tileX, tileY, WALL1);
            }
            if (strcmp(tileName, "GROUND") == 0) {
                MapSetTile(map, tileX, tileY, GROUND);
            }
        }
            
        lineIdx++;
    }
    fclose(file);

    // TEMPORARY
    map->wall_img = LoadTexture("test.png");

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

    // TEMPORARY
    UnloadTexture(map->wall_img);

    Tile tile = (Tile) HashMapGet(map->tileMap, "GROUND"); TileDestroy(&tile);
    tile = (Tile) HashMapGet(map->tileMap, "WALL1"); TileDestroy(&tile);
    tile = (Tile) HashMapGet(map->tileMap, "WALL2"); TileDestroy(&tile);
    tile = (Tile) HashMapGet(map->tileMap, "WALL3"); TileDestroy(&tile);

    HashMapDestroy(&(map->tileMap));
    free(map->tileNames);
    
    free(map);

    *mp = NULL;
}

void MapSetTile(Map map, int row, int col, MapTiles tile) {
    assert(map != NULL);
    assert(row >= 0);
    assert(col >= 0);
    
    map->grid[row][col] = tile;
}

MapTiles MapGetTile(Map map, int row, int col) {
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

    return TileGetTexture((Tile) HashMapGet(map->tileMap, map->tileNames[MapGetTile(map, row, col)]));
}


void MapDraw2D(Map map) {
    assert(map != NULL);

    for (int row = 0; row < map->numRows; row++) {
        for (int col = 0; col < map->numCols; col++) {
            Color color;
            if (map->grid[row][col] == GROUND) {
                color = (Color) {0, 0, 0, 255};
            } else {
                color = (Color) {255, 255, 255, 255};
            }
            DrawRectangle(row*map->tileSize, col*map->tileSize, map->tileSize, map->tileSize, color);
        }
    }
}
