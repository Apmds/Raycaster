#include <stdbool.h>
#include "map.h"

#ifndef PLAYER_H
#define PLAYER_H

typedef struct player* Player;

Player PlayerCreate(int playerX, int playerY, int playerRotationDeg, int numRays, Map map);
void PlayerDestroy(Player* pp);

void PlayerSetMap(Player p, Map map);
int PlayerGetX(Player p);
int PlayerGetY(Player p);
int PlayerGetRotationDeg(Player p);
double PlayerGetRotationRad(Player p);

bool PlayerIsColliding(Player p);

void PlayerDraw2D(Player p);

void PlayerInput(Player p);

#endif