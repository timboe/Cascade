#pragma once
#include "game.h"
#include "peg.h"

const static uint8_t WF_BG_OFFSET[] = {194};

void initBitmap(void);

void setRoobert10(void);
void setRoobert24(void);
void setGreatVibes24(void);
void setGreatVibes109(void);

LCDFont* getRoobert24(void);
LCDFont* getRoobert10(void);
LCDFont* getGreatVibes24(void);
LCDFont* getGreatVibes109(void);

LCDBitmap* getBitmapWfFg(uint8_t _wf, uint32_t _x, uint32_t _y);
LCDBitmap* getBitmapWfFg_byidx(uint8_t _wf, uint32_t _idx);
LCDBitmap* getBitmapWfBg(uint8_t wf);
LCDBitmap* getBitmapWfFront(void);


LCDBitmap* getSpriteSplash(void);

LCDBitmap* getTitleSelectedBitmap(void);

LCDBitmap* getBitmapTurretBody(void);
LCDBitmap* getBitmapTurretBarrel(void);
LCDBitmap* getBitmapHeader(void); // not used
LCDBitmap* getInfoTopperBitmap(void);
LCDBitmap* getLevelSplashBitmap(void);

LCDBitmap* getBitmapPeg(const struct Peg_t* p);

LCDBitmap* getBitmapBall(void);

LCDBitmap* getBitmapAnimPoot(uint8_t i);

void updateInfoTopperBitmap(void);
void updateLevelSplashBitmap(void);

float sizeToScale(uint8_t size);

#define WF_ID(X, Y) ((WFSHEET_SIZE_X * Y) + X)

