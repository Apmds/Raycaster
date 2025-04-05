#include <assert.h>
#include <stdlib.h>
#include "mapray.h"

struct mapray {
    double angle;               // Not the true angle (usually the same as the player's angle);  Radians.
    double angle_offset;        // Offset in relation to player; Add this to angle to get the true angle; Radians.
    int max_length;             // In pixels.
    Map map;
};


MapRay MapRayCreate(double angle, double angleOffset, Map map) {
    MapRay map_ray = malloc(sizeof(struct mapray));
    assert(map_ray != NULL);

    map_ray->angle_offset = angleOffset;
    map_ray->angle = angle;
    map_ray->max_length = 300;
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

void MapRayCast(MapRay ray) {
    assert(ray != NULL);

    // TODO: implement
}

void DrawRay2D(MapRay ray) {
    assert(ray != NULL);

    // TODO: implement
}

