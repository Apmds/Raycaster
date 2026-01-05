#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "map.h"

typedef struct player* Player;
typedef const struct player* CPlayer;

Player PlayerCreate(int playerX, int playerY, int playerRotationDeg, int numRays, Map map);
void PlayerDestroy(Player* pp);

void PlayerSetMap(Player p, Map map);
void PlayerRotate(Player p, double rot); // rot is in radians
int PlayerGetX(CPlayer p);
int PlayerGetY(CPlayer p);
int PlayerGetRotationDeg(CPlayer p);
double PlayerGetRotationRad(CPlayer p);
double PlayerGetCameraSensitivity(CPlayer p);

bool PlayerIsColliding(Player p);

void PlayerDraw2D(CPlayer p);
void PlayerDraw3D(Player p, int screenWidth, int screenHeight);

void PlayerInput(Player p);

#endif