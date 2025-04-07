#include <stdbool.h>
#include "raylib.h"
#include "map.h"

#ifndef MAPRAY_H
#define MAPRAY_H

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
Vector2 MapRayGetCollisionPoint(MapRay ray);
Vector2 MapRayGetCollisionPointGrid(MapRay ray);
int MapRayGetMaxLength(MapRay ray);

void MapRayCast(MapRay ray);
void MapRayDraw2D(MapRay ray);


#endif