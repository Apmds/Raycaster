

#ifndef MAPRAY_H
#define MAPRAY_H

typedef struct mapray* MapRay;

MapRay MapRayCreate(int angle_offset);
void MapRayDestroy(MapRay* mrp);




#endif