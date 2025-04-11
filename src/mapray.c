#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "mapray.h"
#include "raymath.h"

struct mapray {
    double angle;               // Not the true angle (usually the same as the player's angle);  Radians.
    double angle_offset;        // Offset in relation to player; Add this to angle to get the true angle; Radians.
    int max_length;             // In pixels.
    int posX;
    int posY;
    bool is_colliding;
    double collisionX;
    double collisionY;
    Map map;
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

MapRay MapRayCreate(int posX, int posY, double angle, double angleOffset, Map map) {
    MapRay map_ray = malloc(sizeof(struct mapray));
    assert(map_ray != NULL);

    map_ray->angle = angle;
    map_ray->angle_offset = angleOffset;
    map_ray->max_length = 500;
    map_ray->posX = posX;
    map_ray->posY = posY;
    map_ray->is_colliding = false;
    map_ray->collisionX = 0;
    map_ray->collisionY = 0;
    map_ray->map = map;

    return map_ray;
}

void MapRayDestroy(MapRay* mrp) {
    assert(mrp != NULL);
    assert(*mrp != NULL);

    MapRay map_ray = *mrp;

    free(map_ray);

    *mrp = NULL;
}


void MapRaySetAngle(MapRay ray, double angle) {
    assert(ray != NULL);

    ray->angle = angle;
}

void MapRaySetPosition(MapRay ray, int posX, int posY) {
    assert(ray != NULL);

    ray->posX = posX;
    ray->posY = posY;
}

void MapRaySetMap(MapRay ray, Map map) {
    assert(ray != NULL);

    ray->map = map;
}

double MapRayGetTrueAngleRad(MapRay ray) {
    assert(ray != NULL);

    return ray->angle + ray->angle_offset;
}

int MapRayGetTrueAngleDeg(MapRay ray) {
    assert(ray != NULL);

    return (int) ((ray->angle + ray->angle_offset)*RAD2DEG);
}

double MapRayGetAngleOffsetRad(MapRay ray) {
    assert(ray != NULL);

    return ray->angle_offset;
}


bool MapRayIsColliding(MapRay ray) {
    assert(ray != NULL);

    return ray->is_colliding;
}

Vector2 MapRayGetCollisionPoint(MapRay ray) {
    assert(ray != NULL);
    
    return (Vector2) {ray->collisionX, ray->collisionY};
}

Vector2 MapRayGetCollisionPointGrid(MapRay ray) {
    assert(ray != NULL);
    
    if (ray->map == NULL) {
        return (Vector2) {0, 0};
    }
    
    return (Vector2) {(int) (ray->collisionX / MapGetTileSize(ray->map)), (int) (ray->collisionY / MapGetTileSize(ray->map))};
}

int MapRayGetMaxLength(MapRay ray) {
    assert(ray != NULL);

    return ray->max_length;
}


void MapRayCast(MapRay ray) {
    assert(ray != NULL);

    // Set start variables
    double length = 0;
    ray->collisionX = ray->posX;
    ray->collisionY = ray->posY;
    ray->is_colliding = false;

    // No map behaviour
    if (ray->map == NULL) {
        return;
    }
    // If inside wall, do not do anything more.
    ray->is_colliding = isColliding(ray->collisionX, ray->collisionY, ray->map);
    if (ray->is_colliding) {
        return;
    }

    int tileSize = MapGetTileSize(ray->map);

    // Position of ray in map
    Vector2 mapPos = MapRayGetCollisionPointGrid(ray);
    int mapX = (int) mapPos.x;
    int mapY = (int) mapPos.y;
    //printf("init: %d\n", mapX);

    // Angle of ray by axis
    double rayDirX = cos(MapRayGetTrueAngleRad(ray));
    double rayDirY = sin(MapRayGetTrueAngleRad(ray));
    //printf("%f, %f\n", rayDirX, rayDirY);
    
    // Length that ray must traverse to go from one X/Y to the next, respectively.
    double deltaDistX = ray->max_length;
    if (rayDirX != 0) {
        deltaDistX = fabs(1 / rayDirX);
        deltaDistX *= tileSize;
    }
    double deltaDistY = ray->max_length;
    if (rayDirY != 0) {
        deltaDistY = fabs(1 / rayDirY);
        deltaDistY *= tileSize;
    }

    // Length that ray must traverse initially to go to the next X/Y, respectively. (fraction of deltaDist)
    double sideDistX;
    double sideDistY;
    // Side where player is facing in each direction
    int sideX;
    int sideY;
    if (rayDirX < 0) {
        sideX = -1;
        sideDistX = (ray->posX - mapX * tileSize) * deltaDistX / tileSize;
    } else {
        sideX = 1;
        sideDistX = ((mapX + 1)*tileSize - ray->posX) * deltaDistX / tileSize;
    }
    if (rayDirY < 0) {
        sideY = -1;
        sideDistY = (ray->posX - mapY * tileSize) * deltaDistY / tileSize;
    } else {
        sideY = 1;
        sideDistY = ((mapY + 1)*tileSize - ray->posY) * deltaDistY / tileSize;
    }
    
    int i = 0;
    while (/*length < ray->max_length && */!ray->is_colliding) {
        // Slow method
        //length += 1;
        //ray->collisionX = ray->posX + length*cos(MapRayGetTrueAngleRad(ray));
        //ray->collisionY = ray->posY + length*sin(MapRayGetTrueAngleRad(ray));
        //ray->is_colliding = isColliding(ray->collisionX, ray->collisionY, ray->map);

        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += sideX;
            length = sideDistX - deltaDistX;
            //length = (mapX - ray->posX + (1 - sideX) / 2) / rayDirX;
        } else {
            sideDistY += deltaDistY;
            mapY += sideY;
            length = sideDistY - deltaDistY;
            //length = (mapY - ray->posY + (1 - sideY) / 2) / rayDirY;
        }
        //printf("length: %f, max: (%d, %d), rayDir: (%f, %f), sideDist: (%f, %f), deltaDist: (%f, %f)\n", length, mapX, mapY, rayDirX, rayDirY, sideDistX, sideDistY, deltaDistX, deltaDistY);
        
        ray->is_colliding = MapGetTile(ray->map, mapX, mapY) != GROUND;
        if (i == 50) {
            break;
        }
        i++;
    }
    //ray->collisionX = mapX*tileSize;
    //ray->collisionY = mapY*tileSize;
    ray->collisionX = ray->posX + length*rayDirX;
    ray->collisionY = ray->posY + length*rayDirY;
    //printf("%d, %d\n", mapX, mapY);
}

void MapRayDraw2D(MapRay ray) {
    assert(ray != NULL);
    
    // TODO: implement
    Color color;
    if (ray->is_colliding) {
        color = (Color) {255, 0, 255, 255};
    } else {
        color = (Color) {0, 255, 255, 255};
    }
    DrawLine(ray->posX, ray->posY, ray->collisionX, ray->collisionY, color);
}

