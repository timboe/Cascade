#pragma once
#include "game.h"
#include "fsm.h"
#include "physics.h"
#include "peg.h"

void renderDoInit(void);

void renderDo(const int32_t fc, const enum FSM_t fsm, const enum GameMode_t gm);

void renderAddTrauma(const float amount);

void renderAddFreeze(const uint16_t amount);

bool renderGetSubFreeze(void);

void renderSetScale(const uint8_t scale);

void renderSetMarbleTrace(const uint16_t i, const uint16_t x, const uint16_t y);

void renderSetMarblePootCircle(const uint16_t radius);

void renderDoTriggerSplash(const uint8_t ball, const int16_t x);
void renderDoResetTriggerSplash(void);

void renderDoAddSpecialBlast(cpBody* body);

void renderDoAddEndBlast(cpBody* body);
void renderDoResetEndBlast(void);

void renderDoAddSpecial(cpBody* body, const enum PegSpecial_t special);

void renderSetMarbleFallN(const uint16_t n);
void renderSetMarbleFallX(const uint16_t x);
void renderSetMarbleFallY(const uint16_t ball, const float y);

void renderSetNumeralOffset(const float no);

void renderDoResetMarbleTrace(void);

void renderTitlesHeader(const int32_t fc);
void renderTitlesPlayerSelect(const bool locked);
void renderTitlesLevelSelect(const bool locked);
void renderTitlesHoleSelect(const bool locked);
void renderTitlesWfPond(const int32_t fc);
void renderTitlesTransitionLevelSplash(void);

void renderGameMarble(const int32_t fc);
void renderGamePoot(const enum FSM_t fsm);
void renderGameTurret(void);
void renderGameTrajectory(void);
void renderGameBoard(const int32_t fc);
void renderGamePops(const int32_t fc);
void renderGameFountains(const int32_t fc);
void renderGameBackground(void);
void renderGameGutter(const int32_t fc);
void renderGameTutorial(const int32_t fc, const enum FSM_t fsm);