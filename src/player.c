#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "player.h"
#include "mapray.h"
#include "raylib.h"
#include "raymath.h"

struct player {
    double posX;
    double posY;
    int size;
    int speed;
    double rotation;                // Radians
    double rotationSpeed;           // Radians
    int FOV;                        // Degrees
    int numRays;
    MapRay* rays;
    Map map;                        // NULL if player is not in any map
};

// Internal: check if position is colliding with map
static bool isColliding(int posX, int posY, Map map) {
    // No collision if there's no map.
    if (map == NULL) {
        return false;
    }

    // Get grid position
    int gridPosX = (int) (posX) / MapGetTileSize(map);
    int gridPosY = (int) (posY) / MapGetTileSize(map);

    return MapGetTile(map, gridPosX, gridPosY) != GROUND;
}

Player PlayerCreate(int playerX, int playerY, int playerRotationDeg, int numRays, Map map) {
    Player pl = malloc(sizeof(struct player));
    assert(pl != NULL);

    pl->posX = playerX;
    pl->posY = playerY;
    pl->size = 10;
    pl->speed = 10;
    pl->rotation = playerRotationDeg*DEG2RAD;
    pl->rotationSpeed = 2*DEG2RAD;
    pl->FOV = 60;
    pl->numRays = numRays;
    pl->map = map;

    pl->rays = malloc(sizeof(MapRay)*numRays);
    assert(pl->rays != NULL);

    // Initialize rays.
    if (pl->numRays == 1) {
        pl->rays[0] = MapRayCreate(pl->posX, pl->posY, pl->rotation, 0, pl->map);
    } else {   
        double angle_offset = -pl->FOV/2;
        for (int i = 0; i < pl->numRays; i++) {
            pl->rays[i] = MapRayCreate(pl->posX, pl->posY, pl->rotation, angle_offset*DEG2RAD, pl->map);
            angle_offset += (double) pl->FOV / (double) (pl->numRays - 1);
        }
    }

    return pl;
}

void PlayerDestroy(Player* pp) {
    assert(pp != NULL);
    assert(*pp != NULL);

    Player p = *pp;

    // Destroy rays.
    for (int i = 0; i < p->numRays; i++) {
        MapRayDestroy(&p->rays[i]);
    }
    free(p->rays);
    free(p);
    *pp = NULL;
}

void PlayerSetMap(Player p, Map map) {
    assert(p != NULL);

    p->map = map;

    for (int i = 0; i < p->numRays; i++) {
        MapRaySetMap(p->rays[i], map);
    }
}


int PlayerGetX(Player p) {
    assert(p != NULL);
    
    return p->posX;
}

int PlayerGetY(Player p) {
    assert(p != NULL);
    
    return p->posY;
}

int PlayerGetRotationDeg(Player p) {
    assert(p != NULL);
    
    return (int) (p->rotation*RAD2DEG);
}

double PlayerGetRotationRad(Player p) {
    assert(p != NULL);
    
    return p->rotation;
}

bool PlayerIsColliding(Player p) {
    assert(p != NULL);

    // No collision if there's no map.
    if (p->map == NULL) {
        return false;
    }

    bool colliding = isColliding(p->posX, p->posY, p->map);

    // Try collision at 8 points around player.
    for (int angle = 0; angle < 360; angle+=45) {
        colliding = colliding || isColliding(p->posX + 10*cos(p->rotation + angle*DEG2RAD), p->posY + 10*sin(p->rotation + angle*DEG2RAD), p->map);;
    }
    return colliding;
}


void PlayerDraw2D(Player p) {
    assert(p != NULL);

    DrawCircle(p->posX, p->posY, p->size, (Color) {255, 0, 0, 255});
    DrawLine(p->posX, p->posY, p->posX + (20*cos(p->rotation)), p->posY + (20*sin(p->rotation)), (Color) {0, 0, 255, 255});

    // Draw MapRays
    for (int i = 0; i < p->numRays; i++) {
        MapRayDraw2D(p->rays[i]);
    }
}

void PlayerDraw3D(Player p, int screenWidth, int screenHeight) {
    assert(p != NULL);

    int line_width = screenWidth / p->numRays;

    for (int i = 0; i < p->numRays; i++) {
        MapRay ray = p->rays[i];
        if (!MapRayIsColliding(ray)) {
            continue;
        }
        
        int rayX = (line_width/2)+i*line_width;

        List collisions = MapRayGetCollisions(ray);
        ListMoveToStart(collisions);
        while (ListCanOperate(collisions)) {
        
            rayCollision currentCollision = *((rayCollision*) (ListGetCurrent(collisions)));

            Vector2 collisionPoint = (Vector2) {currentCollision.collisionX, currentCollision.collisionY};
            
            double distaux = sqrt(pow(p->posX-collisionPoint.x, 2) + pow(p->posY-collisionPoint.y, 2));
            double distance = (1.5*MapGetTileSize(p->map)*screenHeight) / (distaux*cos(MapRayGetAngleOffsetRad(ray)));
    
            Color drawColor;
            if (currentCollision.hitSide == X_AXIS) {
                drawColor = (Color) {255, 255, 255, 255};
            } else {
                drawColor = (Color) {210, 210, 210, 255};
            }
            //DrawRectangle(rayX, (screenHeight/2)-(distance/2), line_width, distance, drawColor);

            Vector2 collisionPointGrid = (Vector2) {currentCollision.collisionGridX, currentCollision.collisionGridY};

            Texture tex = TileGetTexture(currentCollision.tile);

            int ray_percentage;  // Percentage of tile that ray hit (not really percentage, just number of tile pixels)
            if (currentCollision.hitSide == X_AXIS) {
                ray_percentage = (int) (collisionPoint.y) % MapGetTileSize(p->map) + 1;
            } else {
                ray_percentage = (int) (collisionPoint.x) % MapGetTileSize(p->map) + 1; 
            }
            double texture_offset = ((double) (ray_percentage) / (double) (MapGetTileSize(p->map)))*((double) tex.width);
            int texture_width = 1;

            DrawTexturePro(tex,
                (Rectangle) {texture_offset-1, 0, texture_width, tex.height},
                (Rectangle) {rayX, (screenHeight/2)-(distance/2), line_width, distance},
                (Vector2) {0, 0}, 0, drawColor);
            
            ListMoveToNext(collisions);
        }
    }
}

void PlayerInput(Player p) {
    assert(p != NULL);

    double movement_angle = p->rotation;
    bool moving = false;

    if (IsKeyDown(KEY_W)) {
        moving = true;
        movement_angle = p->rotation + 0;
    }
    if (IsKeyDown(KEY_S)) {
        moving = true;
        movement_angle = p->rotation + 180*DEG2RAD;
    }
    if (IsKeyDown(KEY_A)) {
        moving = true;
        movement_angle = p->rotation + 270*DEG2RAD;
    }
    if (IsKeyDown(KEY_D)) {
        moving = true;
        movement_angle = p->rotation + 90*DEG2RAD;
    }

    double move_amount_x = p->speed*cos(movement_angle);
    double move_amount_y = p->speed*sin(movement_angle);

    // Movement with collision
    if (moving) {
        p->posX += move_amount_x;
        if (PlayerIsColliding(p)) {
            p->posX -= move_amount_x;
            while (!PlayerIsColliding(p)) {
                p->posX += move_amount_x / fabs(move_amount_x);
            }
            p->posX -= move_amount_x / fabs(move_amount_x);
        }
        
        p->posY += move_amount_y;
        if (PlayerIsColliding(p)) {
            p->posY -= move_amount_y;
            while (!PlayerIsColliding(p)) {
                p->posY += move_amount_y / fabs(move_amount_y);
            }
            p->posY -= move_amount_y / fabs(move_amount_y);
        }
    }

    if (IsKeyDown(KEY_LEFT)) {
        p->rotation -= p->rotationSpeed;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        p->rotation += p->rotationSpeed;
    }

    // Update MapRays
    for (int i = 0; i < p->numRays; i++) {
        MapRaySetAngle(p->rays[i], p->rotation);
        MapRaySetPosition(p->rays[i], p->posX, p->posY);
        MapRayCast(p->rays[i]);
    }
}
