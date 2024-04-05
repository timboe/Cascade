#pragma once
#include "game.h"
#include "peg.h"

void initBoard(void);

void randomiseBoard(void);

void updateBoard(void);

void renderBoard(void);

void clearBoard(void);

void popBoard(float y);

bool popRandom(void);

struct Peg_t* getPeg(uint16_t i);

int16_t requiredPegsInPlay(void);

void requiredPegHit(void);