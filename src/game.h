#pragma once

#include <stdio.h>
#include <stdbool.h> 

#include "pd_api.h"
#include "pdxalloc.h"
#include "constants.h"

int gameLoop(void* data);

int32_t gameGetFrameCount(void);
void gameDoResetFrameCount(void);

void gameSetTurretBarrelAngle(const float angle);
float gameGetTurretBarrelAngle(void);

uint16_t gameGetPreviousWaterfallFg(void);
uint16_t gameGetPreviousWaterfallBg(void);
void gameDoResetPreviousWaterfall(void);

int16_t gameGetMinimumY(void);
void gameSetMinimumY(int16_t y);

float gameGetYOffset(void);
void gameSetYOffset(float set, const bool force);
void gameModYVelocity(const float mod);
float gameDoApplyYEasing(void);

void gameSetXOffset(const float set);
float gameGetXOffset(void);

float gameGetParalaxFactorFar(const bool hard);
float gameGetParalaxFactorNear(const bool hard);

float gameGetParalaxFactorNearForY(const bool hard, const float y);

void gameDoPopulateMenuGame(void);
void gameDoPopulateMenuTitlesPlayer(void);
void gameDoPopulateMenuTitles(void);
void gameMenuStateSafetyReset(void);