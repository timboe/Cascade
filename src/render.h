#pragma once
#include "game.h"
#include "fsm.h"

void render(const int32_t fc, const enum kFSM fsm, const enum kGameMode gm);

void addTrauma(const float amount);

void addFreeze(const uint16_t amount);

bool getSubFreeze(void);

void setBallTrace(const uint16_t i, const uint16_t x, const uint16_t y);

void setBallPootCircle(const uint16_t radius);

void setBallFallN(const uint16_t n);
void setBallFallX(const uint16_t x);
void setBallFallY(const uint16_t ball, const float y);

void setNumeralOffset(const float no);

void resetBallTrace(void);

void renderTitlesSplash(void);
void renderTitlesPlayerSelect(const bool locked);
void renderTitlesLevelSelect(const bool locked);
void renderTitlesHoleSelect(const bool locked);
void renderTitlesTransitionLevelSplash(void);

void renderGameBall(const int32_t fc);
void renderGamePoot(const enum kFSM fsm);
void renderGameTurret(void);
void renderGameTrajectory(void);
void renderGameBoard(void);
void renderGameBackground(void);
void renderGameGutter(void);