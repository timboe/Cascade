#pragma once
#include "game.h"

void gameClickConfigHandler(uint32_t _buttonPressed);

void clickHandlerReplacement(void);

bool getPressed(PDButtons b);

float getCrankAngle(void);

float getCrankChanged(void);

bool getPressedAny(void);

bool bPressed(void);
