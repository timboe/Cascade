#pragma once
#include "game.h"
#include "chipmunk/chipmunk.h"

void initSpace(void);

void updateSpace(float timestep);

void updatePath(void);

bool ballInPlay(void);

void setBallInPlay(bool bip);

cpBody* getBall(void);

cpShape* getBallShape(void);

void launchBall(void);

void resetBall(void);

cpSpace* getSpace(void);

uint8_t radToByte(float rad);

uint8_t angToByte(float ang);

float angToRad(float ang);

float len(const float x1, const float x2, const float y1, const float y2);

float len2(const float x1, const float x2, const float y1, const float y2);