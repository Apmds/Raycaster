#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "map.h"
#include "raylib.h"
#include "hashmap.h"
#include <stdbool.h>
#include "tile.h"
#include "list.h"


struct map {
    int numRows;
    int numCols;
    int tileSize;                       // Size of each tile (pixels)
    HashMap tileMap;                    // HashMap that contains the details (texture) for a tile, given its name
    List tileNames;                   // Array associating tile indices (MapTiles) to the tile names (char*)             TODO: change to ArrayList
    Color  groundColor;     // TEMPORARY
    Color  ceilingColor;    // TEMPORARY
    int** grid;                    // The grid of tiles that represents this map
};

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

// INTERNAL: registers a new tile type in a map. Returns false on error (if tile with that name already exists)
static bool registerTileData(Map map, char* tileName, char* fileName, bool isTransparent, int* tileID) {
    if (HashMapContains(map->tileMap, tileName)) { // Duplicate checking
        return false;
    }
    
    // Add information in map
    HashMapPut(map->tileMap, tileName, TileCreate(tileName, *tileID, fileName, isTransparent));
    ListAppendLast(map->tileNames, tileName);

    // Advances to the next tile
    (*tileID)++;

    return true;
}

// DO NOT USE NOW
// TODO: alterar para receber um HashMap de cenas para preencher o tileNames e tileMap
Map MapCreate(int numRows, int numCols, int tileSize) {
    Map map = malloc(sizeof(struct map));
    assert(map != NULL);
    assert(numRows > 0);
    assert(numCols > 0);

    // Initialize grid
    map->grid = malloc(sizeof(int*)*numRows);
    assert(map->grid != NULL);
    for (int i = 0; i < numRows; i++) {
        map->grid[i] = calloc(numCols, sizeof(int));
        assert(map->grid[i] != NULL);
    }

    map->numCols = numCols;
    map->numRows = numRows;
    map->tileSize = tileSize;

    map->tileNames = ListCreate(NULL);
    char* ground = calloc(7, sizeof(char)); assert(ground != NULL); ground = strncpy(ground, "GROUND", 6); ListAppendLast(map->tileNames, ground);

    map->tileMap = HashMapCreate(5, djb2hash, hashmapstrcmp);
    HashMapPut(map->tileMap, ground, TileCreate(ground, TILE_GROUND, "test.png", false));

    // TEMPORARY
    map->ceilingColor = (Color) {255, 255, 255, 255};
    map->groundColor = (Color) {128, 100, 20, 255};


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
    int tileID = 1;     // ID of next tile to be defined

    char line[500];
    int lineNumber = 0; // Used for error printing
    int currentPhase = 0; // 0 - map general settings; 1 - reading tile data; 2 - tile placing
    
    map->tileNames = ListCreate(NULL);
    char* ground = calloc(7, sizeof(char)); assert(ground != NULL); ground = strncpy(ground, "GROUND", 6); ListAppendLast(map->tileNames, ground);

    // Default tileMap values
    map->tileMap = HashMapCreate(5, djb2hash, hashmapstrcmp);
    HashMapPut(map->tileMap, ground, TileCreate(ground, TILE_GROUND, "test.png", false));

    // TEMPORARY
    map->ceilingColor = (Color) {255, 255, 255, 255};
    map->groundColor = (Color) {128, 100, 20, 255};

    while (fgets(line, sizeof(line), file) != NULL) {
        lineNumber++;

        // Ignore empty lines
        size_t line_size = strlen(line);
        if (line_size == 0 || line[0] == '\n' || line_size == strspn(line, " \r\n\t")) {
            continue;
        }
        
        // Format first line
        if (currentPhase == 0) {
            if (sscanf(line, " %d , %d , %d ", &numRows, &numCols, &tileSize) != 3) {
                printf("Line %d: Map properties not formatted correctly: the first line must be: <numRows>,<numCols>,<tileSize>\n", lineNumber);
                exit(EXIT_FAILURE);
            }
            
            // Initialize grid
            map->grid = malloc(sizeof(int*)*numRows);
            assert(map->grid != NULL);
            for (int i = 0; i < numRows; i++) {
                map->grid[i] = calloc(numCols, sizeof(int));
                assert(map->grid[i] != NULL);
            }
            
            map->numCols = numCols;
            map->numRows = numRows;
            map->tileSize = tileSize;
            
            currentPhase++;
            continue;
        }
        
        // Getting tile data
        if (currentPhase == 1) {
            char* tileName = calloc(50, sizeof(char)); assert(tileName != NULL); // On the heap to be saved later
            char fileName[50];
            char transparent[12];
            bool isTransparent;

            int nSelected = sscanf(line, " %50s : %50s , %12s ", tileName, fileName, transparent);
            if (nSelected < 2) { // May be fit for the next phase
                free(tileName);
                currentPhase++;
            } else if (nSelected == 2) { // No other options (currently only transparency)
                isTransparent = false;
                registerTileData(map, tileName, fileName, isTransparent, &tileID);
            } else if (nSelected == 3) { // Transparent
                
                if (strcmp(transparent, "transparent") != 0) {
                    printf("Option \"%s\" does not exist!\n", transparent);
                    exit(EXIT_FAILURE);
                }
                
                isTransparent = true;
                registerTileData(map, tileName, fileName, isTransparent, &tileID);
            } else {    // Not fit for here and for the next phase) (TODO: change to use regular expressions)
                printf("Line %d: Tile definition incorrectly formatted (must be: <tileName> : <filename> [ , transparent])\n", lineNumber);
                exit(EXIT_FAILURE);
            }
        
        }

        // Reading tiles
        if (currentPhase == 2) {
            // Read tiles
            int tileX = 0;
            int tileY = 0;
            char tileName[50];
            if (sscanf(line, " %d , %d , %50s ", &tileX, &tileY, tileName) != 3) {
                printf("Line %d: Map properties not incorrectly formatted (must be <tileX>,<tileY>,<tileName>)\n", lineNumber);
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

    // Clear (free) tiles in tilemap
    HashMapIterator iter = HashMapGetIterator(map->tileMap);
    while (HashMapIterCanOperate(iter)) {
        char* str = (char*) HashMapIterGetCurrent(iter);
        Tile tile = (Tile) HashMapGet(map->tileMap, str);

        TileDestroy(&tile);

        HashMapIterGoToNext(iter);
    }
    HashMapIterDestroy(&iter);
    HashMapDestroy(&(map->tileMap));

    // Destroy tileNames
    ListMoveToStart(map->tileNames);
    while (ListCanOperate(map->tileNames)) {
        char* item = (char*) ListPopFirst(map->tileNames);
        free(item);
    }
    ListDestroy(&(map->tileNames));
    
    free(map);

    *mp = NULL;
}

void MapSetTile(Map map, int row, int col, int tile) {
    assert(map != NULL);
    assert(row >= 0);
    assert(col >= 0);
    
    map->grid[row][col] = tile;
}

int MapGetTile(Map map, int row, int col) {
    assert(map != NULL);
    if (row >= map->numRows || row < 0 || col >= map->numCols || col < 0) {
        return TILE_GROUND;
    }

    return map->grid[row][col];
}

Tile MapGetTileObject(Map map, int tile) {
    assert(map != NULL);
    return HashMapGet(map->tileMap, ListGet(map->tileNames, tile));
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

    return TileGetTexture((Tile) HashMapGet(map->tileMap, ListGet(map->tileNames, MapGetTile(map, row, col))));
}


void MapDraw2D(Map map) {
    assert(map != NULL);

    for (int row = 0; row < map->numRows; row++) {
        for (int col = 0; col < map->numCols; col++) {
            Color color;
            if (map->grid[row][col] == TILE_GROUND) {
                color = (Color) {0, 0, 0, 255};
            } else {
                color = (Color) {255, 255, 255, 255};
            }
            DrawRectangle(row*map->tileSize, col*map->tileSize, map->tileSize, map->tileSize, color);
        }
    }
}

void MapDraw3D(Map map, int screenWidth, int screenHeight) {
    assert(map != NULL);

    DrawRectangle(0, 0, screenWidth, screenHeight/2, map->ceilingColor);
    DrawRectangle(0, screenHeight/2, screenWidth, screenHeight/2, map->groundColor);
}