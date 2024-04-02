#pragma once
#include "game.h"
 
enum kGameMode {
  kTitles, 
  kGameWindow,
  kNGameModes
};

enum kFSM {
  kTitlesFSM_DisplayTitles,
  kTitlesFSM_TitlesToSplash,
  kGameFSM_DisplaySplash,
  kGameFSM_SplashToStart,
  kGameFSM_AimMode,
  kGameFSM_BallInPlay,
  kGameFSM_BallStuck,
  kGameFSM_CloseUp,
  kGameFSM_BallGutter,
  kGameFSM_GuttetToTurret,
  kGameFSM_GutterToScores,
  kGameFSM_ScoresToTitle,
  kGameFSM_ScoresToSplash,
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

void setScrollOffset(float set, bool force);

void modScrollVelocity(float mod);

float applyScrollEasing(void);

void renderDebug(void);

float getParalaxFactorFar(void);

float getParalaxFactorNear(void);

int16_t getMinimumY(void);

void setMinimumY(int16_t y);

extern float m_popLevel; // TODO remove me