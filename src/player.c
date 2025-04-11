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

    free(p);

    // Destroy rays.
    for (int i = 0; i < p->numRays; i++) {
        MapRayDestroy(&p->rays[i]);
    }

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

    // Try collision around player.
    bool colliding_front = isColliding(p->posX + 10*cos(p->rotation), p->posY + 10*sin(p->rotation), p->map);
    bool colliding_back = isColliding(p->posX + 10*cos(p->rotation + 180*DEG2RAD), p->posY + 10*sin(p->rotation + 180*DEG2RAD), p->map);
    bool colliding_left = isColliding(p->posX + 10*cos(p->rotation + 270*DEG2RAD), p->posY + 10*sin(p->rotation + 270*DEG2RAD), p->map);
    bool colliding_right = isColliding(p->posX + 10*cos(p->rotation + 90*DEG2RAD), p->posY + 10*sin(p->rotation + 90*DEG2RAD), p->map);

    bool colliding_front_left = isColliding(p->posX + 10*cos(p->rotation + 315*DEG2RAD), p->posY + 10*sin(p->rotation + 315*DEG2RAD), p->map);
    bool colliding_front_right = isColliding(p->posX + 10*cos(p->rotation + 45*DEG2RAD), p->posY + 10*sin(p->rotation + 45*DEG2RAD), p->map);
    
    bool colliding_back_left = isColliding(p->posX + 10*cos(p->rotation + 225*DEG2RAD), p->posY + 10*sin(p->rotation + 225*DEG2RAD), p->map);
    bool colliding_back_right = isColliding(p->posX + 10*cos(p->rotation + 135*DEG2RAD), p->posY + 10*sin(p->rotation + 135*DEG2RAD), p->map);
    
    return colliding || colliding_front || colliding_back || colliding_left || colliding_right || colliding_front_left || colliding_front_right || colliding_back_left || colliding_back_right;
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

        Vector2 collisionPoint = MapRayGetCollisionPoint(ray);
        double distance = (double) MapRayGetMaxLength(ray) / Vector2Distance(collisionPoint, (Vector2) {p->posX, p->posY});
        distance *= cos(-((p->FOV/2))*DEG2RAD);
        distance *= 50;

        Color drawColor;
        if (MapRayGetHitSide(ray) == X_AXIS) {
            drawColor = (Color) {255, 255, 255, 255};
        } else {
            drawColor = (Color) {210, 210, 210, 255};
        }
        DrawRectangle(rayX, (screenHeight/2)-(distance/2), line_width, distance, drawColor);
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
        }
        
        p->posY += move_amount_y;
        if (PlayerIsColliding(p)) {
            p->posY -= move_amount_y;
        }
    }

    if (IsKeyDown(KEY_LEFT)) {
        p->rotation -= p->rotationSpeed;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        p->rotation += p->rotationSpeed;
    }

    //printf("%f (%f, %f)\n", p->rotation, cos(p->rotation), sin(p->rotation));
    // Update MapRays
    for (int i = 0; i < p->numRays; i++) {
        MapRaySetAngle(p->rays[i], p->rotation);
        MapRaySetPosition(p->rays[i], p->posX, p->posY);
        MapRayCast(p->rays[i]);
    }
}