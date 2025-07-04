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

float min(float v1, float v2) {
    return v1 < v2 ? v1 : v2;
}

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

void parserElemPrint(void* a);
void parserElemPrintTable(void* k, void* v);

void parserElemPrint(void* a) {
    ParserElement p = (ParserElement) a;
    switch (ParserElementGetType(p))
    {
    case BOOL_TYPE:
        bool val = *(bool*)ParserElementGetValue(p);
        printf("%s", val ? "true" : "false");
        break;
    
    case STRING_TYPE:
        printf("\"%s\"", (char *)ParserElementGetValue(p));
        break;
    case INT_TYPE:
        printf("%d", *(int*)ParserElementGetValue(p));
        break;
    case FLOAT_TYPE:
        printf("%lf", *(double*)ParserElementGetValue(p));
        break;
    case LIST_TYPE:
        List lst = (List) ParserElementGetValue(p);
        ListPrint(lst, false, parserElemPrint);
        break;
    case TABLE_TYPE:
        HashMap map = (HashMap) ParserElementGetValue(p);
        HashMapPrint(map, false, parserElemPrintTable);
        break;
    default:
        printf("IDK LMAO");
        break;
    }
}

void parserElemPrintTable(void* k, void* v) {
    char* key = (char*) k;
    ParserElement p = (ParserElement) v;
    printf("%s: ", key);
    switch (ParserElementGetType(p))
    {
    case BOOL_TYPE:
        bool val = *(bool*)ParserElementGetValue(p);
        printf("%s", val ? "true" : "false");
        break;
    
    case STRING_TYPE:
        printf("\"%s\"", (char *)ParserElementGetValue(p));
        break;
    case INT_TYPE:
        printf("%d", *(int*)ParserElementGetValue(p));
        break;
    case FLOAT_TYPE:
        printf("%lf", *(double*)ParserElementGetValue(p));
        break;
    case LIST_TYPE:
        List lst = (List) ParserElementGetValue(p);
        ListPrint(lst, false, parserElemPrint);
        break;
    case TABLE_TYPE:
        HashMap map = (HashMap) ParserElementGetValue(p);
        HashMapPrint(map, false, parserElemPrintTable);
        break;
    default:
        printf("IDK LMAO");
        break;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        printf("Testing mode :]\n");

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

    RenderTexture2D render_texture = LoadRenderTexture(window_size_x, window_size_y);
    
    // MAP VARS
    Map map = MapCreateFromFile("maptest.map");
    
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
