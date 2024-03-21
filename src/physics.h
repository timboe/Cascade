#pragma once
#include "game.h"
#include "chipmunk/chipmunk.h"



void initSpace(void);

void updateSpace(void);

cpBody* getBall(void);

cpBody* getObst(uint32_t i);

cpBody* getBox(uint32_t i);