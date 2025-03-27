#include <stdlib.h>
#include <assert.h>
#include "player.h"

struct player {
    int posX;
    int posY;
    int size;
    int speed;
    int rotation;
    int rotationSpeed;
};


Player PlayerCreate(int playerX, int playerY, int playerRotationDeg) {
    Player pl = malloc(sizeof(struct player));
    assert(pl != NULL);

    pl->posX = playerX;
    pl->posY = playerY;
    pl->size = 10;
    pl->speed = 2;
    pl->rotation = playerRotationDeg;
    pl->rotationSpeed = 1;

    return pl;
}

void PlayerDestroy(Player* pp) {
    assert(pp != NULL);
    assert(*pp != NULL);

    Player p = *pp;

    free(p);

    *pp = NULL;
}

int PlayerGetX(Player p) {
    return p->posX;
}

int PlayerGetY(Player p) {
    return p->posY;
}

int PlayerGetRotationDeg(Player p) {
    return p->rotation;
}

int PlayerGetRotationRad(Player p) {
    return p->rotation*DEG2RAD;
}

void PlayerDraw2D(Player p) {
    DrawCircle(p->posX, p->posY, p->size, (Color) {255, 0, 0, 255});
    DrawLine(p->posX, p->posY, p->posX + (20*cos(DEG2RAD*p->rotation)), p->posY + (20*sin(DEG2RAD*p->rotation)), (Color) {0, 0, 255, 255});
}

void PlayerInput(Player p) {
    if (IsKeyDown(KEY_W)) {
        p->posY -= p->speed;
    }
    if (IsKeyDown(KEY_S)) {
        p->posY += p->speed;
    }
    if (IsKeyDown(KEY_A)) {
        p->posX -= p->speed;
    }
    if (IsKeyDown(KEY_D)) {
        p->posX += p->speed;
    }
    if (IsKeyDown(KEY_LEFT)) {
        p->rotation -= p->speed;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        p->rotation += p->speed;
    }
}