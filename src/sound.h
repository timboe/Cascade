#pragma once
#include "game.h"
#include "fsm.h"

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
  kPopSfx1,
  kPopSfx2,
  kPopSfx3,
  kFizzleSfx,
  kBoingSfx1,
  kBoingSfx2,
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
  kBirdSfx1,
  kBirdSfx2,
  kBirdSfx3,
  kBirdSfx4,
  kBirdSfx5,
  kBirdSfx6,
  kBirdSfx7,
  kBirdSfx8,
  kBirdSfx9,
  kBirdSfx10,
  kBirdSfx11,
  kBirdSfx12,
  kBirdSfx13,
  kBirdSfx14,
  kFountainSfx,
  kRelocateTurretSfx,
  kTumblerClickSfx,
  kCrankClickSfx,
  kWompSfx,
  kSuccessSfx,
  kNSFX
};

void soundDoInit(void);

void soundWaterfallDoInit(void);

void soundResetPling(void);

void soundDoMusic(void);

void soundPlayMusic(const uint8_t id);

void soundDoWaterfall(const uint8_t id);

void soundDoVolumes(const enum FSM_t fsm, const enum GameMode_t gm);

void soundDoSfx(enum SfxSample sample);

void soundStopSfx(enum SfxSample sample);

void soundSetDoMusic(const bool doit);

void soundSetDoSfx(const bool doit);

void soundSetDoingExplosion(const bool expOn);

int32_t soundGetSetting(void);

void soundSetSetting(const int32_t setting);