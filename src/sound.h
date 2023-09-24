#pragma once
#include "game.h"

enum SfxSample {
 kNSFX
};

#define N_FOOTSTEPS 6

void initSound(void);

void updateMusic(bool _isTitle);

void updateMusicVol(void);

void updateSfx(void);

void pauseMusic(void);

void chooseMusic(int8_t _id);

void resumeMusic(void);

void sfx(enum SfxSample _sample);