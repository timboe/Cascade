#pragma once
#include "game.h"

enum SfxSample {
  kPlingSfx1,
  kPlingSfx2,
  kPlingSfx3, 
  kPlingSfx4,
  kPlingSfx5,
  kPlingSfx6,
  kPlingSfx7,
  kSplashSfx1,
  kSplashSfx2,
  kSplashSfx3,
  kSplashSfx4,
  kSplashSfx5,
  kPopSfx,
  kExplosionSfx,
  kBallClinkSfx,
  kNSFX
};

void soundDoInit(void);

void soundWaterfallDoInit(void);

void soundResetPling(void);

void soundDoMusic(void);

void soundDoWaterfall(const uint8_t id);

void soundDoSfx(enum SfxSample sample);

void soundSetDoMusic(const bool doit);

void soundSetDoSfx(const bool doit);

void soundSetDoingExplosion(const bool expOn);