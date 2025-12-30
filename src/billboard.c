#include "billboard.h"
#include "raylib.h"
#include <stdlib.h>
#include <assert.h>

struct billboard {
    Texture sprite;
    int posX;
    int posY;
};


Billboard BillboardCreate(Texture sprite, int posX, int posY) {
    Billboard bb = malloc(sizeof(struct billboard));
    assert(bb != NULL);

    bb->sprite = sprite;
    bb->posX = posX;
    bb->posY = posY;

    return bb;
}

void BillboardDestroy(Billboard* bbp) {
    assert(bbp != NULL);
    assert(*bbp != NULL);

    Billboard bb = *bbp;

    free(bb);

    *bbp = NULL;
}

Texture BillboardGetTexture(Billboard bb) {
    assert(bb != NULL);

    return bb->sprite;
}

int BillboardGetX(Billboard bb) {
    assert(bb != NULL);

    return bb->posX;
}

int BillboardGetY(Billboard bb) {
    assert(bb != NULL);

    return bb->posY;
}
