#pragma once
#include "game.h"
#include "fsm.h"
#include "chipmunk/chipmunk.h"

void initSpace(void);

void updateSpace(int32_t frameCount, enum FSM_t fsm);

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
