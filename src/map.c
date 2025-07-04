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
#include "mapparser.h"

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

    int tileID = 1;     // ID of next tile to be defined

    char line[500];
    int lineNumber = 0; // Used for error printing
    int currentPhase = 0; // 0 - map general settings; 1 - reading tile data; 2 - tile placing
    
    map->tileNames = ListCreate(NULL);
    char* ground = calloc(7, sizeof(char)); assert(ground != NULL); ground = strncpy(ground, "GROUND", 6); ListAppendLast(map->tileNames, ground);

    // Default tileMap values
    map->tileMap = HashMapCreate(5, djb2hash, hashmapstrcmp);
    HashMapPut(map->tileMap, ground, TileCreate(ground, TILE_GROUND, "test.png", false));

    MapParser parser = MapParserCreate(filename);
    ParserResult res = MapParserParse(parser);
    
    ParserTable mapSettings = ParserResultGetTable(res, "MapSettings");
    ParserTable tileDefinition = ParserResultGetTable(res, "TileDefinition");
    ParserTable tilePlacing = ParserResultGetTable(res, "TilePlacing");
    if (mapSettings == NULL) {
        fprintf(stderr, "Error opening \"%s\": No table named \"MapSettings\".\n", filename);
        exit(EXIT_FAILURE);
    }
    if (tileDefinition == NULL) {
        fprintf(stderr, "Error opening \"%s\": No table named \"TileDefinition\".\n", filename);
        exit(EXIT_FAILURE);
    }
    if (tilePlacing == NULL) {
        fprintf(stderr, "Error opening \"%s\": No table named \"TilePlacing\".\n", filename);
        exit(EXIT_FAILURE);
    }

    // Map dimensions
    ParserElement e = ParserTableGetElement(mapSettings, "mapSize");
    if (e == NULL || ParserElementGetType(e) != LIST_TYPE) {
        fprintf(stderr, "Error opening \"%s\": MapSettings must have a \"mapSize\" list parameter.\n", filename);
        exit(EXIT_FAILURE);
    }
    List val = ParserElementGetValue(e);
    if (ListGetSize(val) != 2 || ParserElementGetType(ListGet(val, 0)) != INT_TYPE || ParserElementGetType(ListGet(val, 1)) != INT_TYPE) {
        fprintf(stderr, "Error opening \"%s\": mapSize must be [sizeX, sizeY] (both positive integers).\n", filename);
        exit(EXIT_FAILURE);
    }

    map->numRows = *(int*) ParserElementGetValue(ListGet(val, 0));
    map->numCols = *(int*) ParserElementGetValue(ListGet(val, 1));
    if (map->numRows <= 0 || map->numCols <= 0) {
        fprintf(stderr, "Error opening \"%s\": mapSize must be [sizeX, sizeY] (both positive integers).\n", filename);
        exit(EXIT_FAILURE);
    }

    // Initialize grid
    map->grid = malloc(sizeof(int*) * map->numRows);
    assert(map->grid != NULL);
    for (int i = 0; i < map->numRows; i++) {
        map->grid[i] = calloc(map->numCols, sizeof(int));
        assert(map->grid[i] != NULL);
    }

    // Tile dimensions
    e = ParserTableGetElement(mapSettings, "tileSize");
    if (e == NULL || ParserElementGetType(e) != INT_TYPE) {
        fprintf(stderr, "Error opening \"%s\": MapSettings must have a \"tileSize\" integer parameter.\n", filename);
        exit(EXIT_FAILURE);
    }

    map->tileSize = *(int*) ParserElementGetValue(e);
    if (map->tileSize <= 0) {
        fprintf(stderr, "Error opening \"%s\": MapSettings must have a \"tileSize\" positive integer parameter.\n", filename);
        exit(EXIT_FAILURE);
    }
    
    // Ceiling color
    // TODO: Move color parsing to static function (also make alpha be optional)
    e = ParserTableGetElement(mapSettings, "ceilingColor");
    if (e == NULL) { // Give default value
        map->ceilingColor = (Color) {0, 0, 0, 255};
    } else if (ParserElementGetType(e) != LIST_TYPE) {  // Wrong because it was defined but with wrong type
        fprintf(stderr, "Error opening \"%s\": ceilingColor must be an array of RGB(A) values (0-255 integers).\n", filename);
        exit(EXIT_FAILURE);
    }
    val = (List) ParserElementGetValue(e);
    if (ListGetSize(val) != 3 && ListGetSize(val) != 4) {   // Wrong because of color definition
        fprintf(stderr, "Error opening \"%s\": ceilingColor must be an array of RGB(A) values (0-255 integers).\n", filename);
        exit(EXIT_FAILURE);
    }

    // Type checking
    ListMoveToStart(val);
    while (ListCanOperate(val)) {
        ParserElement elem = ListGetCurrent(val);
        if (ParserElementGetType(elem) != INT_TYPE || (*(int*) ParserElementGetValue(elem)) < 0 || (*(int*) ParserElementGetValue(elem)) > 255) {
            fprintf(stderr, "Error opening \"%s\": ceilingColor must be an array of RGB(A) values (0-255 integers).\n", filename);
            exit(EXIT_FAILURE);
        }
        ListMoveToNext(val);
    }
    map->ceilingColor = (Color) {*(int*) ParserElementGetValue(ListGet(val, 0)), *(int*) ParserElementGetValue(ListGet(val, 1)), *(int*) ParserElementGetValue(ListGet(val, 2)), *(int*) ParserElementGetValue(ListGet(val, 3))};

    // Ground color
    e = ParserTableGetElement(mapSettings, "groundColor");
    if (e == NULL) { // Give default value
        map->groundColor = (Color) {0, 0, 0, 255};
    } else if (ParserElementGetType(e) != LIST_TYPE) {  // Wrong because it was defined but with wrong type
        fprintf(stderr, "Error opening \"%s\": groundColor must be an array of RGB(A) values (0-255 integers).\n", filename);
        exit(EXIT_FAILURE);
    }
    val = (List) ParserElementGetValue(e);
    if (ListGetSize(val) != 3 && ListGetSize(val) != 4) {   // Wrong because of color definition
        fprintf(stderr, "Error opening \"%s\": groundColor must be an array of RGB(A) values (0-255 integers).\n", filename);
        exit(EXIT_FAILURE);
    }

    // Type checking
    ListMoveToStart(val);
    while (ListCanOperate(val)) {
        ParserElement elem = ListGetCurrent(val);
        if (ParserElementGetType(elem) != INT_TYPE || (*(int*) ParserElementGetValue(elem)) < 0 || (*(int*) ParserElementGetValue(elem)) > 255) {
            fprintf(stderr, "Error opening \"%s\": groundColor must be an array of RGB(A) values (0-255 integers).\n", filename);
            exit(EXIT_FAILURE);
        }
        ListMoveToNext(val);
    }
    map->groundColor = (Color) {*(int*) ParserElementGetValue(ListGet(val, 0)), *(int*) ParserElementGetValue(ListGet(val, 1)), *(int*) ParserElementGetValue(ListGet(val, 2)), *(int*) ParserElementGetValue(ListGet(val, 3))};


    // Tile definitions
    HashMap tiledefs = ParserTableGetHashMap(tileDefinition);
    HashMapIterator iter = HashMapGetIterator(tiledefs);
    while (HashMapIterCanOperate(iter)) {
        ParserElement tile = (ParserElement) HashMapIterGetCurrentValue(iter);
        char* n = ParserElementGetKey(tile);
        
        char* tilename = calloc(strlen(n)+1, sizeof(char));
        assert(tilename != NULL);
        strncpy(tilename, n, strlen(n));
        
        if (ParserElementGetType(tile) != TABLE_TYPE) {
            fprintf(stderr, "Error opening \"%s\": In TileDefinition, only tiles can be defined ({surface: SURFACE, <options>...}).\n", filename);
            exit(EXIT_FAILURE);
        }

        HashMap tilestuff = (HashMap) ParserElementGetValue(tile);
        if (!HashMapContains(tilestuff, "surface")) {
            fprintf(stderr, "Error opening \"%s\": Tile \"%s\" has no attribute \"surface\".\n", filename, n);
            exit(EXIT_FAILURE);
        }
        if (ParserElementGetType(HashMapGet(tilestuff, "surface")) != STRING_TYPE) {    // Not the name of a file
            fprintf(stderr, "Error opening \"%s\": Tile surfaces must be strings! (in tile \"%s\")\n", filename, n);
            exit(EXIT_FAILURE);
        }

        char* tileSurfaceName = (char*) ParserElementGetValue(HashMapGet(tilestuff, "surface"));

        // Handle transparency
        bool isTransparent = false;
        if (HashMapContains(tilestuff, "transparent")) {
            ParserElement transparencyelem = HashMapGet(tilestuff, "transparent");
            if (ParserElementGetType(transparencyelem) != BOOL_TYPE) {
                fprintf(stderr, "Error opening \"%s\": Tile transparency must be represented by a bool value! (in tile \"%s\")\n", filename, n);
                exit(EXIT_FAILURE);
            }
            isTransparent = *(bool*) ParserElementGetValue(transparencyelem);
        }


        registerTileData(map, tilename, tileSurfaceName, isTransparent, &tileID);

        HashMapIterGoToNext(iter);
    }
    
    HashMapIterDestroy(&iter);

    
    // Tile placements
    ParserElement tiles = ParserTableGetElement(tilePlacing, "Tiles");
    if (tiles != NULL) {
        if (ParserElementGetType(tiles) != LIST_TYPE) {
            fprintf(stderr, "Error opening \"%s\": \"Tiles\" parameter must be a list of [int tileX, int tileY, string tileName].\n", filename);
            exit(EXIT_FAILURE);
        }

        List tileList = (List) ParserElementGetValue(tiles);
        ListMoveToStart(tileList);
        while (ListCanOperate(tileList)) {
            if (ParserElementGetType(ListGetCurrent(tileList)) != LIST_TYPE) {
                fprintf(stderr, "Error opening \"%s\": \"Tiles\" parameter must be a list of [int tileX, int tileY, string tileName].\n", filename);
                exit(EXIT_FAILURE);
            }
            
            // Verify tile placement list semantics
            List tilePlacement = (List) ParserElementGetValue(ListGetCurrent(tileList));
            if (ListGetSize(tilePlacement) != 3 || ParserElementGetType(ListGet(tilePlacement, 0)) != INT_TYPE || ParserElementGetType(ListGet(tilePlacement, 1)) != INT_TYPE || ParserElementGetType(ListGet(tilePlacement, 2)) != STRING_TYPE) {
                fprintf(stderr, "Error opening \"%s\": \"Tiles\" parameter must be a list of [int tileX, int tileY, string tileName].\n", filename);
                exit(EXIT_FAILURE);
            }

            Tile tile = (Tile) HashMapGet(map->tileMap, (char*) ParserElementGetValue(ListGet(tilePlacement, 2)));
            MapSetTile(map,
                *((int*) ParserElementGetValue(ListGet(tilePlacement, 0))),
                *((int*) ParserElementGetValue(ListGet(tilePlacement, 1))),
                TileGetMapTiles(tile));

            ListMoveToNext(tileList);
        }
    } else {
        fprintf(stderr, "Warning opening \"%s\": No parameter \"Tiles\" was given in table \"TilePlacing\", so the map will be blank.\n", filename);
    }


    // Cleanup
    ParserResultDestroy(&res);
    MapParserDestroy(&parser);

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
        char* str = (char*) HashMapIterGetCurrentKey(iter);
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