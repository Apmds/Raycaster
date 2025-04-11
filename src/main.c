#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"
#include "player.h"
#include "map.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

int main() {
    // Tell the window to use vsync and work on high DPI displays
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

    int window_size_x = 1280;
    int window_size_y = 720;

    bool drawing3D = false;
    
    // MAP VARS
    Map map = MapCreate(20, 20);
    MapSetTile(map, 0, 5, WALL);
    MapSetTile(map, 0, 6, WALL);
    MapSetTile(map, 0, 7, WALL);
    MapSetTile(map, 0, 8, WALL);
    MapSetTile(map, 1, 8, WALL);
    MapSetTile(map, 2, 8, WALL);
    MapSetTile(map, 3, 8, WALL);
    MapSetTile(map, 4, 8, WALL);
    MapSetTile(map, 5, 8, WALL);
    MapSetTile(map, 6, 8, WALL);

    // PLAYER VARS
    Player player = PlayerCreate(10, 10, 0, 1280, map);

    // Create the window and OpenGL context
    InitWindow(window_size_x, window_size_y, "Hello Raylib");
    SetTargetFPS(60);

    // Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
    SearchAndSetResourceDir("resources");

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
