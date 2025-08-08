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

#define USAGE_MESSAGE "Usage: raycaster [-h] mapname\n"
#define DESCRIPTION_MESSAGE "Runs the raycaster, loading the specified map file.\n"

float min(float v1, float v2) {
    return v1 < v2 ? v1 : v2;
}

int main(int argc, char* argv[]) {
    // Argument handling
    if (argc <= 1) {
        fprintf(stderr, USAGE_MESSAGE);
        fprintf(stderr, "Must specify a map file to load!\n");

        return EXIT_FAILURE;
    }
    
    char* map_name;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            printf(USAGE_MESSAGE);
            printf(DESCRIPTION_MESSAGE);
            return EXIT_SUCCESS;
        } else {
            map_name = argv[i];
        }
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

    RenderTexture2D render_texture = LoadRenderTexture(window_size_x, window_size_y);
    
    // MAP VARS
    Map map = MapCreateFromFile(map_name);
    
    // PLAYER VARS
    Player player = PlayerCreate(10, 10, 45, 1280, map);
    
    // game loop
    while (!WindowShouldClose()) {		// run the loop until the user presses ESCAPE or presses the Close button on the window

        // How much the screen is scaled from the starting size
        float scale = min((float)GetScreenWidth()/window_size_x, (float)GetScreenHeight()/window_size_y);

        // Player control
        PlayerInput(player);
        if (IsKeyPressed(KEY_G)) {
            drawing3D = !drawing3D;
        }
        if (IsKeyPressed(KEY_R)) { // Reload map
            MapDestroy(&map);
            map = MapCreateFromFile("wolf/wolfe1m1.map");
            PlayerSetMap(player, map);
        }

        BeginTextureMode(render_texture);
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

        EndTextureMode();

        // Then draw the texture on screen.
        BeginDrawing();
            // Setup the back buffer for drawing (clear color and depth buffers)
            ClearBackground(BLACK);

             DrawTexturePro(
                render_texture.texture, 
                (Rectangle) { 0.0f, 0.0f, (float) render_texture.texture.width, (float) -render_texture.texture.height },       // Draw flipped because of OpenGL shenanigans
                (Rectangle) { (GetScreenWidth() - ((float)window_size_x*scale))*0.5f, (GetScreenHeight() - ((float)window_size_y*scale))*0.5f, // Draw at the center of the screen
                (float)window_size_x*scale, (float)window_size_y*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);


        // End the frame and get ready for the next one  (display frame, poll input, etc...)
        EndDrawing();
    }

    // Cleanup
    // Unload the render texture and objects.
    MapDestroy(&map);
    PlayerDestroy(&player);
    UnloadRenderTexture(render_texture);

    // Destroy the window and cleanup the OpenGL context
    CloseWindow();
    return EXIT_SUCCESS;
}
