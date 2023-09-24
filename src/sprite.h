#pragma once
#include "game.h"

void initSprite(void);

void setRoobert10(void);

void setRoobert24(void);

LCDFont* getRoobert24(void);

LCDFont* getRoobert10(void);

LCDBitmap* getBitmapWf(uint32_t _x, uint32_t _y);

LCDBitmap* getBitmapWf_byidx(uint32_t _idx);

LCDBitmap* getSpriteSplash(void);

LCDBitmap* getTitleSelectedBitmap(void);

LCDBitmap* getBitmapTurretBody(void);

LCDBitmap* getBitmapTurretBarrel(void);

void setBarrelAngle(float _angle);


#define WF_ID(X, Y) ((WFSHEET_SIZE_X * Y) + X)

