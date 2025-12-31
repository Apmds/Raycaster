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
#include "billboard.h"
#include <errno.h>

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

struct map {
    int numRows;
    int numCols;
    int tileSize;                       // Size of each tile (pixels)
    HashMap tileMap;                    // HashMap that contains the details (texture) for a tile, given its name
    List tileNames;                   // Array associating tile indices (MapTiles) to the tile names (char*)             TODO: change to ArrayList
    HashMap billboardMap;           // HashMap that contains the details (texture) for a billboard, given its name
    List billboards;                // List of all billboards (enemies, etc.)
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
static bool registerTile(Map map, Tile tile, int* tileID) {
    char* tileName = (char*) TileGetName(tile);
    if (HashMapContains(map->tileMap, tileName)) { // Duplicate checking
        return false;
    }
    
    // Add information in map
    HashMapPut(map->tileMap, tileName, tile);
    ListAppendLast(map->tileNames, tileName);

    // Advances to the next tile
    (*tileID)++;

    return true;
}

static Color parseColor(ParserElement element, const char* filename) {
    if (element == NULL) { // Give default value
        errno = -1;
        return (Color) {0, 0, 0, 255};
    } else if (ParserElementGetType(element) != LIST_TYPE) {  // Wrong because it was defined but with wrong type
        fprintf(stderr, "Error opening \"%s\": %s must be an array of RGB(A) values (0-255 integers).\n", filename, ParserElementGetKey(element));
        errno = -1;
        return (Color) {0, 0, 0, 255};
    }
    
    List val = (List) ParserElementGetValue(element);
    if (ListGetSize(val) != 3 && ListGetSize(val) != 4) {   // Wrong because of color definition
        fprintf(stderr, "Error opening \"%s\": %s must be an array of RGB(A) values (0-255 integers).\n", filename, ParserElementGetKey(element));
        errno = -1;
        return (Color) {0, 0, 0, 255};
    }

    // Type checking
    ListMoveToStart(val);
    while (ListCanOperate(val)) {
        ParserElement elem = (ParserElement) ListGetCurrent(val);

        if (ParserElementGetType(elem) != INT_TYPE || (*(int*) ParserElementGetValue(elem)) < 0 || (*(int*) ParserElementGetValue(elem)) > 255) {
            fprintf(stderr, "Error opening \"%s\": %s must be an array of RGB(A) values (0-255 integers).\n", filename, ParserElementGetKey(element));
            errno = -1;
            return (Color) {0, 0, 0, 255};
        }
        ListMoveToNext(val);
    }
    
    int r = *(int*) ParserElementGetValue(ListGet(val, 0));
    int g = *(int*) ParserElementGetValue(ListGet(val, 1));
    int b = *(int*) ParserElementGetValue(ListGet(val, 2));
    int a = 255;
    if (ListGetSize(val) == 4) {
        a = *(int*) ParserElementGetValue(ListGet(val, 3));
    }
    return (Color) {r, g, b, a};
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
    HashMapPut(map->tileMap, ground, TileCreateTextured(ground, TILE_GROUND, "resources/default.png", false));

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
    
    map->tileNames = ListCreate(NULL);
    char* ground = calloc(7, sizeof(char)); assert(ground != NULL); ground = strncpy(ground, "GROUND", 6); ListAppendLast(map->tileNames, ground);

    // Default tileMap values
    map->tileMap = HashMapCreate(5, djb2hash, hashmapstrcmp);
    HashMapPut(map->tileMap, ground, TileCreateTextured(ground, TILE_GROUND, "resources/default.png", false));

    MapParser parser = MapParserCreate(filename);
    ParserResult res = MapParserParse(parser);
    
    ParserTable mapSettings = ParserResultGetTable(res, "MapSettings");
    ParserTable tileDefinition = ParserResultGetTable(res, "TileDefinition");
    ParserTable tilePlacing = ParserResultGetTable(res, "TilePlacing");
    ParserTable billboardDefinition = ParserResultGetTable(res, "BillboardDefinition");
    ParserTable billboardPlacing = ParserResultGetTable(res, "BillboardPlacing");
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
    if (billboardDefinition == NULL) {
        fprintf(stderr, "Error opening \"%s\": No table named \"BillboardDefinition\".\n", filename);
        exit(EXIT_FAILURE);
    }
    if (billboardPlacing == NULL) {
        fprintf(stderr, "Error opening \"%s\": No table named \"BillboardPlacing\".\n", filename);
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
    map->ceilingColor = parseColor(ParserTableGetElement(mapSettings, "ceilingColor"), filename);

    // Ground color
    map->groundColor = parseColor(ParserTableGetElement(mapSettings, "groundColor"), filename);
    
    // Change working resource directory to folder containing map file
    const char* last_workdir = GetWorkingDirectory();
    SearchAndSetResourceDir(GetDirectoryPath(filename));

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

        ParserElement tileSurface = (ParserElement) HashMapGet(tilestuff, "surface");
        Tile tileobj = NULL;
        if (ParserElementGetType(tileSurface) == STRING_TYPE) { // Is a file name
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

            tileobj = TileCreateTextured(tilename, tileID, tileSurfaceName, isTransparent);

        } else { // Might be a color
            errno = 0;
            Color color = parseColor(tileSurface, filename);

            if (errno != 0) {   // Not a color, so surface is wrong
                fprintf(stderr, "Error opening \"%s\": Tile surfaces must be either a string file name or a color! (in tile \"%s\")\n", filename, n);
                exit(EXIT_FAILURE);
            }

            tileobj = TileCreateColored(tilename, tileID, color);
        }

        

        registerTile(map, tileobj, &tileID);

        HashMapIterGoToNext(iter);
    }
    
    HashMapIterDestroy(&iter);

    // Change working resource directory back
    ChangeDirectory(last_workdir);
    
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

    // Change working resource directory to folder containing map file
    last_workdir = GetWorkingDirectory();
    SearchAndSetResourceDir(GetDirectoryPath(filename));

    // Billboard definitions
    map->billboardMap = HashMapCreate(5, djb2hash, hashmapstrcmp);
    HashMap billboard_defs = ParserTableGetHashMap(billboardDefinition);
    HashMapIterator billboard_defs_iter = HashMapGetIterator(billboard_defs);

    while (HashMapIterCanOperate(billboard_defs_iter)) {
        ParserElement def = HashMapIterGetCurrentValue(billboard_defs_iter);
        char* n = ParserElementGetKey(def);

        char* bbname = calloc(strlen(n)+1, sizeof(char));
        assert(bbname != NULL);
        strncpy(bbname, n, strlen(n));

        if (ParserElementGetType(def) != TABLE_TYPE) {
            fprintf(stderr, "Warning opening \"%s\": In BillboardDefinition, only billboards can be defined ({surface: SURFACE, <options>...}).\n", filename);
            exit(EXIT_FAILURE);
        }

        HashMap defMap = ParserElementGetValue(def);
        if (!HashMapContains(defMap, "surface")) {
            fprintf(stderr, "Error opening \"%s\": Tile \"%s\" has no attribute \"surface\".\n", filename, n);
            exit(EXIT_FAILURE);
        }

        ParserElement surfaceEl = (ParserElement) HashMapGet(defMap, "surface");
        if (ParserElementGetType(surfaceEl) != STRING_TYPE) { // TODO: this could also MAYBE be a color, since its possible when defining the same attribute in a tile
            fprintf(stderr, "Error opening \"%s\": Billboard surfaces must be a string file name! (in tile \"%s\")\n", filename, n);
            exit(EXIT_FAILURE);
        }

        // TODO: While I don't invent anything else, only a texture will need to be saved, later should probably be a "BillboardData" object or something
        char* fname = ParserElementGetValue(surfaceEl);
        Texture* texp = malloc(sizeof(Texture));
        assert(texp != NULL);

        *texp = LoadTexture(fname);

        // Duplicate checking
        if (!HashMapContains(map->billboardMap, bbname)) {
            HashMapPut(map->billboardMap, bbname, texp);
        } else {
            UnloadTexture(*texp);
            free(texp); texp = NULL;
        }

        HashMapIterGoToNext(billboard_defs_iter);
    }

    HashMapIterDestroy(&billboard_defs_iter);

    // Change working resource directory back
    ChangeDirectory(last_workdir);
    
    // Billboard placements
    map->billboards = ListCreate(NULL);
    if (!ParserTableHasElement(billboardPlacing, "Billboards")) {
        fprintf(stderr, "Warning opening \"%s\": No parameter \"Billboards\" was given in table \"BillboardPlacing\", so the map will be blank.\n", filename);
    }

    ParserElement billboardsEl = ParserTableGetElement(billboardPlacing, "Billboards");
    if (ParserElementGetType(billboardsEl) != LIST_TYPE) {
        fprintf(stderr, "Error opening \"%s\": \"Billboards\" parameter must be a list of [int bbX, int bbY, string bbName].\n", filename);
        exit(EXIT_FAILURE);
    }

    List billboards = ParserElementGetValue(billboardsEl);
    ListMoveToStart(billboards);
    while (ListCanOperate(billboards)) {
        ParserElement bbEl = ListGetCurrent(billboards);

        if (ParserElementGetType(bbEl) != LIST_TYPE) {
            fprintf(stderr, "Error opening \"%s\": \"Billboards\" parameter must be a list of [int bbX, int bbY, string bbName].\n", filename);
            exit(EXIT_FAILURE);
        }

        // Verify billboard placement list semantics
        List bbPlacement = (List) ParserElementGetValue(bbEl);

        if (ListGetSize(bbPlacement) != 3 || ParserElementGetType(ListGet(bbPlacement, 0)) != INT_TYPE || ParserElementGetType(ListGet(bbPlacement, 1)) != INT_TYPE || ParserElementGetType(ListGet(bbPlacement, 2)) != STRING_TYPE) {
            fprintf(stderr, "Error opening \"%s\": \"Billboards\" parameter must be a list of [int tileX, int tileY, string tileName].\n", filename);
            exit(EXIT_FAILURE);
        }

        Texture* texp = (Texture*) HashMapGet(map->billboardMap, (char*) ParserElementGetValue(ListGet(bbPlacement, 2)));

        // Create and store the billboard itself
        Billboard billboard = BillboardCreate(
            *texp,
            *((int*) ParserElementGetValue(ListGet(bbPlacement, 0))),
            *((int*) ParserElementGetValue(ListGet(bbPlacement, 1))),
            10
        );
        ListAppendLast(map->billboards, billboard);

        ListMoveToNext(billboards);
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

    // Clear (unload) billboard textures in billboardmap
    iter = HashMapGetIterator(map->billboardMap);
    while (HashMapIterCanOperate(iter)) {
        char* str = (char*) HashMapIterGetCurrentKey(iter);
        Texture* texp = HashMapGet(map->billboardMap, str);

        Texture tex = *texp;
        UnloadTexture(tex);
        
        free(texp);

        HashMapIterGoToNext(iter);
    }
    HashMapIterDestroy(&iter);
    HashMapDestroy(&(map->billboardMap));


    // Destroy billboards
    ListMoveToStart(map->billboards);
    while (ListCanOperate(map->billboards)) {
        Billboard billboard = ListPopFirst(map->billboards);
        BillboardDestroy(&(billboard));
    }
    
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

List MapGetBillboardsAt(Map map, int col, int row) {
    assert(map != NULL);

    const int check_margin = 1; // equates to a 3-by-3 square around the bb
    
    List ret = ListCreate(NULL);

    // TODO: this is inneficient, putting all billboards in sectors and only verifying those sectors should help with performance
    ListMoveToStart(map->billboards);
    while (ListCanOperate(map->billboards)) {
        Billboard bb = ListGetCurrent(map->billboards);

        int x = BillboardGetX(bb);
        int y = BillboardGetY(bb);

        int gridX = x / map->tileSize;
        int gridY = y / map->tileSize;

        
        for (int r = -check_margin + gridY; r <= check_margin + gridY; r++) {
            for (int c = -check_margin + gridX; c <= check_margin + gridX; c++) {
                //printf("(%d, %d) grid: (%d, %d) (%d, %d)\n", c, r, gridX, gridY, col, row);
                if (c == col && r == row) {
                    ListAppendLast(ret, bb);
                    break;
                }
            }
        }

        ListMoveToNext(map->billboards);
    }

    if (ListGetSize(ret) > 0) {
        printf("%d\n", ListGetSize(ret));
    }

    return ret;
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

    // Draw billboards
    ListMoveToStart(map->billboards);
    while (ListCanOperate(map->billboards)) {
        Billboard bb = ListGetCurrent(map->billboards);
        DrawCircle(BillboardGetX(bb), BillboardGetY(bb), BillboardGetSize(bb), (Color) {0, 0, 255, 255});

        ListMoveToNext(map->billboards);
    }
}

void MapDraw3D(Map map, int screenWidth, int screenHeight) {
    assert(map != NULL);

    DrawRectangle(0, 0, screenWidth, screenHeight/2, map->ceilingColor);
    DrawRectangle(0, screenHeight/2, screenWidth, screenHeight/2, map->groundColor);
}