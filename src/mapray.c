#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "mapray.h"
#include "raymath.h"
#include "list.h"

#define MAX_RAY_STEPS 50

struct mapray {
    double angle;               // Not the true angle (usually the same as the player's angle);  Radians.
    double angle_offset;        // Offset in relation to player; Add this to angle to get the true angle; Radians.
    int max_length;             // In pixels.
    double length;              // Current length (in pixels)
    int posX;                   // Start position
    int posY;                   //
    bool is_colliding;
    List collisions;
    Map map;                    // Map where this ray is currently in
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

// Internal: returns a collision object from MapRay->collisions
static rayCollision getCollision(List collisions, int idx) {
    return (*((rayCollision*) ListGet(collisions, idx)));
}

MapRay MapRayCreate(int posX, int posY, double angle, double angleOffset, Map map) {
    MapRay map_ray = malloc(sizeof(struct mapray));
    assert(map_ray != NULL);

    map_ray->angle = angle;
    map_ray->angle_offset = angleOffset;
    map_ray->max_length = 500;
    map_ray->length = 0;
    map_ray->posX = posX;
    map_ray->posY = posY;
    map_ray->is_colliding = false;
    map_ray->map = map;
    map_ray->collisions = ListCreate(NULL);

    return map_ray;
}

void MapRayDestroy(MapRay* mrp) {
    assert(mrp != NULL);
    assert(*mrp != NULL);

    MapRay map_ray = *mrp;

    ListMoveToStart(map_ray->collisions);
    while (ListCanOperate(map_ray->collisions)) {
        free(ListGetCurrent(map_ray->collisions));
        ListRemoveFirst(map_ray->collisions);
    }
    ListDestroy(&map_ray->collisions);

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

int MapRayGetCollisionNumber(MapRay ray) {
    assert(ray != NULL);

    return ListGetSize(ray->collisions);
}

List MapRayGetCollisions(MapRay ray)  {
    assert(ray != NULL);

    return ray->collisions;
}

Vector2 MapRayGetCollisionPoint(MapRay ray, int idx) {
    assert(ray != NULL);
    assert(idx >= 0 && idx < MapRayGetCollisionNumber(ray));

    if (ray->map == NULL) {
        return (Vector2) {0, 0};
    }
    
    return (Vector2) {getCollision(ray->collisions, idx).collisionX, getCollision(ray->collisions, idx).collisionY};
}

Vector2 MapRayGetCollisionPointGrid(MapRay ray, int idx) {
    assert(ray != NULL);
    assert(idx >= 0 && idx < MapRayGetCollisionNumber(ray));
    
    if (ray->map == NULL) {
        return (Vector2) {0, 0};
    }
    
    return (Vector2) {getCollision(ray->collisions, idx).collisionGridX, getCollision(ray->collisions, idx).collisionGridY};
}

int MapRayGetMaxLength(MapRay ray) {
    assert(ray != NULL);

    return ray->max_length;
}

double MapRayGetLength(MapRay ray) {
    assert(ray != NULL);

    return ray->length;
}


MapRayHitSide MapRayGetHitSide(MapRay ray, int idx) {
    assert(ray != NULL);

    return getCollision(ray->collisions, idx).hitSide;
}


void MapRayCast(MapRay ray) {
    assert(ray != NULL);

    // Set start variables
    ray->length = 0;
    ray->is_colliding = false;

    // clear collisions
    ListMoveToStart(ray->collisions);
    while (ListCanOperate(ray->collisions)) {
        free(ListGetCurrent(ray->collisions));
        ListRemoveFirst(ray->collisions);
    }

    // No map behaviour
    if (ray->map == NULL) {
        return;
    }
    // If inside wall, do not do anything more.
    ray->is_colliding = isColliding(ray->posX, ray->posY, ray->map);
    if (ray->is_colliding) {
        return;
    }

    int tileSize = MapGetTileSize(ray->map);

    // Position of ray in map
    int mapX = (int) ray->posX / MapGetTileSize(ray->map);
    int mapY = (int) ray->posY / MapGetTileSize(ray->map);

    // Angle of ray by axis
    double rayDirX = cos(MapRayGetTrueAngleRad(ray));
    double rayDirY = sin(MapRayGetTrueAngleRad(ray));
    
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
        sideDistY = (ray->posY - mapY * tileSize) * deltaDistY / tileSize;
    } else {
        sideY = 1;
        sideDistY = ((mapY + 1)*tileSize - ray->posY) * deltaDistY / tileSize;
    }
    
    int i = 0;
    while (!ray->is_colliding) {
        MapRayHitSide hitSide;

        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += sideX;
            ray->length = sideDistX - deltaDistX;
            hitSide = X_AXIS;
        } else {
            sideDistY += deltaDistY;
            mapY += sideY;
            ray->length = sideDistY - deltaDistY;
            hitSide = Y_AXIS;
        }
        
        ray->is_colliding = MapGetTile(ray->map, mapX, mapY) != TILE_GROUND;

        // If the colliding tile is transparent, then just continue
        if (ray->is_colliding) {
            Tile collidingTile = MapGetTileObject(ray->map, MapGetTile(ray->map, mapX, mapY));
            rayCollision* col = malloc(sizeof(rayCollision));
            *col = (rayCollision) {
                .collisionX = ray->posX + ray->length * rayDirX,
                .collisionY = ray->posY + ray->length * rayDirY,
                .collisionGridX = mapX,
                .collisionGridY = mapY,
                .tile = collidingTile,
                .hitSide = hitSide
            };
            ListAppendFirst(ray->collisions, col);
            ray->is_colliding = !TileIsTransparent(collidingTile);
        }

        if (i == MAX_RAY_STEPS) {
            break;
        }
        i++;
    }
    
    ray->is_colliding = ListGetSize(ray->collisions) > 0;
}

void MapRayDraw2D(MapRay ray) {
    assert(ray != NULL);
    
    Color color;
    if (ray->is_colliding) {
        color = (Color) {255, 0, 255, 255};
    } else {
        color = (Color) {0, 255, 255, 255};
    }
        
    if (ray->is_colliding) {
        rayCollision lastCollision = getCollision(ray->collisions, 0);
  
        DrawLine(ray->posX, ray->posY, lastCollision.collisionX, lastCollision.collisionY, color);
        
        ListMoveToStart(ray->collisions);
        while (ListCanOperate(ray->collisions)) {
            rayCollision current = *((rayCollision*) ListGetCurrent(ray->collisions));
            ListMoveToNext(ray->collisions);
        }
    } else {
        DrawLine(ray->posX, ray->posY, ray->posX + MapRayGetLength(ray)*cos(ray->angle + ray->angle_offset), ray->posY + MapRayGetLength(ray)*sin(ray->angle + ray->angle_offset), color);
    }
}

