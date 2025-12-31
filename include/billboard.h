
#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "raylib.h"

typedef struct billboard* Billboard;

Billboard BillboardCreate(Texture sprite, int posX, int posY, int size);
void BillboardDestroy(Billboard* bp);

Texture BillboardGetTexture(Billboard bb);

int BillboardGetX(Billboard bb);
int BillboardGetY(Billboard bb);
int BillboardGetSize(Billboard bb);

int BillboardSetX(Billboard bb, int posX);
int BillboardSetY(Billboard bb, int posY);

#endif