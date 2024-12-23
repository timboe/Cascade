#pragma once
#include "game.h"

enum SfxSample {
  kPling1,
  kPling2,
  kPling3, 
  kPling4,
  kPling5,
  kPling6,
  kPling7,
  kSplash1,
  kSplash2,
  kSplash3,
  kSplash4,
  kSplash5,
  kNSFX
};

void soundDoInit(void);

void soundWaterfallDoInit(void);

void soundResetPling(void);

void soundDoMusic(void);

void soundDoWaterfall(const uint8_t id);

void soundDoSfx(enum SfxSample sample);

void soundSetDoMusic(const bool doit);

void soundSetDoSfx(const bool doit) ;