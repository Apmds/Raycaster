#include <assert.h>
#include <stdlib.h>
#include "mapray.h"

struct mapray {
    double angle;               // Not the true angle (usually the same as the player's angle);  Radians.
    double angle_offset;        // Offset in relation to player; Add this to angle to get the true angle; Radians.
    int max_length;             // In pixels.
    int posX;
    int posY;
    bool is_colliding;
    int collisionX;
    int collisionY;
    Map map;
};


MapRay MapRayCreate(int posX, int posY, double angle, double angleOffset, Map map) {
    MapRay map_ray = malloc(sizeof(struct mapray));
    assert(map_ray != NULL);

    map_ray->angle = angle;
    map_ray->angle_offset = angleOffset;
    map_ray->max_length = 300;
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


void MapRayCast(MapRay ray) {
    assert(ray != NULL);
    
    // TODO: implement
    int length = 0;
    ray->collisionX = 0;
    ray->collisionY = 0;
    
    while (length < ray->max_length && !ray->is_colliding) {
        /* code */
        break;
    }
    
}

void MapRayDraw2D(MapRay ray) {
    assert(ray != NULL);
    
    // TODO: implement
}

