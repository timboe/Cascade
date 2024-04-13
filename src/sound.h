#pragma once
#include "game.h"

enum SfxSample {
 kNSFX
};

void soundDoInit(void);

void musicDoInit(void);

void soundDoUpdate(void);

void soundDoChooseMusic(const int8_t id);

void soundDoSfx(const enum SfxSample sample);