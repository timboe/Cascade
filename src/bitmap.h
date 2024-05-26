#pragma once
#include "game.h"
#include "peg.h"

const static uint8_t WF_BG_OFFSET[MAX_LEVELS] = {0};

void bitmapDoInit(void);

void bitmapSetRoobert10(void);
void bitmapSetRoobert24(void);
void bitmapSetGreatVibes24(void);
void bitmapSetGreatVibes109(void);

LCDFont* bitmapGetRoobert24(void);
LCDFont* bitmapGetRoobert10(void);
LCDFont* bitmapGetGreatVibes24(void);
LCDFont* bitmapGetGreatVibes109(void);

LCDBitmap* bitmapGetWfFg(const uint8_t wf, const uint8_t id);
LCDBitmap* bitmapGetWfBg(const uint8_t wf);
LCDBitmap* bitmapGetWfPond(void);
LCDBitmap* bitmapGetWaterSplash(const uint8_t id);
LCDBitmap* bitmapGetFountain(const uint8_t f, const int id);

LCDBitmap* bitmapGetUseTheCrank(void);
LCDBitmap* bitmapGetTitleHeaderImage(void);
LCDBitmap* bitmapGetNumeral(const int8_t n);
LCDBitmap* bitmapGetStencilWipe(const int8_t n);
LCDBitmap* bitmapGetLevelPreview(const uint16_t level, const uint16_t hole, int16_t offset);

LCDBitmap* bitmapGetTutorialCrankRotate(const uint8_t id);
LCDBitmap* bitmapGetTutorialCrankAngle(const uint8_t id);
LCDBitmap* bitmapGetTutorialButton(const uint8_t id);
LCDBitmap* bitmapGetTutorialDPad(const uint8_t id);
LCDBitmap* bitmapGetTutorialArrows(const uint8_t id);

LCDBitmap* bitmapGetTitlePlayer(void);
LCDBitmap* bitmapGetTitleLevel(void);
LCDBitmap* bitmapGetTitleLevelStats(void);
LCDBitmap* bitmapGetTitleHole(void);
LCDBitmap* bitmapGetTitleHoleStatsA(void);
LCDBitmap* bitmapGetTitleHoleStatsB(void);
LCDBitmap* bitmapGetTitleHoleAuthor(void);
LCDBitmap* bitmapGetTitleHoleName(void);
LCDBitmap* bitmapGetTitleHoleTutorial(void);
LCDBitmap* bitmapGetTitleScoreCard(void);

LCDBitmap* bitmapGetTurretBody(void);
LCDBitmap* bitmapGetTurretBarrel(void);
LCDBitmap* bitmapGetGameInfoTopper(void);
LCDBitmap* bitmapGetLevelTitle(void);
LCDBitmap* bitmapGetScoreHistogram(void);
LCDBitmap* bitmapGetChevron(const uint8_t id);

LCDBitmap* bitmapGetPeg(const struct Peg_t* p);
LCDBitmap* bitmapGetPegPop(const struct Peg_t* p);
LCDBitmap* bitmapGetMarble(void);
LCDBitmap* bitmapGetMarbleFirePoot(const uint8_t i);

LCDBitmap* bitmapGetEndBlast(const uint8_t id, const uint8_t frame);

LCDBitmap* bitmapGetSpecialBlast(const uint8_t id);

LCDBitmap* bitmapGetSpecial(const enum PegSpecial_t special);

void bitmapDoUpdateGameInfoTopper(void);
void bitmapDoUpdateLevelTitle(void);
void bitmapDoUpdateLevelStatsBitmap(void);
void bitmapDoUpdateHoleStatsBitmap(void);
void bitmapDoUpdateScoreHistogram(void);
void bitmapDoUpdateScoreCard(void);

float bitmapSizeToScale(const uint8_t size);

void bitmapDoPreloadA(void);
void bitmapDoPreloadB(const uint8_t anim);
void bitmapDoPreloadC(void);
void bitmapDoPreloadD(void);
void bitmapDoPreloadE(void);
void bitmapDoPreloadF(void);
void bitmapDoPreloadG(const uint8_t size);
void bitmapDoPreloadH(const uint8_t size);
void bitmapDoPreloadH2(const uint8_t size);
void bitmapDoPreloadI(void);
void bitmapDoPreloadJ(void);
void bitmapDoPreloadK(void);
void bitmapDoPreloadL(const uint8_t star);
