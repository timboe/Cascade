#pragma once
#include "game.h"
#include "ui.h"
#include "chipmunk/chipmunk.h"

void initSpace(void);

void updateSpace(int32_t frameCount, enum kFSM fsm);

cpBody* getBall(uint8_t n);

void addSecondBall(void);

void removeSecondBall(void);

void setSecondBallInPlay(void);

bool getSecondBallInPlay(void);

// cpShape* getBallShape(void);

int16_t* motionTrailX(uint8_t n);

int16_t* motionTrailY(uint8_t n);

void launchBall(float strength);

void resetBall(uint8_t n);

void secondTryBall(void);

cpSpace* getSpace(void);

uint8_t radToByte(float rad);

uint8_t angToByte(float ang);

float angToRad(float ang);

float len(const float x1, const float x2, const float y1, const float y2);

float len2(const float x1, const float x2, const float y1, const float y2);