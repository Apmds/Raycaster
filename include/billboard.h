
#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "raylib.h"

typedef struct billboard* Billboard;

Billboard BillboardCreate(Texture sprite, int posX, int posY, int size);
void BillboardDestroy(Billboard* bp);

Texture BillboardGetTexture(const struct billboard* bb);

int BillboardGetX(const struct billboard* bb);
int BillboardGetY(const struct billboard* bb);
int BillboardGetSize(const struct billboard* bb);

void BillboardSetX(Billboard bb, int posX);
void BillboardSetY(Billboard bb, int posY);

#endif