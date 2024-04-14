#pragma once
#include "game.h"
#include "fsm.h"

void renderDo(const int32_t fc, const enum FSM_t fsm, const enum GameMode_t gm);

void renderAddTrauma(const float amount);

void renderAddFreeze(const uint16_t amount);

bool renderGetSubFreeze(void);

void renderSetScale(const uint8_t scale);

void renderSetBallTrace(const uint16_t i, const uint16_t x, const uint16_t y);

void renderSetBallPootCircle(const uint16_t radius);

void renderDoTriggerSplash(const uint8_t ball, const int16_t x);
void renderDoResetTriggerSplash(void);

void renderDoAddStar(const uint8_t ball);
void renderDoResetStars(void);

void renderSetBallFallN(const uint16_t n);
void renderSetBallFallX(const uint16_t x);
void renderSetBallFallY(const uint16_t ball, const float y);

void renderSetNumeralOffset(const float no);

void renderDoResetBallTrace(void);

void renderTitlesSplash(void);
void renderTitlesPlayerSelect(const bool locked);
void renderTitlesLevelSelect(const bool locked);
void renderTitlesHoleSelect(const bool locked);
void renderTitlesTransitionLevelSplash(void);

void renderGameBall(const int32_t fc);
void renderGamePoot(const enum FSM_t fsm);
void renderGameTurret(void);
void renderGameTrajectory(void);
void renderGameBoard(void);
void renderGameBackground(void);
void renderGameGutter(void);