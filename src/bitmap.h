#pragma once
#include "game.h"

const static uint8_t WF_BG_OFFSET[] = {194};

void initSprite(void);

void setRoobert10(void);

void setRoobert24(void);

LCDFont* getRoobert24(void);

LCDFont* getRoobert10(void);

LCDBitmap* getBitmapWfFg(uint8_t _wf, uint32_t _x, uint32_t _y);
LCDBitmap* getBitmapWfFg_byidx(uint8_t _wf, uint32_t _idx);

LCDBitmap* getBitmapWfBg(uint8_t _wf, uint32_t _x, uint32_t _y);
LCDBitmap* getBitmapWfBg_byidx(uint8_t _wf, uint32_t _idx);

LCDBitmap* getSpriteSplash(void);

LCDBitmap* getTitleSelectedBitmap(void);

LCDBitmap* getBitmapTurretBody(void);

LCDBitmap* getBitmapTurretBarrel(void);

LCDBitmap* getBitmapBall(float _angle);

LCDBitmap* getBitmapBox(float _angle);


#define WF_ID(X, Y) ((WFSHEET_SIZE_X * Y) + X)

