#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "player.h"
#include "raylib.h"
#include "raymath.h"

struct player {
    double posX;
    double posY;
    int size;
    int speed;
    double rotation;               // Radianos
    double rotationSpeed;          // Radianos
    Map map;
};


Player PlayerCreate(int playerX, int playerY, int playerRotationRad) {
    Player pl = malloc(sizeof(struct player));
    assert(pl != NULL);

    pl->posX = playerX;
    pl->posY = playerY;
    pl->size = 10;
    pl->speed = 10;
    pl->rotation = playerRotationRad;
    pl->rotationSpeed = 2*DEG2RAD;
    pl->map = NULL;

    return pl;
}

void PlayerDestroy(Player* pp) {
    assert(pp != NULL);
    assert(*pp != NULL);

    Player p = *pp;

    free(p);

    *pp = NULL;
}

void PlayerSetMap(Player p, Map map) {
    assert(p != NULL);

    p->map = map;
}


int PlayerGetX(Player p) {
    assert(p != NULL);
    
    return p->posX;
}

int PlayerGetY(Player p) {
    assert(p != NULL);
    
    return p->posY;
}

int PlayerGetRotationDeg(Player p) {
    assert(p != NULL);
    
    return p->rotation*RAD2DEG;
}

int PlayerGetRotationRad(Player p) {
    assert(p != NULL);
    
    return p->rotation;
}

bool PlayerIsColliding(Player p) {
    assert(p != NULL);

    if (p->map == NULL) {
        return false;
    }

    int gridPosX = (int) p->posX / MapGetTileSize(p->map);
    int gridPosY = (int) p->posY / MapGetTileSize(p->map);

    return MapGetTile(p->map, gridPosX, gridPosY) != GROUND;
}


void PlayerDraw2D(Player p) {
    assert(p != NULL);

    DrawCircle(p->posX, p->posY, p->size, (Color) {255, 0, 0, 255});
    DrawLine(p->posX, p->posY, p->posX + (20*cos(p->rotation)), p->posY + (20*sin(p->rotation)), (Color) {0, 0, 255, 255});
}

void PlayerInput(Player p) {
    assert(p != NULL);

    double movement_angle = p->rotation;
    bool moving = false;

    if (IsKeyDown(KEY_W)) {
        moving = true;
        movement_angle = p->rotation + 0;
    }
    if (IsKeyDown(KEY_S)) {
        moving = true;
        movement_angle = p->rotation + 180*DEG2RAD;
    }
    if (IsKeyDown(KEY_A)) {
        moving = true;
        movement_angle = p->rotation + 270*DEG2RAD;
    }
    if (IsKeyDown(KEY_D)) {
        moving = true;
        movement_angle = p->rotation + 90*DEG2RAD;
    }

    double move_amount_x = p->speed*cos(movement_angle);
    double move_amount_y = p->speed*sin(movement_angle);

    // Movimento com colisão
    if (moving) {
        p->posX += move_amount_x;
        if (PlayerIsColliding(p)) {
            p->posX -= move_amount_x;
        }
        
        p->posY += move_amount_y;
        if (PlayerIsColliding(p)) {
            p->posY -= move_amount_y;
        }
    }

    if (IsKeyDown(KEY_LEFT)) {
        p->rotation -= p->rotationSpeed;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        p->rotation += p->rotationSpeed;
    }
}