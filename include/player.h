#include <stdbool.h>
#include "map.h"

#ifndef PLAYER_H
#define PLAYER_H

typedef struct player* Player;

Player PlayerCreate(int playerX, int playerY, int playerRotationDeg, int numRays, Map map);
void PlayerDestroy(Player* pp);

void PlayerSetMap(Player p, Map map);
void PlayerRotate(Player p, double rot); // rot is in radians
int PlayerGetX(Player p);
int PlayerGetY(Player p);
int PlayerGetRotationDeg(Player p);
double PlayerGetRotationRad(Player p);
double PlayerGetCameraSensitivity(Player p);

bool PlayerIsColliding(Player p);

void PlayerDraw2D(Player p);
void PlayerDraw3D(Player p, int screenWidth, int screenHeight);

void PlayerInput(Player p);

#endif