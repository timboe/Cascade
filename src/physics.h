#pragma once
#include "game.h"
#include "chipmunk/chipmunk.h"

void initSpace(void);

void updateSpace(void);

bool ballInPlay(void);

void setBallInPlay(bool bip);

void setDoMotionPath(bool dmp);

cpBody* getBall(void);

cpShape* getBallShape(void);

void launchBall(void);

void resetBall(void);

cpSpace* getSpace(void);

uint8_t radToByte(float rad);

uint8_t angToByte(float ang);

float angToRad(float ang);