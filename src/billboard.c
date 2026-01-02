#include "billboard.h"
#include "raylib.h"
#include <stdlib.h>
#include <assert.h>

struct billboard {
    Texture sprite;
    int posX;
    int posY;
    int size;
};


Billboard BillboardCreate(Texture sprite, int posX, int posY, int size) {
    Billboard bb = malloc(sizeof(struct billboard));
    assert(bb != NULL);

    bb->sprite = sprite;
    bb->posX = posX;
    bb->posY = posY;
    bb->size = size;

    return bb;
}

void BillboardDestroy(Billboard* bbp) {
    assert(bbp != NULL);
    assert(*bbp != NULL);

    Billboard bb = *bbp;

    free(bb);

    *bbp = NULL;
}

Texture BillboardGetTexture(const struct billboard* bb) {
    assert(bb != NULL);

    return bb->sprite;
}

int BillboardGetX(const struct billboard* bb) {
    assert(bb != NULL);

    return bb->posX;
}

int BillboardGetY(const struct billboard* bb) {
    assert(bb != NULL);

    return bb->posY;
}

int BillboardGetSize(const struct billboard* bb) {
    assert(bb != NULL);

    return bb->size;
}

void BillboardSetX(Billboard bb, int posX) {
    assert(bb != NULL);

    bb->posX = posX;
}

void BillboardSetY(Billboard bb, int posY) {
    assert(bb != NULL);

    bb->posX = posY;
}
