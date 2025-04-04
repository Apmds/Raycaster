#include <assert.h>
#include <stdlib.h>
#include "mapray.h"

struct mapray {
    double angle;
    double angle_offset;        // Offset em relação ao player
};


MapRay MapRayCreate(int angle_offset) {
    MapRay map_ray = malloc(sizeof(struct mapray));
    assert(map_ray != NULL);

    map_ray->angle_offset = angle_offset;
    map_ray->angle = angle_offset;

    return map_ray;
}

void MapRayDestroy(MapRay* mrp) {
    assert(mrp != NULL);
    assert(*mrp != NULL);

    MapRay map_ray = *mrp;

    free(map_ray);

    *mrp = NULL;
}

