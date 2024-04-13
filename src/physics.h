#pragma once
#include "game.h"
#include "fsm.h"
#include "chipmunk/chipmunk.h"

void physicsDoInit(void);

void physicsDoUpdate(const int32_t fc, const enum FSM_t fsm);

cpBody* physicsGetBall(const uint8_t n);

cpSpace* physicsGetSpace(void);

void physicsDoAddSecondBall(void);
void physicsDoRemoveSecondBall(void);

void physicsSetSecondBallInPlay(void);
bool physicsGetSecondBallInPlay(void);

int16_t* physicsGetMotionTrailX(const uint8_t n);
int16_t* physicsGetMotionTrailY(const uint8_t n);

void physicsDoLaunchBall(const float strength);

void physicsDoResetBall(uint8_t n);

void physicsDoSecondTryBall(void);

