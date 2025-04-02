#ifndef PLAYER_H
#define PLAYER_H

typedef struct player* Player;

Player PlayerCreate(int playerX, int playerY, int playerRotationDeg);
void PlayerDestroy(Player* pp);

int PlayerGetX(Player p);
int PlayerGetY(Player p);
int PlayerGetRotationDeg(Player p);
int PlayerGetRotationRad(Player p);

void PlayerDraw2D(Player p);

void PlayerInput(Player p);

#endif