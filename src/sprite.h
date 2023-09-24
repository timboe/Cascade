#pragma once
#include "game.h"

void initSprite(void);

void setRoobert10(void);

void setRoobert24(void);

LCDFont* getRoobert24(void);

LCDFont* getRoobert10(void);

// LCDBitmap* getSprite32(uint32_t _x, uint32_t _y);

// LCDBitmap* getSprite32_byidx(uint32_t _idx);

LCDBitmap* getSpriteSplash(void);

LCDBitmap* getTitleSelectedBitmap(void);


// Short-form versions too
//#define SID(X, Y) ((SHEET32_SIZE_X * Y) + X)
//#define SPRITE32_ID(X, Y) ((SHEET32_SIZE_X * Y) + X)

