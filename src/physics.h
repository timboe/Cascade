#pragma once
#include "game.h"
#include "chipmunk/chipmunk.h"

#define BALL_RADIUS 15.0f
#define BALL_MASS 1.0f

#define M_PIf 3.14159265358979323846f

void initSpace(void);

void updateSpace(void);

cpBody* getBall(void);

cpBody* getObst(uint32_t i);