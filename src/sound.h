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
  kPlingSfx8,
  kPlingSfx9,
  kPlingSfx10,
  kPlingSfx11,
  kPlingSfx12,
  kPlingSfx13,
  kPlingSfx14,
  kPlingSfx15,
  kPlingSfx16,
  kDingSfx1,
  kDingSfx2,
  kDingSfx3,
  kDingSfx4,
  kDingSfx5,
  kDingSfx6,
  kDingSfx7,
  kDingSfx8,
  kDingSfx9,
  kDingSfx10,
  kDingSfx11,
  kDingSfx12,
  kDingSfx13,
  kDingSfx14,
  kDingSfx15,
  kDingSfx16,
  kSplashSfx1,
  kSplashSfx2,
  kSplashSfx3,
  kSplashSfx4,
  kSplashSfx5,
  kPopSfx,
  kExplosionSfx,
  kBallClinkSfx,
  kDrumRollSfx1,
  kDrumRollSfx2,
  kTeleportSfx,
  kSplitSfx,
  kChargeSfx,
  kPootSfx,
  kWhooshSfx1,
  kWhooshSfx2,
  kWhooshSfx3,
  kWhooshSfx4,
  kRelocateTurretSfx,
  kNSFX
};

void soundDoInit(void);

void soundWaterfallDoInit(void);

void soundResetPling(void);

void soundDoMusic(void);

void soundPlayMusic(const uint8_t id);

void soundDoWaterfall(const uint8_t id);

void soundDoSfx(enum SfxSample sample);

void soundStopSfx(enum SfxSample sample);

void soundSetDoMusic(const bool doit);

void soundSetDoSfx(const bool doit);

void soundSetDoingExplosion(const bool expOn);