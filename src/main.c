#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "player.h"
#include "map.h"
#include "hashmap.h"
#include "list.h"

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

void print(void* key, void* value) {
    printf("%s: %s", (char*) key, (char*) value);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        printf("Testing mode :]\n");
        
        HashMap hashmap = HashMapCreate(5, djb2hash);

        HashMapPut(hashmap, "bruh", "dnosada");
        HashMapPut(hashmap, "brh", "doisadnoas");
        HashMapPut(hashmap, "heehe", "kapmdfsad");
        HashMapPut(hashmap, "sigma", "dsapomdwapod,");
        HashMapPut(hashmap, "tralale", "oe2k 'e210je0qwej");

        HashMapPrint(hashmap, print);
        
        // TODO test HashMapGet

        HashMapDestroy(&hashmap);
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
    InitWindow(window_size_x, window_size_y, "Hello Raylib");
    SetTargetFPS(60);
    
    // MAP VARS
    Map map = MapCreateFromFile("maptest.map");
    
    // PLAYER VARS
    Player player = PlayerCreate(10, 10, 0, 1280, map);

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
