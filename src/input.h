#pragma once
#include "game.h"
#include "fsm.h"

void inputDoHandle(const enum FSM_t fsm, const enum GameMode_t gm);

bool inputGetPressed(const PDButtons b);
bool inputGetPressedAny(void);

float inputGetCrankAngle(void);
float inputGetCrankChanged(void);
