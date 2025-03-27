#include <stdlib.h>

#include "raylib.h"
#include "raymath.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

int main() {
    // Tell the window to use vsync and work on high DPI displays
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

    int window_size_x = 1280;
    int window_size_y = 720;

    bool drawing3D = false;

    // PLAYER VARS
    int playerX = 0;
    int playerY = 0;
    int playerSize = 10;
    int playerSpeed = 2;
    int playerRotation = 0;

    // MAP VARS
    int** grid = calloc(20, sizeof(int*));
    for (int i = 0; i < 20; i++) {
        grid[i] = calloc(20, sizeof(int));
    }
    grid[0][5] = 1;
    grid[0][6] = 1;
    grid[0][7] = 1;
    grid[0][8] = 1;
    grid[1][8] = 1;
    grid[2][8] = 1;
    grid[3][8] = 1;
    grid[4][8] = 1;
    grid[5][8] = 1;
    grid[6][8] = 1;

    // Create the window and OpenGL context
    InitWindow(window_size_x, window_size_y, "Hello Raylib");
    SetTargetFPS(60);

    // Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
    SearchAndSetResourceDir("resources");

    // game loop
    while (!WindowShouldClose()) {		// run the loop untill the user presses ESCAPE or presses the Close button on the window

        // Player control
        if (IsKeyDown(KEY_W)) {
            playerY -= playerSpeed;
        }
        if (IsKeyDown(KEY_S)) {
            playerY += playerSpeed;
        }
        if (IsKeyDown(KEY_A)) {
            playerX -= playerSpeed;
        }
        if (IsKeyDown(KEY_D)) {
            playerX += playerSpeed;
        }
        if (IsKeyDown(KEY_LEFT)) {
            playerRotation -= playerSpeed;
        }
        if (IsKeyDown(KEY_RIGHT)) {
            playerRotation += playerSpeed;
        }
        if (IsKeyPressed(KEY_G)) {
            drawing3D = !drawing3D;
        }

        // Then draw the texture on screen.
        BeginDrawing();
            // Setup the back buffer for drawing (clear color and depth buffers)
            ClearBackground(BLACK);

            if (drawing3D) {

            } else {
                for (int i = 0; i < 20; i++) {
                    for (int j = 0; j < 20; j++) {
                        Color color;
                        if (grid[i][j] == 1) {
                            color = (Color) {255, 255, 255, 255};
                        } else {
                            color = (Color) {0, 0, 0, 255};
                        }
                        DrawRectangle(i*50, j*50, 50, 50, color);
                    }
                }
    
                DrawCircle(playerX, playerY, playerSize, (Color) {255, 0, 0, 255});
                DrawLine(playerX, playerY, playerX + (20*cos(DEG2RAD*playerRotation)), playerY + (20*sin(DEG2RAD*playerRotation)), (Color) {0, 0, 255, 255});
            }


        // End the frame and get ready for the next one  (display frame, poll input, etc...)
        EndDrawing();
    }

    // Cleanup
    // Unload the render texture and objects.
    for (int i = 0; i < 20; i++) {
        free(grid[i]);
    }    
    free(grid);

    // Destroy the window and cleanup the OpenGL context
    CloseWindow();
    return EXIT_SUCCESS;
}
