#pragma once
#include "game.h"
#include "ui.h"


void render(int32_t fc, enum kFSM fsm);

void addTrauma(float amount);

void addFreeze(uint16_t amount);

bool getSubFreeze(void);

void setBallTrace(const uint16_t i, const uint16_t x, const uint16_t y);

void setBallPootCircle(uint16_t radius);

void setBallFallN(uint16_t n);

void setBallFallY(uint16_t ball, float y);

void resetBallTrace(void);
