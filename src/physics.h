#pragma once
#include "game.h"
#include "ui.h"
#include "chipmunk/chipmunk.h"

void initSpace(void);

void updateSpace(int32_t frameCount, enum kFSM fsm);

cpBody* getBall(void);

// cpShape* getBallShape(void);

int16_t* motionTrailX(void);

int16_t* motionTrailY(void);

void launchBall(float strength);

void resetBall(void);

void secondTryBall(void);

cpSpace* getSpace(void);

uint8_t radToByte(float rad);

uint8_t angToByte(float ang);

float angToRad(float ang);

float len(const float x1, const float x2, const float y1, const float y2);

float len2(const float x1, const float x2, const float y1, const float y2);