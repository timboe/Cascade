#pragma once
#include "game.h"

bool IOGetIsPreloading(void);
float IOGetPreloadingProgress(void);
void IODoUpdatePreloading(void);

void IODoLoadCurrentHole(void);

void IODoSave(void);

void IOResetPlayerSave(const uint16_t player);

uint16_t IOGetWaterfallBackground(const uint16_t level, const uint16_t hole);
uint16_t IOGetWaterfallForeground(const uint16_t level, const uint16_t hole);

uint16_t IOGetCurrentHoleWaterfallBackground(void);
uint16_t IOGetCurrentHoleWaterfallForeground(void);

uint16_t IOGetCurrentPlayer(void);

void IODoPreviousPlayer(void);
void IODoNextPlayer(void);

bool IOGetIsTutorial(void);

uint16_t IOGetCurrentLevel(void);
uint16_t IOGetPreviousLevel(void);
uint16_t IOGetNextLevel(void);

void IODoPreviousLevel(void);
void IODoNextLevel(void);
void IODoGoToNextUnplayedLevel(void);

uint16_t IOGetCurrentHole(void);
uint16_t IOGetPreviousHole(void);
uint16_t IOGetNextHole(void);

void IOGetLevelStatistics(const uint16_t level, uint16_t* score, uint16_t* par);
void IOGetHoleStatistics(const uint16_t level, uint16_t hole, uint16_t* score, uint16_t* par);

void IODoPreviousHole(void);
void IODoNextHole(void);
void IODoNextHoleWithLevelWrap(void);

void IOSetCurrentHoleScore(const uint16_t score);

uint16_t IOGetPar(const uint16_t level, const uint16_t hole);

uint16_t IOGetScore(uint16_t level, uint16_t hole);

uint16_t IOGetCurrentHolePar(void);

enum PegSpecial_t IOGetCurrentHoleSpecial(void);

uint16_t IOGetCurrentHoleHeight(void);

uint16_t IOGetCurrentHoleScore(void);

const char* IOGetCurrentHoleAuthor(void);

const char* IOGetCurrentHoleName(void);

