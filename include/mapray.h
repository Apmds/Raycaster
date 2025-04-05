#include <stdbool.h>
#include "raylib.h"
#include "map.h"

#ifndef MAPRAY_H
#define MAPRAY_H

typedef struct mapray* MapRay;

MapRay MapRayCreate(double angle, double angleOffset, Map map);
void MapRayDestroy(MapRay* mrp);

void MapRaySetAngle(MapRay ray, double angle);
void MapRaySetMap(MapRay ray, Map map);
double MapRayGetTrueAngleRad(MapRay ray);
int MapRayGetTrueAngleDeg(MapRay ray);
bool MapRayIsColliding(MapRay ray);
Vector2 MapRayGetCollisionPoint(MapRay ray);

void MapRayCast(MapRay ray);
void DrawRay2D(MapRay ray);


#endif