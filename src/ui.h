#pragma once
#include "game.h"
 
enum kGameMode {
  kTitles, 
  kGameWindow,
  kNGameModes
};

enum kFSM {
  kGameFSM_,
  kNFSMModes
};

void updateUI(int fc);

void updateUITitles(int fc);

void modTitleCursor(bool _increment);

uint16_t getTitleCursorSelected(void);

void snprintf_c(char* _buf, uint8_t _bufSize, int _n);

void setGameMode(enum kGameMode _mode);

enum kGameMode getGameMode(void);

void initiUI(void);

void resetUI(void);

LCDBitmap* getTitleNewGameBitmap(uint8_t _i);

float getScrollOffset(void);

void setScrollOffset(float set);

void setScrollToTop(bool stt);

void modScrollVelocity(float mod);

void applyScrollEasing(void);

void activateBallEndSweep(void);

void doBallEndSweep(void);

void renderBallEndSweep(void);

float getParalaxFactorFar(void);

float getParalaxFactorNear(void);

int16_t getMinimumY(void);

void setMinimumY(int16_t y);