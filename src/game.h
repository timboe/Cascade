#pragma once

#include <stdio.h>
#include <stdbool.h> 

#include "pd_api.h"
#include "pdxalloc.h"
#include "constants.h"

int gameLoop(void* data);

int getFrameCount(void);
void resetFrameCount(void);


void setTurretBarrelAngle(const float angle);
float getTurretBarrelAngle(void);

uint16_t getPreviousWaterfall(void);
void resetPreviousWaterfall(void);

int16_t getMinimumY(void);
void setMinimumY(int16_t y);

float getScrollOffset(void);
void setScrollOffset(float set, const bool force);
void modScrollVelocity(const float mod);
float applyScrollEasing(void);

float getParalaxFactorFar(void);
float getParalaxFactorNear(void);

void populateMenuGame(void);
void populateMenuTitlesPlayer(void);
void populateMenuTitles(void);

char* ftos(const float value, const int16_t size, char* dest);

void snprintf_c(char* buf, const uint8_t bufSize, const int n);
