#pragma once
#include "game.h"
#include "peg.h"

const static uint8_t WF_BG_OFFSET[] = {194};

void bitmapDoInit(void);

void bitmapSetRoobert10(void);
void bitmapSetRoobert24(void);
void bitmapSetGreatVibes24(void);
void bitmapSetGreatVibes109(void);

LCDFont* bitmapGetRoobert24(void);
LCDFont* bitmapGetRoobert10(void);
LCDFont* bitmapGetGreatVibes24(void);
LCDFont* bitmapGetGreatVibes109(void);

LCDBitmap* bitmapGetWfFg(const uint8_t wf, const uint32_t x, const uint32_t y);
LCDBitmap* bitmapGetWfFg_byidx(const uint8_t wf, const uint32_t idx);
LCDBitmap* bitmapGetWfBg(const uint8_t wf);
LCDBitmap* bitmapGetWfPond(void);
LCDBitmap* bitmapGetWaterSplash(const uint8_t id);

LCDBitmap* bitmapGetUseTheCrank(void);
LCDBitmap* bitmapGetTitleSplash(void);
LCDBitmap* bitmapGetNumeral(const int8_t n);
LCDBitmap* bitmapGetStencilWipe(const int8_t n);
LCDBitmap* bitmapGetDither(void);
LCDBitmap* bitmapGetLevelPreview(const uint16_t level, const uint16_t hole, int16_t offset);

LCDBitmap* bitmapGetTitlePlayer(void);
LCDBitmap* bitmapGetTitleLevel(void);
LCDBitmap* bitmapGetTitleLevelStats(void);
LCDBitmap* bitmapGetTitleHole(void);
LCDBitmap* bitmapGetTitleHoleStatsA(void);
LCDBitmap* bitmapGetTitleHoleStatsB(void);
LCDBitmap* bitmapGetTitleHoleCreator(void);
LCDBitmap* bitmapGetTitleHoleTutorial(void);

LCDBitmap* bitmapGetTurretBody(void);
LCDBitmap* bitmapGetTurretBarrel(void);
LCDBitmap* bitmapGetGameInfoTopper(void);
LCDBitmap* bitmapGetLevelSplash(void);
LCDBitmap* BitmapGetScoreHistogram(void);

LCDBitmap* bitmapGetPeg(const struct Peg_t* p);
LCDBitmap* bitmapGetBall(void);
LCDBitmap* bitmapGetBallFirePoot(const uint8_t i);

LCDBitmap* bitmapGetStar(const uint8_t type, const uint8_t angle);

LCDBitmap* bitmapGetBlast(const uint8_t id);

LCDBitmap* bitmapGetSpecial(const enum PegSpecial_t special);

void bitmapDoUpdateGameInfoTopper(void);
void bitmapDoUpdateLevelSplash(void);
void bitmapDoUpdateLevelStatsBitmap(void);
void bitmapDoUpdateHoleStatsBitmap(void);
void bitmapDoUpdateScoreHistogram(void);

float bitmapSizeToScale(const uint8_t size);

void bitmapDoPreloadA(void);
void bitmapDoPreloadB(const uint8_t anim);
void bitmapDoPreloadC(void);
void bitmapDoPreloadD(void);
void bitmapDoPreloadE(void);
void bitmapDoPreloadF(void);
void bitmapDoPreloadG(const uint8_t size);
void bitmapDoPreloadH(const uint8_t size);
void bitmapDoPreloadI(void);
void bitmapDoPreloadJ(void);
void bitmapDoPreloadK(void);
void bitmapDoPreloadL(const uint8_t star);


#define WF_ID(X, Y) ((WFSHEET_SIZE_X * Y) + X)

