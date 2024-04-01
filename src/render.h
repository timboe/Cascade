#pragma once
#include "game.h"


void render(int32_t fc);

void addTrauma(float amount);

void addFreeze(uint16_t amount);

bool getSubFreeze(void);

void setBallTrace(const uint16_t i, const uint16_t x, const uint16_t y);