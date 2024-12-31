#include "bitmap.h"
#include "input.h"
#include "util.h"
#include "io.h"
#include "peg.h"

enum RenderColor_t {
  kRenderColorWhite,
  kRenderColorGrey,
  kRenderColorHatched
};

const static LCDPattern kGreyPattern = {
  0,0,0,0,0,0,0,0,
  0b11001100,
  0b11001100,
  0b11001100,
  0b11001100,
  0b11001100,
  0b11001100,
  0b11001100,
  0b11001100};

const static LCDPattern kHatchedPattern = {
  0,0,0,0,0,0,0,0,
  0b11110000,
  0b11110000,
  0b11110000,
  0b11110000,
  0b00001111,
  0b00001111,
  0b00001111,
  0b00001111};

// Titiles
LCDBitmap* m_headerImage;

LCDBitmap* m_playerBitmap;
LCDBitmap* m_levelBitmap;
LCDBitmap* m_levelStatsBitmap;
LCDBitmap* m_holeBitmap;
LCDBitmap* m_holeStatsBitmap[2];
LCDBitmap* m_holeAuthorBitmap;
LCDBitmap* m_holeNameBitmap;
LCDBitmap* m_holeTutorialBitmap;

LCDBitmap* m_scoreCardBitmap;

LCDBitmap* m_useTheCrankBitmap;

LCDBitmap* m_ditherBitmap;

LCDBitmap* m_previewBitmap[MAX_LEVELS][MAX_HOLES] = {0};
LCDBitmap* m_previewBitmapWindow;
LCDBitmap* m_previewBitmapStencil;

LCDBitmap* m_stencilWipeBitmap[STENCIL_WIPE_N];

// Game

LCDBitmap* m_turretBarrel[8][256];
LCDBitmapTable* m_turretBarrelTabel;
LCDBitmap* m_infoTopperBitmap;
LCDBitmap* m_levelTitleBitmap;

LCDBitmap* m_numeralBitmap[10];
LCDBitmap* m_cardBitmap;

LCDBitmap* m_scoreHistogram = NULL;

LCDBitmap* m_pootAnimation[TURRET_RADIUS];

LCDBitmap* m_marbleBitmap;
LCDBitmap* m_ballBitmap[2][MAX_PEG_SIZE];
LCDBitmap* m_rectBitmap[2][MAX_PEG_SIZE][128];
LCDBitmap* m_triBitmap[2][MAX_PEG_SIZE][256];
LCDBitmap* m_hexBitmap[MAX_PEG_SIZE][128];

LCDBitmap* m_specialTextBitmap[(uint8_t)kNPegSpecial];

LCDBitmap* m_wfPond[POND_WATER_TILES][POND_WATER_FRAMES];
LCDBitmap* m_wfBg[N_WF_BG];
LCDBitmapTable* m_sheetWfFg[128]; // N_WF + room for specials
LCDBitmapTable* m_waterSplashTable[N_SPLASHES];
LCDBitmapTable* m_fountainTable[N_FOUNTAINS];
LCDBitmapTable* m_chevronTable;
LCDBitmapTable* m_pegPopTable[MAX_POPS];

LCDBitmapTable* m_specialBlastTable;
LCDBitmapTable* m_endBlastTable[N_END_BLASTS];
LCDBitmapTable* m_tutorialCrankRotateTable;
LCDBitmapTable* m_tutorialCrankAngleTable;
LCDBitmapTable* m_tutorialButtonTable;
LCDBitmapTable* m_tutorialDPadTable;
LCDBitmapTable* m_tutorialArrowsTable;
LCDBitmapTable* m_fwBkwIconTable;
LCDBitmapTable* m_turretBodyTable;

LCDFont* m_fontRoobert24;
LCDFont* m_fontRoobert10;
LCDFont* m_fontGreatvibes24;
LCDFont* m_fontGreatvibes109;

LCDBitmap* bitmapDoLoadImageAtPath(const char* path);

LCDBitmapTable* bitmapDoLoadImageTableAtPath(const char* path);

LCDFont* bitmapDoLoadFontAtPath(const char* path);

void bitmapDoDrawOutlineText(const char text[], const uint16_t textSize, int16_t x, int16_t y, const uint16_t outlineSize);

void bitmapDoDrawRotatedRect(const float x, const float y, const float w2, const float h2, const int16_t iAngle, const enum RenderColor_t rc);

void bitmapDoDrawRotatedPoly(const uint8_t corners, const float x, const float y, const float w2, const float h2, const int16_t iAngle, const enum RenderColor_t rc);

/// ///

LCDBitmap* bitmapDoLoadImageAtPath(const char* path) {
  const char* outErr = NULL;
  LCDBitmap* img = pd->graphics->loadBitmap(path, &outErr);
  if (outErr != NULL) {
    pd->system->error("Error loading image at path '%s': %s", path, outErr);
  }
  return img;
}

LCDBitmapTable* bitmapDoLoadImageTableAtPath(const char* path) {
  const char* outErr = NULL;
  LCDBitmapTable* table = pd->graphics->loadBitmapTable(path, &outErr);
  if (outErr != NULL) {
    pd->system->error("Error loading image table at path '%s': %s", path, outErr);
  }
  return table;
}

LCDFont* bitmapDoLoadFontAtPath(const char* path) {
  const char* outErr = NULL;
  LCDFont* f = pd->graphics->loadFont(path, &outErr);
  if (outErr != NULL) {
    pd->system->error("Error loading font at path '%s': %s", path, outErr);
  }
  return f;
}

void bitmapDoDrawOutlineText(const char text[], const uint16_t textSize, int16_t x, int16_t y, const uint16_t outlineSize) {
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  for (int i = 0; i < 8; ++i) {
    switch (i) {
      case 0: y += outlineSize; break;
      case 1: x -= outlineSize; break;
      case 2: y -= outlineSize; break;
      case 3: y -= outlineSize; break;
      case 4: x += outlineSize; break;
      case 5: x += outlineSize; break;
      case 6: y += outlineSize; break;
      case 7: y += outlineSize; break;
    }
    pd->graphics->drawText(text, 128, kUTF8Encoding, x, y);
  }
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  y -= outlineSize;
  x -= outlineSize;
  pd->graphics->drawText(text, 128, kUTF8Encoding, x, y);
}

void bitmapDoUpdateLevelTitle(void) {
  pd->graphics->clearBitmap(m_levelTitleBitmap, kColorBlack);
  pd->graphics->pushContext(m_levelTitleBitmap);
  char text[128];
  
  if (IOIsCredits()) {
    pd->graphics->setDrawMode(kDrawModeInverted);
    pd->graphics->drawBitmap(m_headerImage, 0, 0, kBitmapUnflipped);
    snprintf(text, 128, "BY TIM MARTIN");
    const int32_t w2 = pd->graphics->getTextWidth(bitmapGetRoobert24(), text, 128, kUTF8Encoding, 0);
    bitmapSetRoobert24(); 
    pd->graphics->setDrawMode(kDrawModeFillWhite);
    const uint16_t yC = 128+64;
    pd->graphics->drawText(text, 128, kUTF8Encoding, DEVICE_PIX_X/2 - w2/2, yC);
    pd->graphics->popContext();
    return;
  }

  const bool hasTitle = (strlen(IOGetCurrentHoleName()) > 0);
  uint16_t yA = 0;
  uint16_t yB = 0;
  uint16_t yC = 128+32;
  if (hasTitle) {
    yA = 16;
    yB = 32+8;
    yC = 128+64;
    snprintf(text, 128, "%s", IOGetCurrentHoleName());
    const int32_t w0 = pd->graphics->getTextWidth(bitmapGetRoobert24(), text, 128, kUTF8Encoding, 0);
    bitmapSetRoobert24();
    pd->graphics->setDrawMode(kDrawModeFillWhite);
    pd->graphics->drawText(text, 128, kUTF8Encoding, DEVICE_PIX_X/2 - w0/2, yA);
  }
  //
  snprintf(text, 128, "%i~%i", (int)IOGetCurrentLevel() + 1, (int)IOGetCurrentHole() + 1);
  const int32_t w1 = pd->graphics->getTextWidth(bitmapGetGreatVibes109(), text, 128, kUTF8Encoding, 0);
  bitmapSetGreatVibes109();
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  bitmapDoDrawOutlineText(text, 128, DEVICE_PIX_X/2 - w1/2, yB, 4);
  //
  snprintf(text, 128, "PAR %i", (int)IOGetCurrentHolePar());
  const int32_t w2 = pd->graphics->getTextWidth(bitmapGetRoobert24(), text, 128, kUTF8Encoding, 0);
  bitmapSetRoobert24(); 
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  pd->graphics->drawText(text, 128, kUTF8Encoding, DEVICE_PIX_X/2 - w2/2, yC);
  pd->graphics->popContext();
}

void bitmapDoUpdateGameInfoTopper(void) {
  pd->graphics->clearBitmap(m_infoTopperBitmap, kColorBlack);
  if (IOIsCredits()) { return; }
  pd->graphics->pushContext(m_infoTopperBitmap);
  char text[128];
  snprintf(text, 128, "%i~%i", (int)IOGetCurrentLevel() + 1, (int)IOGetCurrentHole() + 1);
  bitmapSetGreatVibes24();
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  pd->graphics->drawText(text, 128, kUTF8Encoding, 0, 8);
  bitmapSetRoobert24();
  if (strlen(IOGetCurrentHoleAuthor()) > 0){
    snprintf(text, 128, "BY %s", IOGetCurrentHoleAuthor());
    const int32_t width = pd->graphics->getTextWidth(bitmapGetRoobert24(), text, 128, kUTF8Encoding, 0);
    pd->graphics->drawText(text, 128, kUTF8Encoding, DEVICE_PIX_X - width, 2);
  }
  pd->graphics->popContext();
}

LCDBitmap* bitmapGetMarbleFirePoot(const uint8_t i) { return m_pootAnimation[i]; }

LCDBitmap* bitmapGetUseTheCrank(void) { return m_useTheCrankBitmap; }

LCDBitmap* bitmapGetTitleHeaderImage(void) { return m_headerImage; }

LCDBitmap* bitmapGetWfPond(const uint8_t n, const int32_t fc) { return m_wfPond[n % POND_WATER_TILES][fc/2 % POND_WATER_FRAMES]; }

LCDBitmap* bitmapGetWfBg(const uint8_t wf) { return m_wfBg[wf]; }

LCDBitmap* bitmapGetWaterSplash(const uint8_t id) { 
  return pd->graphics->getTableBitmap(m_waterSplashTable[0], id);
}

LCDBitmap* bitmapGetFountain(const uint8_t f, const int id) { 
  return pd->graphics->getTableBitmap(m_fountainTable[f % N_FOUNTAINS], id % FOUNTAIN_FRAMES);
}

LCDBitmap* bitmapGetWfFg(const uint8_t wf, const uint8_t id) {
  return pd->graphics->getTableBitmap(m_sheetWfFg[wf], id);
}

LCDBitmap* bitmapGetSpecialBlast(const uint8_t id) {
  return pd->graphics->getTableBitmap(m_specialBlastTable, id % 9);
}

LCDBitmap* bitmapGetEndBlast(const uint8_t id, const uint8_t frame) {
  return pd->graphics->getTableBitmap(m_endBlastTable[id % N_END_BLASTS], frame % END_BLAST_FRAMES);
}

LCDBitmap* bitmapGetTutorialCrankRotate(const uint8_t id) {
  return pd->graphics->getTableBitmap(m_tutorialCrankRotateTable, id % 8);
}

LCDBitmap* bitmapGetTutorialCrankAngle(const uint8_t id) {
  return pd->graphics->getTableBitmap(m_tutorialCrankAngleTable, id % 5);
}

LCDBitmap* bitmapGetTutorialButton(const uint8_t id) {
  return pd->graphics->getTableBitmap(m_tutorialButtonTable, id % 4);
}

LCDBitmap* bitmapGetTutorialDPad(const uint8_t id) {
  return pd->graphics->getTableBitmap(m_tutorialDPadTable, id % 3);
}

LCDBitmap* bitmapGetTutorialArrows(const uint8_t id) {
  return pd->graphics->getTableBitmap(m_tutorialArrowsTable, id % 2);
}

LCDBitmap* bitmapGetFwBkwIcon(const int8_t id) {
  return pd->graphics->getTableBitmap(m_fwBkwIconTable, id % 3);
}

LCDBitmap* bitmapGetTurretBody(const int8_t id) {
  return pd->graphics->getTableBitmap(m_turretBodyTable, id % kNPegSpecial);
}

LCDBitmap* bitmapGetTitlePlayer(void) { return m_playerBitmap; }

LCDBitmap* bitmapGetTitleLevel(void) { return m_levelBitmap; }

LCDBitmap* bitmapGetTitleLevelStats(void) { return m_levelStatsBitmap; }

LCDBitmap* bitmapGetTitleHole(void) { return m_holeBitmap; }

LCDBitmap* bitmapGetTitleHoleStatsA(void) { return m_holeStatsBitmap[0]; }

LCDBitmap* bitmapGetTitleHoleStatsB(void) { return m_holeStatsBitmap[1]; }

LCDBitmap* bitmapGetTitleHoleAuthor(void) { return m_holeAuthorBitmap; }

LCDBitmap* bitmapGetTitleHoleName(void) { return m_holeNameBitmap; }

LCDBitmap* bitmapGetTitleHoleTutorial(void) { return m_holeTutorialBitmap; }

LCDBitmap* bitmapGetTitleScoreCard(void) { return m_scoreCardBitmap; }

LCDBitmap* bitmapGetLevelPreview(const uint16_t level, const uint16_t hole, int16_t offset) {
  pd->graphics->clearBitmap(m_previewBitmapWindow, kColorClear);
  if (!m_previewBitmap[level][hole]) { return m_previewBitmapWindow; }
  pd->graphics->pushContext(m_previewBitmapWindow);
  pd->graphics->setStencilImage(m_previewBitmapStencil, 0);
  pd->graphics->setDrawMode(kDrawModeInverted);
  if (IOGetCurrentHoleHeight() <= DEVICE_PIX_Y*2) {
    offset = (DEVICE_PIX_Y - (IOGetCurrentHoleHeight()/2)) / 2;
    pd->graphics->drawBitmap(m_previewBitmap[level][hole], 0, offset, kBitmapUnflipped);
  } else {
    offset = offset % (IOGetCurrentHoleHeight()/2);
    pd->graphics->drawBitmap(m_previewBitmap[level][hole], 0, -offset, kBitmapUnflipped);
    pd->graphics->drawBitmap(m_previewBitmap[level][hole], 0, -offset + (IOGetCurrentHoleHeight()/2), kBitmapUnflipped);
  }
  pd->graphics->setDrawMode(kDrawModeCopy);
  pd->graphics->setStencilImage(NULL, 0);
  pd->graphics->popContext();
  return m_previewBitmapWindow;
}

LCDBitmap* bitmapGetPeg(const struct Peg_t* p) {
  switch (p->shape) {
    case kPegShapeHex:  return m_hexBitmap[p->size % MAX_PEG_SIZE][p->iAngle % 128];
    case kPegShapeBall: return m_ballBitmap[p->type == 0 ? 0 : 1][p->size % MAX_PEG_SIZE];
    case kPegShapeRect: return m_rectBitmap[p->type == 0 ? 0 : 1][p->size % MAX_PEG_SIZE][p->iAngle % 128]; // Symmetry
    case kPegShapeTri: return m_triBitmap[p->type == 0 ? 0 : 1][p->size % MAX_PEG_SIZE][p->iAngle % 256]; // Symmetry
    default: return NULL;
  }
  return NULL;
}

LCDBitmap* bitmapGetPegPop(const struct Peg_t* p) {
  return pd->graphics->getTableBitmap(m_pegPopTable[p->popAnim], p->popFrame % POP_ANIM_FRAMES);
}

LCDBitmap* bitmapGetMarble(void) { return m_marbleBitmap; }

LCDBitmap* bitmapGetNumeral(const int8_t n) { return m_numeralBitmap[n % 10]; }

LCDBitmap* bitmapGetStencilWipe(const int8_t n) { return m_stencilWipeBitmap[n % STENCIL_WIPE_N]; }

LCDBitmap* bitmapGetGameInfoTopper(void) { return m_infoTopperBitmap; }

LCDBitmap* bitmapGetLevelTitle(void) { return m_levelTitleBitmap; }

LCDBitmap* bitmapGetTurretBarrel(void) {
  return m_turretBarrel[(gameGetFrameCount() % 32) / 4][ angToByte(gameGetTurretBarrelAngle()) ];
}

LCDBitmap* bitmapGetSpecial(const enum PegSpecial_t special) { return m_specialTextBitmap[(uint8_t)special]; }

void bitmapSetRoobert10(void) { pd->graphics->setFont(m_fontRoobert10); }

void bitmapSetRoobert24(void) { pd->graphics->setFont(m_fontRoobert24); }

void bitmapSetGreatVibes24(void) { pd->graphics->setFont(m_fontGreatvibes24); }

void bitmapSetGreatVibes109(void) { pd->graphics->setFont(m_fontGreatvibes109); }

LCDFont* bitmapGetRoobert10(void) { return m_fontRoobert10; }

LCDFont* bitmapGetRoobert24(void) { return m_fontRoobert24; }

LCDFont* bitmapGetGreatVibes24(void) { return m_fontGreatvibes24; }

LCDFont* bitmapGetGreatVibes109(void) { return m_fontGreatvibes109; }

float bitmapSizeToScale(uint8_t size) {
  switch (size) {
    case 0: return 1.25f;
    case 1: return 1.75f;
    case 2: return 2.5f;
    default: pd->system->error("Error bitmapSizeToScale called with unknown size %i", size);
  }
  return 1.0f;
}

LCDBitmap* bitmapGetScoreHistogram(void) { return m_scoreHistogram; }

LCDBitmap* bitmapGetChevron(const uint8_t id) {
  return pd->graphics->getTableBitmap(m_chevronTable, id % 3);
}

// Can't currently use bitmapDoDrawRotatedPoly here as the points are not evenly spaced around the unit circle
void bitmapDoDrawRotatedRect(const float x, const float y, const float w2, const float h2, const int16_t iAngle, const enum RenderColor_t rc) {
    const float angleRad = (M_PIf / 128.0f) * iAngle;
    const float ca = cosf(angleRad);
    const float sa = sinf(angleRad);
    int points[8] = {
      -w2*ca - -h2*sa + x,
      -w2*sa + -h2*ca + y,
      //
      -w2*ca - h2*sa + x,
      -w2*sa + h2*ca + y,
      //
      w2*ca - h2*sa + x,
      w2*sa + h2*ca + y,
      //
      w2*ca - -h2*sa + x,
      w2*sa + -h2*ca + y
    };
    if (rc == kRenderColorGrey) {
      pd->graphics->fillPolygon(4, points, kColorWhite, kPolygonFillNonZero);
      pd->graphics->fillPolygon(4, points, (uintptr_t)kGreyPattern, kPolygonFillNonZero);
    } else {
      pd->graphics->fillPolygon(4, points, kColorWhite, kPolygonFillNonZero);
    }
    pd->graphics->drawLine(points[0], points[1], points[2], points[3], 2, kColorBlack);
    pd->graphics->drawLine(points[2], points[3], points[4], points[5], 2, kColorBlack);
    pd->graphics->drawLine(points[4], points[5], points[6], points[7], 2, kColorBlack);
    pd->graphics->drawLine(points[6], points[7], points[0], points[1], 2, kColorBlack);
}

void bitmapDoDrawRotatedPoly(const uint8_t corners, const float x, const float y, const float w2, const float h2, const int16_t iAngle, const enum RenderColor_t rc) {
    const float angleOff = (M_PIf / 128.0f) * iAngle;
    int points[128] = {0};
    const float angleAdvance = M_2PIf / (float)corners;
    for (uint8_t p = 0; p < corners; ++p) {
      const float angle = (angleAdvance * p) + angleOff;
      points[2*p + 0] = x + (w2 * cosf(angle));
      points[2*p + 1] = y + (h2 * sinf(angle));
    }
    if (rc == kRenderColorGrey)  {
      pd->graphics->fillPolygon(corners, points, kColorWhite, kPolygonFillNonZero);
      pd->graphics->fillPolygon(corners, points, (uintptr_t)kGreyPattern, kPolygonFillNonZero);
    } else if (rc == kRenderColorWhite) {
      pd->graphics->fillPolygon(corners, points, kColorWhite, kPolygonFillNonZero);
    } else if (rc == kRenderColorHatched) {
      pd->graphics->fillPolygon(corners, points, kColorWhite, kPolygonFillNonZero);
      pd->graphics->fillPolygon(corners, points, (uintptr_t)kHatchedPattern, kPolygonFillNonZero);
    }
    for (uint8_t p = 0; p < corners; ++p) {
      pd->graphics->drawLine(
        points[(2*p + 0) % (2*corners)],
        points[(2*p + 1) % (2*corners)],
        points[(2*p + 2) % (2*corners)],
        points[(2*p + 3) % (2*corners)], 2, kColorBlack);
    }
  }

void bitmapDoUpdateScoreHistogram(void) {
  pd->graphics->clearBitmap(m_scoreHistogram, kColorBlack);
  pd->graphics->pushContext(m_scoreHistogram);
  const int16_t maxHistoBalls = 12;
  const int16_t histoHeight = maxHistoBalls*2*BALL_RADIUS;
  const int16_t histoWidth = MAX_HOLES*3*BALL_RADIUS;
  const int16_t tick = 4;
  int16_t balls[MAX_HOLES] = {0};
  int16_t par[MAX_HOLES] = {0};
  for (int hole = 0; hole < MAX_HOLES;  ++hole) {
    const int16_t score = IOGetScore(IOGetCurrentLevel(), hole);;
    balls[hole] = score > maxHistoBalls ? maxHistoBalls : score;
    par[hole] = IOGetPar(IOGetCurrentLevel(), hole);
  }
  // This will be animated in instead
  balls[IOGetCurrentHole()] = 0;

  for (int i = 0; i < MAX_HOLES; ++i) {
    pd->graphics->fillRect(
      BUF + i*3*BALL_RADIUS, BUF + (maxHistoBalls - par[i])*2*BALL_RADIUS,
      3*BALL_RADIUS,         par[i]*2*BALL_RADIUS,
      kColorWhite);
  }
  pd->graphics->fillRect(0, 0, DEVICE_PIX_X, DEVICE_PIX_Y, (uintptr_t)kGreyPattern);

  pd->graphics->drawLine(BUF, BUF, BUF, BUF+histoHeight, 4, kColorWhite);
  pd->graphics->drawLine(BUF, BUF+histoHeight, BUF+histoWidth, BUF+histoHeight, 4, kColorWhite);

  for (int i = 1; i <= MAX_HOLES; ++i) {
    pd->graphics->drawLine(BUF + i*3*BALL_RADIUS, BUF+histoHeight+tick, BUF + i*3*BALL_RADIUS, BUF+histoHeight-tick, 2, kColorWhite);
  }
  for (int i = 0; i < maxHistoBalls; ++i) {
    pd->graphics->drawLine(BUF+tick, BUF + i*2*BALL_RADIUS, BUF-tick, BUF + i*2*BALL_RADIUS, 2, kColorWhite);
  }

  bitmapSetRoobert10();
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  char text[8];
  for (int i = 0; i < MAX_HOLES; ++i) {
    snprintf(text, 8, "%i", i+1);
    pd->graphics->drawText(text, 8, kUTF8Encoding, BUF + tick*2 + i*3*BALL_RADIUS, BUF+histoHeight+tick);
  }

  pd->graphics->setDrawMode(kDrawModeCopy);
  for (int i = 0; i < MAX_HOLES; ++i) {
    pd->system->logToConsole("for hole %i the histo will draw %i balls", i+1, balls[i]);
    for (int j = 1; j <= balls[i]; j++) {
      pd->graphics->drawBitmap(m_marbleBitmap, BUF + BALL_RADIUS/2 + i*3*BALL_RADIUS, BUF + (maxHistoBalls - j)*2*BALL_RADIUS, kBitmapUnflipped);
    }
  }

  pd->graphics->popContext();
}

void bitmapDoUpdateScoreCard(void) {

  uint16_t rounds[MAX_LEVELS] = {-1};
  uint16_t holesPlayed[MAX_LEVELS] = {0};
  uint16_t holesTotal[MAX_LEVELS] = {0};
  uint16_t pars[MAX_LEVELS] = {0};
  uint16_t scores[MAX_LEVELS] = {0};
  uint16_t totPar = 0;
  uint16_t totScore = 0;
  uint16_t d = 0;
  for (int l = 0; l < MAX_LEVELS; ++l) {
    uint16_t par = 0;
    uint16_t score = 0;
    uint16_t played = 0;
    uint16_t total = 0;
    for (int h = 0; h < MAX_HOLES; ++h) {
      const uint16_t p = IOGetPar(l, h);
      const uint16_t s = IOGetScore(l, h);
      par += p;
      score += s;
      if (p) { total++; }
      if (p && s) { 
        played++;
        totPar += p;
        totScore += s;
      } 
    }
    //
    if (!played && d) { continue; }
    if (total != played) { score = 0; }
    //
    rounds[d] = l;
    holesPlayed[d] = played;
    holesTotal[d] = total;
    pars[d] = par;
    scores[d] = score;
    ++d;
  }


  pd->graphics->clearBitmap(m_scoreCardBitmap, kColorClear);
  pd->graphics->pushContext(m_scoreCardBitmap);

  const uint8_t PIXX = 17;
  const uint8_t PIXY = 16;
  const uint8_t N_MAX = 7;
  const uint8_t TBAR = 56;
  const uint8_t BBAR = 11*PIXY + PIXY/2;

  int points[12] = {
    1*PIXX,  TBAR - (3*PIXY),
    10*PIXX, TBAR - (3*PIXY),
    13*PIXX, TBAR,
    13*PIXX, BBAR + PIXY + PIXY/2,
    4*PIXX, BBAR + PIXY + PIXY/2,
    4*PIXX, TBAR
  };
  pd->graphics->fillPolygon(6, points, kColorBlack, kPolygonFillNonZero);

  pd->graphics->drawLine(1*PIXX, TBAR - (3*PIXY), 10*PIXX, TBAR - (3*PIXY), 4, kColorWhite); // ---
  pd->graphics->drawLine(4*PIXX, TBAR, 13*PIXX, TBAR, 4, kColorWhite); // ---

  pd->graphics->drawLine(4*PIXX, BBAR, 13*PIXX, BBAR, 4, kColorWhite); // ---
  pd->graphics->drawLine(4*PIXX, BBAR + PIXY + PIXY/2, 13*PIXX, BBAR + PIXY + PIXY/2, 4, kColorWhite); // ---


  pd->graphics->drawLine(4*PIXX, TBAR, 4*PIXX, BBAR + PIXY + PIXY/2, 4, kColorWhite);
  pd->graphics->drawLine(1*PIXX, TBAR - (3*PIXY), 4*PIXX, TBAR, 4, kColorWhite);

  pd->graphics->drawLine(6*PIXX, TBAR, 6*PIXX, BBAR, 4, kColorWhite);
  pd->graphics->drawLine(3*PIXX, TBAR - (3*PIXY), 6*PIXX, TBAR, 4, kColorWhite);

  pd->graphics->drawLine(8*PIXX, TBAR, 8*PIXX, BBAR, 4, kColorWhite);
  pd->graphics->drawLine(5*PIXX, TBAR - (3*PIXY), 8*PIXX, TBAR, 4, kColorWhite);

  pd->graphics->drawLine(10*PIXX, TBAR, 10*PIXX, BBAR, 4, kColorWhite);
  pd->graphics->drawLine(7*PIXX, TBAR - (3*PIXY), 10*PIXX, TBAR, 4, kColorWhite);

  pd->graphics->drawLine(13*PIXX, TBAR, 13*PIXX, BBAR + PIXY + PIXY/2, 4, kColorWhite);
  pd->graphics->drawLine(10*PIXX, TBAR - (3*PIXY), 13*PIXX, TBAR, 4, kColorWhite);

  bitmapSetRoobert10();
  LCDBitmap* tempBitmap = pd->graphics->newBitmap(64, 64, kColorClear);
  for (int i = 0; i < 4; ++i) {
    pd->graphics->clearBitmap(tempBitmap, kColorClear);
    pd->graphics->pushContext(tempBitmap);
    bitmapSetRoobert10();
    const char* text;
    switch (i) {
      case 0: text = "ROUND"; break;
      case 1: text = "HOLES"; break;
      case 2: text = "PAR  "; break; // Spaces for better alignment
      case 3: text = "SCORE"; break;
    }
    const int32_t w = pd->graphics->getTextWidth(bitmapGetRoobert10(), text, 128, kUTF8Encoding, 0);
    pd->graphics->setDrawMode(kDrawModeFillWhite);
    pd->graphics->drawText(text, 128, kUTF8Encoding, 64 - w, 32 - 8);
    pd->graphics->popContext();
    uint8_t extra = (i == 3 ? BUF/2 : 0); 
    pd->graphics->drawRotatedBitmap(tempBitmap, 3*PIXX + i*PIXX*2 + extra, 24, 45.0f, 0.5f, 0.5f, 1.0f, 1.0f);
  }
  pd->graphics->freeBitmap(tempBitmap);

  uint32_t start = 0;
  uint32_t stop = d;
  uint16_t yOff = 0;

  static uint32_t timer = 0;
  ++timer;

  // Scroll mode
  if (d > N_MAX) {
    start = (timer/2) / BUF;
    stop = start + N_MAX + 1;
    yOff = (timer/2) % BUF;
  }

  pd->graphics->setClipRect(4*PIXX, TBAR, 9*PIXX, BBAR - TBAR);

  pd->graphics->setDrawMode(kDrawModeFillWhite);
  uint8_t count = 0;
  for (int i = start; i < stop; ++i) {
    const uint16_t l = i % d; 
    char text[128];

    snprintf(text, 128, "%i", rounds[l]+1);
    int32_t w = pd->graphics->getTextWidth(bitmapGetRoobert10(), text, 128, kUTF8Encoding, 0);
    pd->graphics->drawText(text, 128, kUTF8Encoding, 5*PIXX - w/2, 4*PIXY + (count*PIXY) - yOff);

    snprintf(text, 128, "%i/%i", holesPlayed[l], holesTotal[l]);
    w = pd->graphics->getTextWidth(bitmapGetRoobert10(), text, 128, kUTF8Encoding, 0);
    pd->graphics->drawText(text, 128, kUTF8Encoding, 7*PIXX - w/2, 4*PIXY + (count*PIXY) - yOff);

    snprintf(text, 128, "%i", pars[l]);
    w = pd->graphics->getTextWidth(bitmapGetRoobert10(), text, 128, kUTF8Encoding, 0);
    pd->graphics->drawText(text, 128, kUTF8Encoding, 9*PIXX - w/2, 4*PIXY + (count*PIXY) - yOff);

    if (scores[l] == 0) { // Not played all holes
      snprintf(text, 128, "~");
    } else {
      int16_t diff = pars[l] - scores[l];
      if (diff > 0) { snprintf(text, 128, "+%i", diff); }
      else { snprintf(text, 128, "%i", diff); }
    }
    w = pd->graphics->getTextWidth(bitmapGetRoobert10(), text, 128, kUTF8Encoding, 0);
    pd->graphics->drawText(text, 128, kUTF8Encoding, 11*PIXX + PIXX/2 - w/2, 4*PIXY + (count*PIXY) - yOff);

    ++count;
  }

  pd->graphics->clearClipRect();

  {
    char text[128];
    int16_t tot = totPar - totScore;
    if (tot > 0) { snprintf(text, 128, "%i OVER PAR", tot); }
    else if (tot < 0) { snprintf(text, 128, "%i UNDER PAR", tot*-1); }
    else { snprintf(text, 128, "EQUAL TO PAR"); }
    const int32_t w = pd->graphics->getTextWidth(bitmapGetRoobert10(), text, 128, kUTF8Encoding, 0);
    pd->graphics->drawText(text, 128, kUTF8Encoding, 8*PIXX + PIXX/2 - w/2, BBAR + PIXY/4 + 1);
  }

  pd->graphics->popContext();
}


void bitmapDoUpdateLevelStatsBitmap(void) {
  bitmapSetRoobert24();
  char text[128];
  uint16_t score = 0;
  uint16_t par = 0;
  IOGetLevelStatistics(IOGetCurrentLevel(), &score, &par);
  if (!score && !par) {
    // This means that the player hasn't finished any of the level yet
    uint16_t holes = 0;
    uint16_t played = 0;
    for (int h = 0; h < MAX_HOLES; ++h) {
      if (IOGetPar(IOGetCurrentLevel(), h)) { holes++; }
      if (IOGetScore(IOGetCurrentLevel(), h)) { played++; }
    }
    if (holes == 1) { snprintf(text, 128, "%i OF 1 HOLE", (int)played); }
    else { snprintf(text, 128, "%i OF %i HOLES", (int)played, (int)holes); }
  } else {
    const int16_t displayScore = par - score;
    if (displayScore < 0) {
      snprintf(text, 128, "%i UNDER PAR", (int)displayScore*-1);
    } else if (displayScore > 0) {
      snprintf(text, 128, "%i OVER PAR", (int)displayScore);
    } else {
      snprintf(text, 128, "EQUAL TO PAR");
    }
  }
  const int32_t w = pd->graphics->getTextWidth(bitmapGetRoobert24(), text, 128, kUTF8Encoding, 0);
  pd->graphics->clearBitmap(m_levelStatsBitmap, kColorClear);
  pd->graphics->pushContext(m_levelStatsBitmap);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  bitmapDoDrawOutlineText(text, 129, NUMERAL_PIX_X - w/2, 0, 2);
  pd->graphics->popContext();
}

void bitmapDoUpdateHoleStatsBitmap(void) {

  uint16_t score = 0;
  uint16_t par = 0;
  IOGetHoleStatistics(IOGetCurrentLevel(), IOGetCurrentHole(), &score, &par);
  bitmapSetRoobert24();
  char text[128];
  snprintf(text, 128, "PAR %i", par);
  const int32_t w1 = pd->graphics->getTextWidth(bitmapGetRoobert24(), text, 128, kUTF8Encoding, 0);
  pd->graphics->clearBitmap(m_holeStatsBitmap[0], kColorClear);
  pd->graphics->pushContext(m_holeStatsBitmap[0]);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  bitmapDoDrawOutlineText(text, 129, NUMERAL_PIX_X/2 - w1/2, 0, 2);
  pd->graphics->popContext();

  pd->graphics->clearBitmap(m_holeStatsBitmap[1], kColorClear);
  if (score) {
    int16_t displayScore = par - score;
    if (displayScore < 0) {
      snprintf(text, 128, "%i UNDER", (int)displayScore*-1);
    } else if (displayScore > 0) {
      snprintf(text, 128, "%i OVER", (int)displayScore);
    } else {
      snprintf(text, 128, "PAR");
    }
    const int32_t w2 = pd->graphics->getTextWidth(bitmapGetRoobert24(), text, 128, kUTF8Encoding, 0);
    pd->graphics->pushContext(m_holeStatsBitmap[1]);
    pd->graphics->setDrawMode(kDrawModeFillBlack);
    bitmapDoDrawOutlineText(text, 129, (NUMERAL_PIX_X+(NUMERAL_BUF*2))/2 - w2/2, 0, 2);
    pd->graphics->popContext();
  }

  pd->graphics->clearBitmap(m_holeAuthorBitmap, kColorClear);
  const char* author = IOGetCurrentHoleAuthor();
  if (strlen(author)) {
    pd->graphics->pushContext(m_holeAuthorBitmap);
    bitmapSetRoobert24();
    snprintf(text, 128, "BY %s", author);
    const int32_t w3 = pd->graphics->getTextWidth(bitmapGetRoobert24(), text, 128, kUTF8Encoding, 0);
    pd->graphics->setDrawMode(kDrawModeFillBlack);
    bitmapDoDrawOutlineText(text, 128, HALF_DEVICE_PIX_X/2 - w3/2, 0, 2);
    pd->graphics->popContext();
  }

  pd->graphics->clearBitmap(m_holeNameBitmap, kColorClear);
  const char* name = IOGetCurrentHoleName();
  if (strlen(name)) {
    pd->graphics->pushContext(m_holeNameBitmap);
    bitmapSetRoobert24();
    snprintf(text, 128, "%s", name);
    const int32_t w4 = pd->graphics->getTextWidth(bitmapGetRoobert24(), text, 128, kUTF8Encoding, 0);
    pd->graphics->setDrawMode(kDrawModeFillBlack);
    bitmapDoDrawOutlineText(text, 128, HALF_DEVICE_PIX_X/2 - w4/2, 0, 2);
    pd->graphics->popContext();
  }
}

void bitmapDoPreloadA(void) {
  m_useTheCrankBitmap = bitmapDoLoadImageAtPath("images/useTheCrank");
  m_ditherBitmap = bitmapDoLoadImageAtPath("images/dither");
  m_holeTutorialBitmap = bitmapDoLoadImageAtPath("images/tut/tutorial");
  //m_wfPond = bitmapDoLoadImageAtPath("images/falls_pond");
  m_cardBitmap = bitmapDoLoadImageAtPath("images/card");

  m_turretBarrelTabel = bitmapDoLoadImageTableAtPath("images/anim/turretBarrel");
  m_waterSplashTable[0] = bitmapDoLoadImageTableAtPath("images/anim/MarbleSplash0");
  m_specialBlastTable = bitmapDoLoadImageTableAtPath("images/anim/blast");
  m_tutorialCrankRotateTable = bitmapDoLoadImageTableAtPath("images/tut/crankClockwise");
  m_tutorialCrankAngleTable = bitmapDoLoadImageTableAtPath("images/tut/crankSpin");
  m_tutorialButtonTable = bitmapDoLoadImageTableAtPath("images/tut/buttonPressed");
  m_tutorialDPadTable = bitmapDoLoadImageTableAtPath("images/tut/dPad");
  m_tutorialArrowsTable = bitmapDoLoadImageTableAtPath("images/tut/tutorialPoint");
  m_chevronTable =  bitmapDoLoadImageTableAtPath("images/chevron");
  m_fwBkwIconTable = bitmapDoLoadImageTableAtPath("images/forwarbackward");
  m_turretBodyTable = bitmapDoLoadImageTableAtPath("images/turretBody");
  char text[128];
  for (int i = 0; i < MAX_POPS; ++i) {
    snprintf(text, 128, "images/anim/Pop%i", i);
    m_pegPopTable[i] =  bitmapDoLoadImageTableAtPath(text);
  }
  for (int i = 0; i < N_FOUNTAINS; ++i) {
    snprintf(text, 128, "images/anim/Fountain%i", i);
    m_fountainTable[i] =  bitmapDoLoadImageTableAtPath(text);
  }
  for (int i = 0; i < N_END_BLASTS; ++i) {
    snprintf(text, 128, "images/anim/Blast%i", i);
    m_endBlastTable[i] =  bitmapDoLoadImageTableAtPath(text);
  }

  m_fontRoobert24 = bitmapDoLoadFontAtPath("fonts/Roobert-24-Medium");
  m_fontGreatvibes24 = bitmapDoLoadFontAtPath("fonts/GreatVibes-Regular-24");
  m_fontGreatvibes109 = bitmapDoLoadFontAtPath("fonts/GreatVibes-Regular-109");
  pd->graphics->setFont(m_fontGreatvibes24);

  m_infoTopperBitmap = pd->graphics->newBitmap(DEVICE_PIX_X, TITLETEXT_HEIGHT, kColorClear);
  m_levelTitleBitmap = pd->graphics->newBitmap(DEVICE_PIX_X, DEVICE_PIX_Y, kColorClear);
  m_scoreHistogram = pd->graphics->newBitmap(DEVICE_PIX_X, DEVICE_PIX_Y, kColorBlack);

  m_levelStatsBitmap = pd->graphics->newBitmap(NUMERAL_PIX_X*2, TITLETEXT_HEIGHT, kColorClear);

  m_scoreCardBitmap = pd->graphics->newBitmap(HALF_DEVICE_PIX_X + 32, DEVICE_PIX_Y, kColorClear);

  m_holeStatsBitmap[0] = pd->graphics->newBitmap(NUMERAL_PIX_X, TITLETEXT_HEIGHT, kColorClear);
  m_holeStatsBitmap[1] = pd->graphics->newBitmap(NUMERAL_PIX_X + (2*NUMERAL_BUF), TITLETEXT_HEIGHT, kColorClear);
  m_holeAuthorBitmap = pd->graphics->newBitmap(HALF_DEVICE_PIX_X, TITLETEXT_HEIGHT, kColorClear);
  m_holeNameBitmap = pd->graphics->newBitmap(HALF_DEVICE_PIX_X, TITLETEXT_HEIGHT, kColorClear);
}

void bitmapDoPreloadB(const uint8_t anim) { // TURRET_LAUNCH_FRAMES
  m_turretBarrel[anim][0] = pd->graphics->getTableBitmap(m_turretBarrelTabel, anim);
  for (int32_t a = 1; a < 256; ++a) {
    const float angle = (360.0f / 256.0f) * a;
    m_turretBarrel[anim][a] = pd->graphics->newBitmap(TURRET_RADIUS*2, TURRET_RADIUS*2, kColorClear);
    pd->graphics->pushContext(m_turretBarrel[anim][a]);
    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->drawRotatedBitmap(m_turretBarrel[anim][0], TURRET_RADIUS, TURRET_RADIUS, angle, 0.5f, 0.5f, 1.0f, 1.0f);
    pd->graphics->popContext();
  }
}

void bitmapDoPreloadC(void) {
  for (int i = 0; i < TURRET_RADIUS; ++i) {
    m_pootAnimation[i] = pd->graphics->newBitmap(TURRET_RADIUS*2, TURRET_RADIUS*2, kColorClear);
    pd->graphics->pushContext(m_pootAnimation[i]);
    pd->graphics->fillEllipse(TURRET_RADIUS-i, TURRET_RADIUS-i, 2*i, 2*i, 0.0f, 360.0f, kColorBlack);
    pd->graphics->popContext();
  }
}

void bitmapDoPreloadD(void) {
  char text[128];
  for (int32_t i = 1; i < N_WF_FG; ++i) { // Did 0 already as a critical load
    snprintf(text, 128, "images/falls_fg/falls%i_fg", (int)i);
    m_sheetWfFg[i] = bitmapDoLoadImageTableAtPath(text);
  }
  // Special
  for (int32_t i = WF_SPECIAL_START; i < WF_SPECIAL_END; ++i) { 
    snprintf(text, 128, "images/falls_fg/falls%i_fg", (int)i);
    m_sheetWfFg[i] = bitmapDoLoadImageTableAtPath(text);
  }
  // Credits
  snprintf(text, 128, "images/falls_fg/falls100_fg");
  m_sheetWfFg[100] = bitmapDoLoadImageTableAtPath(text);
#ifndef WF_FIXED_BG
  for (int32_t i = 1; i < N_WF_BG; ++i) { // Did 0 already as a critical load
    snprintf(text, 128, "images/falls_bg/falls%i_bg", (int)i);
    m_wfBg[i] = bitmapDoLoadImageAtPath(text);
  }
#endif
}

void bitmapDoPreloadE(void) {
  m_previewBitmapWindow = pd->graphics->newBitmap(HALF_DEVICE_PIX_X, DEVICE_PIX_Y, kColorClear);
  m_previewBitmapStencil = bitmapDoLoadImageAtPath("images/previewBorder");
  char text[128];
  for (int l = 0; l < MAX_LEVELS; ++l) {
    for (int h = 0; h < MAX_HOLES; ++h) {
      if (!IOGetPar(l,h)) { continue; } // No level
      snprintf(text, 128, "images/holes/level_%i_hole_%i", l+1, h+1);
      m_previewBitmap[l][h] = pd->graphics->loadBitmap(text, NULL);
    }
  }
}

void bitmapDoPreloadF(void) {
  m_marbleBitmap = pd->graphics->newBitmap(BALL_RADIUS*2, BALL_RADIUS*2, kColorClear);
  pd->graphics->pushContext(m_marbleBitmap);
  pd->graphics->fillEllipse(0, 0, BALL_RADIUS*2, BALL_RADIUS*2, 0.0f, 360.0f, kColorWhite);
  pd->graphics->drawEllipse(0, 0, BALL_RADIUS*2, BALL_RADIUS*2, 2, 0.0f, 360.0f, kColorBlack);
  pd->graphics->popContext();
  for (int s = 0; s < MAX_PEG_SIZE; ++s) {
    const float scale = bitmapSizeToScale(s);
    const float ballSize = BALL_RADIUS * 2.0f * scale;
    m_ballBitmap[0][s] = pd->graphics->newBitmap(ballSize, ballSize, kColorClear);
    pd->graphics->pushContext(m_ballBitmap[0][s]);
    pd->graphics->fillEllipse(0, 0, ballSize, ballSize, 0.0f, 360.0f, kColorWhite);
    pd->graphics->drawEllipse(0, 0, ballSize, ballSize, 2, 0.0f, 360.0f, kColorBlack);
    pd->graphics->popContext();
    m_ballBitmap[1][s] = pd->graphics->newBitmap(ballSize, ballSize, kColorClear);
    pd->graphics->pushContext(m_ballBitmap[1][s]);
    pd->graphics->fillEllipse(0, 0, ballSize, ballSize, 0.0f, 360.0f, kColorWhite);
    pd->graphics->fillEllipse(0, 0, ballSize, ballSize, 0.0f, 360.0f, (uintptr_t)kGreyPattern);
    pd->graphics->drawEllipse(0, 0, ballSize, ballSize, 2, 0.0f, 360.0f, kColorBlack);
    pd->graphics->popContext();
  }
}

//  for (int s = 0; s < MAX_PEG_SIZE; ++s) {
void bitmapDoPreloadG(const uint8_t size) {
  const float scale = bitmapSizeToScale(size);
  for (int32_t iAngle = 0; iAngle < 128; ++iAngle) {
    m_rectBitmap[0][size][iAngle] = pd->graphics->newBitmap(BOX_HALF_MAX*2*scale, BOX_HALF_MAX*2*scale, kColorClear);
    pd->graphics->pushContext(m_rectBitmap[0][size][iAngle]);
    bitmapDoDrawRotatedRect(BOX_HALF_MAX * scale, BOX_HALF_MAX * scale, (BOX_HWIDTH/2) * scale, (BOX_HALF_HEIGHT/2) * scale, iAngle, kRenderColorWhite);
    // pd->graphics->drawRect(0, 0, BOX_HALF_MAX*2*scale, BOX_HALF_MAX*2*scale, kColorWhite);
    pd->graphics->popContext();
    m_rectBitmap[1][size][iAngle] = pd->graphics->newBitmap(BOX_HALF_MAX*2*scale, BOX_HALF_MAX*2*scale, kColorClear);
    pd->graphics->pushContext(m_rectBitmap[1][size][iAngle]);
    bitmapDoDrawRotatedRect(BOX_HALF_MAX * scale, BOX_HALF_MAX * scale, (BOX_HWIDTH/2) * scale, (BOX_HALF_HEIGHT/2) * scale, iAngle, kRenderColorGrey);
    // pd->graphics->drawRect(0, 0, BOX_HALF_MAX*2*scale, BOX_HALF_MAX*2*scale, kColorWhite);
    pd->graphics->popContext();
  }
}

void bitmapDoPreloadH(const uint8_t size) {
  const float scale = bitmapSizeToScale(size);
  for (int32_t iAngle = 0; iAngle < 128; ++iAngle) {
    m_hexBitmap[size][iAngle] = pd->graphics->newBitmap(HEX_MAX*2*scale, HEX_MAX*2*scale, kColorClear);
    pd->graphics->pushContext(m_hexBitmap[size][iAngle]);
    bitmapDoDrawRotatedPoly(6, HEX_MAX * scale, HEX_MAX * scale, (HEX_WIDTH/2) * scale, (HEX_WIDTH/2) * scale, iAngle, kRenderColorHatched);
    pd->graphics->popContext();
  }
}

void bitmapDoPreloadH2(const uint8_t size) {
  const float scale = bitmapSizeToScale(size);
  for (int32_t iAngle = 0; iAngle < 256; ++iAngle) {
    m_triBitmap[0][size][iAngle] = pd->graphics->newBitmap(TRI_MAX*2*scale, TRI_MAX*2*scale, kColorClear);
    pd->graphics->pushContext(m_triBitmap[0][size][iAngle]);
    bitmapDoDrawRotatedPoly(3, TRI_MAX * scale, TRI_MAX * scale, (TRI_WIDTH/2) * scale, (TRI_WIDTH/2) * scale, iAngle - 64, kRenderColorWhite);
    pd->graphics->popContext();

    m_triBitmap[1][size][iAngle] = pd->graphics->newBitmap(TRI_MAX*2*scale, TRI_MAX*2*scale, kColorClear);
    pd->graphics->pushContext(m_triBitmap[1][size][iAngle]);
    bitmapDoDrawRotatedPoly(3, TRI_MAX * scale, TRI_MAX * scale, (TRI_WIDTH/2) * scale, (TRI_WIDTH/2) * scale, iAngle - 64, kRenderColorGrey);
    pd->graphics->popContext();
  }
}

void bitmapDoPreloadI(void) {
  char text[128];
  const uint8_t pixelFineTuning[10] = {
    2, // 0
    4, // 1
    0, // 2
    2, // 3
    8, // 4
    0, // 5
    0, // 6
    0, // 7
    0, // 8
    0  // 9
  };
  for (int n = 0; n < 10; ++n) {
    m_numeralBitmap[n] = pd->graphics->newBitmap(NUMERAL_PIX_X, NUMERAL_PIX_Y, kColorClear);
    pd->graphics->pushContext(m_numeralBitmap[n]);
    pd->graphics->drawBitmap(m_cardBitmap, 0, 0, kBitmapFlippedY);
    snprintf(text, 128, "%i", n);
    const int32_t w = pd->graphics->getTextWidth(bitmapGetGreatVibes109(), text, 128, kUTF8Encoding, 0);
    bitmapSetGreatVibes109();
    pd->graphics->setDrawMode(kDrawModeFillBlack);
    bitmapDoDrawOutlineText(text, 128, NUMERAL_PIX_X/2 - w/2 - pixelFineTuning[n], 0, 4);
    pd->graphics->popContext();
  }
}

void bitmapDoPreloadJ(void) {
  char text[128];

  for (int s = 0; s < kNPegSpecial; ++s) {
    m_specialTextBitmap[s] = pd->graphics->newBitmap(SPECIAL_TEXT_WIDTH, TITLETEXT_HEIGHT, kColorClear);
    pd->graphics->pushContext(m_specialTextBitmap[s]);
    bitmapSetRoobert24();
    const int32_t w0 = pd->graphics->getTextWidth(bitmapGetRoobert24(), pegGetSpecialTxt(s), 128, kUTF8Encoding, 0);
    pd->graphics->setDrawMode(kDrawModeFillBlack);
    bitmapDoDrawOutlineText(pegGetSpecialTxt(s), 128, SPECIAL_TEXT_WIDTH/2 - w0/2, 0, 2);
    pd->graphics->popContext();
  }

  //

  LCDBitmap* tempBitmap = pd->graphics->newBitmap(NUMERAL_PIX_Y, TITLETEXT_HEIGHT, kColorClear);
  pd->graphics->pushContext(tempBitmap);
  bitmapSetRoobert24();
  const int32_t w1 = pd->graphics->getTextWidth(bitmapGetRoobert24(), "LEVEL", 128, kUTF8Encoding, 0);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  bitmapDoDrawOutlineText("LEVEL", 128, NUMERAL_PIX_Y/2 - w1/2, 0, 2);
  pd->graphics->popContext();

  m_levelBitmap = pd->graphics->newBitmap(TITLETEXT_HEIGHT, NUMERAL_PIX_Y, kColorClear);
  pd->graphics->pushContext(m_levelBitmap);
  pd->graphics->drawRotatedBitmap(tempBitmap, TITLETEXT_HEIGHT/2, NUMERAL_PIX_Y/2, 90.0f, 0.5f, 0.5f, 1.0f, 1.0f);
  pd->graphics->popContext();

  //

  pd->graphics->clearBitmap(tempBitmap, kColorClear);
  pd->graphics->pushContext(tempBitmap);
  bitmapSetRoobert24();
  const int32_t w2 = pd->graphics->getTextWidth(bitmapGetRoobert24(), "HOLE", 128, kUTF8Encoding, 0);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  bitmapDoDrawOutlineText("HOLE", 128, NUMERAL_PIX_Y/2 - w2/2, 0, 2);
  pd->graphics->popContext();

  m_holeBitmap = pd->graphics->newBitmap(TITLETEXT_HEIGHT, NUMERAL_PIX_Y, kColorClear);
  pd->graphics->pushContext(m_holeBitmap);
  pd->graphics->drawRotatedBitmap(tempBitmap, TITLETEXT_HEIGHT/2, NUMERAL_PIX_Y/2, 3*90.0f, 0.5f, 0.5f, 1.0f, 1.0f);
  pd->graphics->popContext();

  //

  pd->graphics->clearBitmap(tempBitmap, kColorClear);
  pd->graphics->pushContext(tempBitmap);
  bitmapSetRoobert24();
  const int32_t w3 = pd->graphics->getTextWidth(bitmapGetRoobert24(), "PLAYER", 128, kUTF8Encoding, 0);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  bitmapDoDrawOutlineText("PLAYER", 128, NUMERAL_PIX_Y/2 - w3/2, 0, 2);
  pd->graphics->popContext();

  m_playerBitmap = pd->graphics->newBitmap(TITLETEXT_HEIGHT, NUMERAL_PIX_Y, kColorClear);
  pd->graphics->pushContext(m_playerBitmap);
  pd->graphics->drawRotatedBitmap(tempBitmap, TITLETEXT_HEIGHT/2, NUMERAL_PIX_Y/2, 3*90.0f, 0.5f, 0.5f, 1.0f, 1.0f);
  pd->graphics->popContext();

  //

  pd->graphics->freeBitmap(tempBitmap);
  tempBitmap = NULL;
}

void bitmapDoPreloadK(void) {
  const uint16_t stencilStep = (DEVICE_PIX_Y + (2*WF_DIVISION_PIX_Y)) / STENCIL_WIPE_N; // WF_DIVISION_PIX_Y is the height of the dither
  for (int i = 0; i < STENCIL_WIPE_N; ++i) {
    m_stencilWipeBitmap[i] = pd->graphics->newBitmap(DEVICE_PIX_X, DEVICE_PIX_Y, kColorClear);
    pd->graphics->pushContext(m_stencilWipeBitmap[i]);
    pd->graphics->setDrawMode(kDrawModeInverted); // White is used by the stencil
    pd->graphics->drawBitmap(m_ditherBitmap, 0, -WF_DIVISION_PIX_Y + (stencilStep * i), kBitmapFlippedY);
    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->fillRect(0, -WF_DIVISION_PIX_Y, DEVICE_PIX_X, (stencilStep * i), kColorWhite);
    pd->graphics->popContext();
  }
}


void bitmapDoPreloadL(void) {
  for (int n = 0; n < POND_WATER_TILES; ++n) {
    for (int i = 0; i < POND_WATER_FRAMES; ++i) {
      m_wfPond[n][i] = pd->graphics->newBitmap(DEVICE_PIX_X, POND_WATER_HEIGHT, kColorBlack);
    }
  }
}

#define N_POND_LINES 3
void bitmapDoPreloadM(const uint8_t n) {
  const uint16_t offset = POND_WATER_HEIGHT * n;
  int16_t x[POND_WATER_HEIGHT][N_POND_LINES];
  uint8_t w[POND_WATER_HEIGHT][N_POND_LINES];
  for (int line = 0; line < POND_WATER_HEIGHT; line += 2) {
    uint8_t nLines = (rand() % (N_POND_LINES - 1)) + 1;
    for (int n = 0; n < N_POND_LINES; ++n) { x[line][n] = -1; }
    for (int n = 0; n < nLines; ++n) { 
      x[line][n] = rand() % DEVICE_PIX_X;
      w[line][n] = (rand() % (POND_WATER_HEIGHT * POND_WATER_TILES) > (line + offset) ? 1 : 2);
    }
  }
  for (int t = 0; t < POND_WATER_FRAMES; ++t) {
    pd->graphics->pushContext(m_wfPond[n][t]);
    pd->graphics->setDrawMode(kDrawModeCopy);
    for (int line = 0; line < POND_WATER_HEIGHT; line += 2) {
      const float speed = (line + offset)/256.0f + 0.1f;
      const int width = MAX((line + offset)/6, 4);
      for (int n = 0; n < N_POND_LINES; ++n) {
        if (x[line][n] == -1) { continue; }
        const int x1 = x[line][n] + (t * speed);
        const int x2 = x1 + width;
        for (int xOff = -20; xOff < 18; ++xOff) {
          const int x3 = x1 + (POND_WATER_FRAMES * speed * xOff);
          const int x4 = x3 + width;
          pd->graphics->drawLine(x3, line, x4, line, w[line][n], kColorWhite);
        }
      }
    }
    pd->graphics->popContext();
  }
}

void bitmapDoInit(void) {
  // Load critical bitmaps
  m_fontRoobert10 = bitmapDoLoadFontAtPath("fonts/Roobert-10-Bold");
  m_headerImage = bitmapDoLoadImageAtPath("images/splash");
  m_wfBg[0] = bitmapDoLoadImageAtPath("images/falls_bg/falls0_bg");
  m_sheetWfFg[0] = bitmapDoLoadImageTableAtPath("images/falls_fg/falls0_fg");
}
