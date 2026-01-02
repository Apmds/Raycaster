
#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "raylib.h"

typedef struct billboard* Billboard;
typedef const struct billboard* CBillboard;

Billboard BillboardCreate(Texture sprite, int posX, int posY, int size);
void BillboardDestroy(Billboard* bp);

Texture BillboardGetTexture(CBillboard bb);

int BillboardGetX(CBillboard bb);
int BillboardGetY(CBillboard bb);
int BillboardGetSize(CBillboard bb);

void BillboardSetX(Billboard bb, int posX);
void BillboardSetY(Billboard bb, int posY);

#endif