#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "player.h"
#include "map.h"
#include "hashmap.h"
#include "list.h"
#include <string.h>
#include "mapparser.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

// djb2 hash
unsigned int djb2hash(void* key) {
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

void print(void* a, void* b) {
    printf("%s: %s", (char*) a, (char*) b);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        printf("Testing mode :]\n");
        SearchAndSetResourceDir("resources");

        MapParser parser = MapParserCreate("parsertest.map");
    
        ParserResult res = MapParserParse(parser);

        ParserTable mapSettings = ParserResultGetTable(res, "MapSettings");
        ParserTable tileDefinition = ParserResultGetTable(res, "TileDefinition");
        ParserTable tilePlacing = ParserResultGetTable(res, "TilePlacing");


        printf("%s, %d, %lf, %d\n",
            (char*)ParserElementGetValue(ParserTableGetElement(mapSettings, "string1")),
            *(bool*)ParserElementGetValue(ParserTableGetElement(tileDefinition, "falso")),
            *(double*)ParserElementGetValue(ParserTableGetElement(tileDefinition, "val2")),
            *(int*)ParserElementGetValue(ParserTableGetElement(tilePlacing, "val"))
        );

        MapParserDestroy(&parser);
        ParserResultDestroy(&res);

        return EXIT_SUCCESS;
    }

    // Tell the window to use vsync and work on high DPI displays
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
    
    // Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
    SearchAndSetResourceDir("resources");

    int window_size_x = 1280;
    int window_size_y = 720;

    bool drawing3D = false;
    
    // Create the window and OpenGL context
    InitWindow(window_size_x, window_size_y, "Raycaster");
    SetTargetFPS(60);
    
    // MAP VARS
    Map map = MapCreateFromFile("maptest.map");
    
    // PLAYER VARS
    Player player = PlayerCreate(10, 10, 45, 1280, map);

    // game loop
    while (!WindowShouldClose()) {		// run the loop untill the user presses ESCAPE or presses the Close button on the window

        // Player control
        PlayerInput(player);
        if (IsKeyPressed(KEY_G)) {
            drawing3D = !drawing3D;
        }


        // Then draw the texture on screen.
        BeginDrawing();
            // Setup the back buffer for drawing (clear color and depth buffers)
            ClearBackground(BLACK);

            if (drawing3D) {
                MapDraw3D(map, window_size_x, window_size_y);

                PlayerDraw3D(player, window_size_x, window_size_y);
            } else {
                MapDraw2D(map);
                
                PlayerDraw2D(player);
            }

            DrawFPS(0, 0);
        // End the frame and get ready for the next one  (display frame, poll input, etc...)
        EndDrawing();
    }

    // Cleanup
    // Unload the render texture and objects.
    MapDestroy(&map);
    PlayerDestroy(&player);

    // Destroy the window and cleanup the OpenGL context
    CloseWindow();
    return EXIT_SUCCESS;
}
