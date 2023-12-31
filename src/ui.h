#pragma once
#include "game.h"
 
enum kGameMode {
  kTitles, 
  kGameWindow,
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

LCDBitmap* getTitleNewGameBitmap(uint8_t _i);

int32_t getScrollOffset(void);

int32_t modScrollOffset(int32_t _mod);

void setScrollOffset(int32_t _set);