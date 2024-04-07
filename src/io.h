#pragma once
#include "game.h"

void scanLevels(void);

void loadCurrentHole(void);

void doSave(void);

void resetPlayerSave(uint16_t player);

uint16_t getWaterfallBackground(uint16_t level, uint16_t hole);

uint16_t getWaterfallForeground(uint16_t level, uint16_t hole);

uint16_t getCurrentPlayer(void);

void doPreviousPlayer(void);

void doNextPlayer(void);

uint16_t getCurrentLevel(void);

uint16_t getPreviousLevel(void);

uint16_t getNextLevel(void);

void doPreviousLevel(void);

void doNextLevel(void);

void goToNextUnplayedLevel(void);

uint16_t getCurrentHole(void);

uint16_t getPreviousHole(void);

uint16_t getNextHole(void);

void getLevelStatistics(uint16_t level, uint16_t* score, uint16_t* par);

void getHoleStatistics(uint16_t level, uint16_t hole, uint16_t* score, uint16_t* par);

void doPreviousHole(void);

void doNextHole(void);

void setHoleScore(uint16_t score);

uint16_t getPar(uint16_t level, uint16_t hole);

uint16_t getCurrentHolePar(void);

uint16_t getScore(uint16_t level, uint16_t hole);

uint16_t getCurrentHoleScore(void);
