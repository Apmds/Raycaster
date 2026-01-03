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
    double rotationSpeed;           // For rotating with the keys. In radians
    double sensitivity;             // For the mouse. In radians
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

    return MapGetTile(map, gridPosX, gridPosY) != TILE_GROUND;
}

Player PlayerCreate(int playerX, int playerY, int playerRotationDeg, int numRays, Map map) {
    Player pl = malloc(sizeof(struct player));
    assert(pl != NULL);

    pl->posX = playerX;
    pl->posY = playerY;
    pl->size = 6;
    pl->speed = 250;
    pl->rotation = (double) playerRotationDeg*DEG2RAD;
    pl->rotationSpeed = 80*DEG2RAD;
    pl->sensitivity = 80*DEG2RAD;
    pl->FOV = 60;
    pl->numRays = numRays;
    pl->map = map;

    pl->rays = malloc(sizeof(MapRay)*numRays);
    assert(pl->rays != NULL);

    // Initialize rays.
    if (pl->numRays == 1) {
        pl->rays[0] = MapRayCreate((int) pl->posX, (int) pl->posY, pl->rotation, 0, pl->map);
    } else {   
        double angle_offset = -pl->FOV/2;
        for (int i = 0; i < pl->numRays; i++) {
            pl->rays[i] = MapRayCreate((int) pl->posX, (int) pl->posY, pl->rotation, angle_offset*DEG2RAD, pl->map);
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

static void updateRays(Player p) {
    assert(p != NULL);

    for (int i = 0; i < p->numRays; i++) {
        MapRaySetAngle(p->rays[i], p->rotation);
        MapRaySetPosition(p->rays[i], (int) p->posX, (int) p->posY);
        MapRayCast(p->rays[i]);
    }
}

void PlayerRotate(Player p, double rot) { // rot is in radians
    assert(p != NULL);

    p->rotation += rot;
}

int PlayerGetX(Player p) {
    assert(p != NULL);
    
    return (int) p->posX;
}

int PlayerGetY(Player p) {
    assert(p != NULL);
    
    return (int) p->posY;
}

int PlayerGetRotationDeg(Player p) {
    assert(p != NULL);
    
    return (int) (p->rotation*RAD2DEG);
}

double PlayerGetRotationRad(Player p) {
    assert(p != NULL);
    
    return p->rotation;
}

double PlayerGetCameraSensitivity(Player p) {
    assert(p != NULL);
    
    return p->sensitivity;
}


bool PlayerIsColliding(Player p) {
    assert(p != NULL);

    // No collision if there's no map.
    if (p->map == NULL) {
        return false;
    }

    bool colliding = isColliding((int) p->posX, (int) p->posY, p->map);

    // Try collision at 8 points around player.
    for (int angle = 0; angle < 360; angle+=45) {
        colliding = colliding || isColliding((int) (p->posX + 10*cos(p->rotation + angle*DEG2RAD)), (int) (p->posY + 10*sin(p->rotation + angle*DEG2RAD)), p->map);
    }
    return colliding;
}


void PlayerDraw2D(Player p) {
    assert(p != NULL);

    DrawCircle((int) p->posX, (int) p->posY, (float) p->size, (Color) {255, 0, 0, 255});
    DrawLine((int) p->posX, (int) p->posY, (int) (p->posX + (20*cos(p->rotation))), (int) (p->posY + (20*sin(p->rotation))), (Color) {0, 0, 255, 255});

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

            Vector2 collisionPoint = (Vector2) {(float) currentCollision.collisionX, (float) currentCollision.collisionY};
            
            if (currentCollision.collisionType == COLLISION_MAP_TILE) {
                double distaux = sqrt(pow(p->posX-collisionPoint.x, 2) + pow(p->posY-collisionPoint.y, 2));
                double distance = (1.5*MapGetTileSize(p->map)*screenHeight) / (distaux*cos(MapRayGetAngleOffsetRad(ray)));

                Color drawColor = currentCollision.hitSide == X_AXIS ?
                    (Color) {255, 255, 255, 255}
                  : (Color) {210, 210, 210, 255};
    
                Texture tex = TileGetTexture(currentCollision.tile);
                
                int coll_point_axis = currentCollision.hitSide == X_AXIS ?
                    (int) (collisionPoint.y)
                  : (int) (collisionPoint.x);

                // Percentage of tile that ray hit (not really percentage, just number of tile pixels)
                int ray_percentage = coll_point_axis % MapGetTileSize(p->map) + 1;
                
                double texture_offset = ((double) (ray_percentage) / (double) (MapGetTileSize(p->map)))*((double) tex.width);
                int texture_width = 1;
    
                DrawTexturePro(tex,
                    (Rectangle) {(float) (texture_offset-1), 0, (float) texture_width, (float) tex.height},
                    (Rectangle) {(float) rayX, (float) ((screenHeight/2)-(distance/2)), (float) line_width, (float) distance},
                    (Vector2) {0, 0}, 0, drawColor);
                
                ListMoveToNext(collisions);
            } else if (currentCollision.collisionType == COLLISION_BILLBOARD) {
                Billboard bb = currentCollision.billboard;

                double distaux = sqrt(pow(p->posX-collisionPoint.x, 2) + pow(p->posY-collisionPoint.y, 2));
                double distance = (1.5*BillboardGetSize(bb)*screenHeight) / (distaux*cos(MapRayGetAngleOffsetRad(ray)));
                
                Color drawColor = (Color) {255, 255, 255, 255};
    
                Texture tex = BillboardGetTexture(bb);

                // Project collision point to plane

                double bbX = BillboardGetX(bb);
                double bbY = BillboardGetY(bb);

                // Angle between bb and player
                double angle_bb_player = atan2(p->posY-bbY, p->posX-bbX);

                // Vector from point in plane to collision point
                double dx = collisionPoint.x - bbX;
                double dy = collisionPoint.y - bbY;

                // Plane normal vector
                double nx = cos(angle_bb_player + PI/2);
                double ny = sin(angle_bb_player + PI/2);

                double dist = dx*nx + dy*ny;

                double dist_normalized = (dist/(2*BillboardGetSize(bb))) + 0.5;

                double texture_offset = dist_normalized*tex.width;
                
                int texture_width = 1;    

                DrawTexturePro(tex,
                    (Rectangle) {(float) (texture_offset-1), 0, (float) texture_width, (float) tex.height},
                    (Rectangle) {(float) rayX, (float) ((screenHeight/2)-(distance/2)), (float) line_width, (float) distance},
                    (Vector2) {0, 0}, 0, drawColor);
                
                ListMoveToNext(collisions);
            }
        }
    }
}

void PlayerInput(Player p) {
    assert(p != NULL);

    double deltatime = GetFrameTime();
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

    double move_amount_x = p->speed*cos(movement_angle)*deltatime;
    double move_amount_y = p->speed*sin(movement_angle)*deltatime;

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
        PlayerRotate(p, -p->rotationSpeed*deltatime);
    }
    if (IsKeyDown(KEY_RIGHT)) {
        PlayerRotate(p, p->rotationSpeed*deltatime);
    }

    updateRays(p);
}
