#pragma once
#include "game.h"
#include "chipmunk/chipmunk.h"

void initSpace(void);

void updateSpace(void);

bool ballInPlay(void);

void setBallInPlay(bool bip);

cpBody* getBall(void);

cpShape* getBallShape(void);

cpBody* getObst(uint32_t i);

cpBody* getBox(uint32_t i);

cpSpace* getSpace(void);