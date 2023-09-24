#pragma once
#include "game.h"
 
enum kGameMode {
  kTitles, 
  kNGameModes
};

void updateUI(int _fc);

void updateUITitles(int _fc);

void modTitleCursor(bool _increment);

uint16_t getTitleCursorSelected(void);

void snprintf_c(char* _buf, uint8_t _bufSize, int _n);

void setGameMode(enum kGameMode _mode);

enum kGameMode getGameMode(void);

void initiUI(void);

void resetUI(void);

