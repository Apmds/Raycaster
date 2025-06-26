#include <stdbool.h>
#include "raylib.h"
#include "map.h"
#include "list.h"

#ifndef MAPRAY_H
#define MAPRAY_H

typedef enum MapRayHitSide {
    X_AXIS,
    Y_AXIS,
    NONE,
} MapRayHitSide;

typedef struct rayCollision {
    double collisionX;          // Position of the collision (pixels)
    double collisionY;          //
    int collisionGridX;         // Position in the map grid of the collision
    int collisionGridY;         //
    MapRayHitSide hitSide;      // Side where the the collision occured (for shading)
    Tile tile;                  // The tile present in the collision
} rayCollision;

typedef struct mapray* MapRay;

MapRay MapRayCreate(int posX, int posY, double angle, double angleOffset, Map map);
void MapRayDestroy(MapRay* mrp);

void MapRaySetAngle(MapRay ray, double angle);
void MapRaySetPosition(MapRay ray, int posX, int posY);
void MapRaySetMap(MapRay ray, Map map);
double MapRayGetTrueAngleRad(MapRay ray);
int MapRayGetTrueAngleDeg(MapRay ray);
double MapRayGetAngleOffsetRad(MapRay ray);
bool MapRayIsColliding(MapRay ray);
int MapRayGetCollisionNumber(MapRay ray);
List MapRayGetCollisions(MapRay ray);
Vector2 MapRayGetCollisionPoint(MapRay ray, int idx);
Vector2 MapRayGetCollisionPointGrid(MapRay ray, int idx);
int MapRayGetMaxLength(MapRay ray);
double MapRayGetLength(MapRay ray);
MapRayHitSide MapRayGetHitSide(MapRay ray, int idx);

void MapRayCast(MapRay ray);
void MapRayDraw2D(MapRay ray);


#endif