
#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "raylib.h"

typedef struct billboard* Billboard;

Billboard BillboardCreate(Texture sprite, int posX, int posY);
void BillboardDestroy(Billboard* bp);

Texture BillboardGetTexture(Billboard bb);

int BillboardGetX(Billboard bb);
int BillboardGetY(Billboard bb);

#endif