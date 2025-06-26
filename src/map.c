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

static bool hashmapstrcmp(void* key1, void* key2) {
    return strcmp((char*) key1, (char*) key2) == 0;
}

struct map {
    int numRows;
    int numCols;
    int tileSize;                       // Size of each tile (pixels)
    HashMap tileMap;                    // HashMap that contains the details (texture) for a tile, given its name
    char** tileNames;                   // Array associating tile indices (MapTiles) to the tile names (char*)
    MapTiles** grid;                    // The grid of tiles that represents this map
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

    map->tileNames = (char**) malloc(TOTALTILES*sizeof(char*));
    map->tileNames[GROUND] = "GROUND";
    map->tileNames[WALL1] = "WALL1";
    map->tileNames[WALL2] = "WALL2";
    map->tileNames[WALL_TRANSPARENT] = "WALL_TRANSPARENT";

    map->tileMap = HashMapCreate(5, djb2hash, hashmapstrcmp);
    HashMapPut(map->tileMap, "GROUND", TileCreate("GROUND", GROUND, "test.png", false));
    HashMapPut(map->tileMap, "WALL1", TileCreate("WALL1", WALL1, "map.png", false));
    HashMapPut(map->tileMap, "WALL2", TileCreate("WALL2", WALL2, "map2.png", false));
    HashMapPut(map->tileMap, "WALL_TRANSPARENT", TileCreate("WALL_TRANSPARENT", WALL_TRANSPARENT, "wabbit_alpha.png", true));

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
    int lineNumber = 0; // Used for error printing
    int lineIdx = 0; // Used for getting the index of the current line (only updates after processing a non-empty file)
    
    map->tileNames = (char**) malloc(TOTALTILES*sizeof(char*));
    map->tileNames[GROUND] = "GROUND";
    map->tileNames[WALL1] = "WALL1";
    map->tileNames[WALL2] = "WALL2";
    map->tileNames[WALL_TRANSPARENT] = "WALL_TRANSPARENT";

    // TEMPORARY (change to add tiles when reading the map data)
    map->tileMap = HashMapCreate(5, djb2hash, hashmapstrcmp);
    HashMapPut(map->tileMap, "GROUND", TileCreate("GROUND", GROUND, "test.png", false));
    HashMapPut(map->tileMap, "WALL1", TileCreate("WALL1", WALL1, "map.png", false));
    HashMapPut(map->tileMap, "WALL2", TileCreate("WALL2", WALL2, "map2.png", false));
    HashMapPut(map->tileMap, "WALL_TRANSPARENT", TileCreate("WALL_TRANSPARENT", WALL_TRANSPARENT, "wabbit_alpha.png", true));

    while (fgets(line, sizeof(line), file) != NULL) {
        lineNumber++;

        // Ignore empty lines
        size_t line_size = strlen(line);
        if (line_size == 0 || line[0] == '\n' || line_size == strspn(line, " \r\n\t")) {
            continue;
        }
        
        // Format first line
        if (lineIdx == 0) {
            if (sscanf(line, " %d , %d , %d ", &numRows, &numCols, &tileSize) != 3) {
                printf("Line %d: Map properties not formatted correctly: the first line must be: <numRows>,<numCols>,<tileSize>\n", lineNumber);
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
            if (sscanf(line, " %d , %d , %50s ", &tileX, &tileY, tileName) != 3) {
                printf("Line %d: Map properties not formatted correctly: the line must be: <tileX>,<tileY>,<tileName>\n", lineNumber);
                exit(EXIT_FAILURE);
            }
            
            // Set tiles
            if (!HashMapContains(map->tileMap, tileName)) {
                printf("Line %d: Tile \"%s\" does not exist!\n", lineNumber, tileName);
                exit(EXIT_FAILURE);
            }
            Tile tile = (Tile) HashMapGet(map->tileMap, tileName);
            MapSetTile(map, tileX, tileY, TileGetMapTiles(tile));
        }
            
        lineIdx++;
    }
    fclose(file);

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

    // Clear tilemap
    Tile tile = (Tile) HashMapGet(map->tileMap, "GROUND"); TileDestroy(&tile);
    tile = (Tile) HashMapGet(map->tileMap, "WALL1"); TileDestroy(&tile);
    tile = (Tile) HashMapGet(map->tileMap, "WALL2"); TileDestroy(&tile);
    tile = (Tile) HashMapGet(map->tileMap, "WALL_TRANSPARENT"); TileDestroy(&tile);

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

Tile MapGetTileObject(Map map, MapTiles tile) {
    assert(map != NULL);
    return HashMapGet(map->tileMap, map->tileNames[tile]);
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
