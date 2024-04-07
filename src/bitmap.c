#include "bitmap.h"
#include "input.h"
#include "physics.h"
#include "io.h"

// Titiles

LCDBitmap* m_titleSelected;
LCDBitmap* m_splash;

LCDBitmap* m_playerBitmap;
LCDBitmap* m_levelBitmap;
LCDBitmap* m_levelStatsBitmap;
LCDBitmap* m_holeBitmap;
LCDBitmap* m_holeStatsBitmap[2];
LCDBitmap* m_holeTutorialBitmap;

LCDBitmap* m_useTheCrankBitmap;

LCDBitmap* m_ditherBitmap;

LCDBitmap* m_stencilWipeBitmap[STENCIL_WIPE_N];

// Game

LCDBitmap* m_header;
LCDBitmap* m_turretBody;
LCDBitmap* m_turretBarrel[8][256];
LCDBitmapTable* m_turretBarrelTabel;
LCDBitmap* m_infoTopperBitmap;
LCDBitmap* m_levelSplashBitmap;

LCDBitmap* m_numeralBitmap[10];
LCDBitmap* m_numeralStencil;

LCDBitmap* m_scoreHistogram = NULL;

LCDBitmap* m_pootAnimation[TURRET_RADIUS];

LCDBitmap* m_ballBitmap[2][MAX_PEG_SIZE];
LCDBitmap* m_rectBitmap[2][MAX_PEG_SIZE][128];

LCDBitmap* m_wfPond;
LCDBitmap* m_wfBg[N_WATERFALLS];
LCDBitmapTable* m_sheetWfFg[N_WATERFALLS];

LCDFont* m_fontRoobert24;
LCDFont* m_fontRoobert10;
LCDFont* m_fontGreatvibes24;
LCDFont* m_fontGreatvibes109;

LCDBitmap* loadImageAtPath(const char* _path);

LCDBitmapTable* loadImageTableAtPath(const char* _path);

LCDFont* loadFontAtPath(const char* _path);

/// ///

LCDBitmap* loadImageAtPath(const char* _path) {
  const char* _outErr = NULL;
  LCDBitmap* _img = pd->graphics->loadBitmap(_path, &_outErr);
  if (_outErr != NULL) {
    pd->system->error("Error loading image at path '%s': %s", _path, _outErr);
  }
  return _img;
}

LCDBitmapTable* loadImageTableAtPath(const char* _path) {
  const char* _outErr = NULL;
  LCDBitmapTable* _table = pd->graphics->loadBitmapTable(_path, &_outErr);
  if (_outErr != NULL) {
    pd->system->error("Error loading image table at path '%s': %s", _path, _outErr);
  }
  return _table;
}

LCDFont* loadFontAtPath(const char* _path) {
  const char* _outErr = NULL;
  LCDFont* _f = pd->graphics->loadFont(_path, &_outErr);
  if (_outErr != NULL) {
    pd->system->error("Error loading font at path '%s': %s", _path, _outErr);
  }
  return _f;
}

void drawOutlineText(char text[], uint16_t textSize, int16_t x, int16_t y, uint16_t outlineSize) {
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

void updateLevelSplashBitmap(void) {
  pd->graphics->clearBitmap(m_levelSplashBitmap, kColorBlack);
  pd->graphics->pushContext(m_levelSplashBitmap);
  char text[128];
  snprintf(text, 128, "%i~%i", (int)getCurrentLevel() + 1, (int)getCurrentHole() + 1);
  const int32_t w1 = pd->graphics->getTextWidth(getGreatVibes109(), text, 128, kUTF8Encoding, 0);
  setGreatVibes109();
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  drawOutlineText(text, 128, DEVICE_PIX_X/2 - w1/2, 0, 4);
  //
  snprintf(text, 128, "PAR %i", (int)getCurrentHolePar());
  const int32_t w2 = pd->graphics->getTextWidth(getRoobert24(), text, 128, kUTF8Encoding, 0);
  setRoobert24();
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  pd->graphics->drawText(text, 128, kUTF8Encoding, DEVICE_PIX_X/2 - w2/2, 128+32);
  pd->graphics->popContext();
}

void updateInfoTopperBitmap(void) {
  pd->graphics->clearBitmap(m_infoTopperBitmap, kColorBlack);
  pd->graphics->pushContext(m_infoTopperBitmap);
  char text[128];
  snprintf(text, 128, "%i~%i", (int)getCurrentLevel(), (int)getCurrentHole());
  setGreatVibes24();
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  pd->graphics->drawText(text, 128, kUTF8Encoding, 0, 8);
  setRoobert24();
  const int32_t width = pd->graphics->getTextWidth(getRoobert24(), "By Tim Martin", 128, kUTF8Encoding, 0);
  pd->graphics->drawText("By Tim Martin", 128, kUTF8Encoding, DEVICE_PIX_X - width, 2);
  pd->graphics->popContext();
}

LCDBitmap* getBitmapAnimPoot(uint8_t i) { return m_pootAnimation[i]; }

LCDBitmap* getBitmapUseTheCrank(void) { return m_useTheCrankBitmap; }

LCDBitmap* getSpriteSplash() { return m_splash; }

LCDBitmap* getBitmapWfPond() { return m_wfPond; }

LCDBitmap* getBitmapWfBg(uint8_t wf) { return m_wfBg[wf]; }

LCDBitmap* getBitmapWfFg(uint8_t _wf, uint32_t _x, uint32_t _y) {
  return getBitmapWfFg_byidx(_wf, WF_ID(_x, _y));
}

LCDBitmap* getBitmapWfFg_byidx(uint8_t _wf, uint32_t _idx) {
  return pd->graphics->getTableBitmap(m_sheetWfFg[_wf], _idx);
}

LCDBitmap* getBitmapPlayer(void) { return m_playerBitmap; }

LCDBitmap* getBitmapLevel(void) { return m_levelBitmap; }

LCDBitmap* getBitmapLevelStats(void) { return m_levelStatsBitmap; }

LCDBitmap* getBitmapHole(void) { return m_holeBitmap; }

LCDBitmap* getBitmapHoleStatsA(void) { return m_holeStatsBitmap[0]; }

LCDBitmap* getBitmapHoleStatsB(void) { return m_holeStatsBitmap[1]; }

LCDBitmap* getBitmapHoleTutorial(void) { return m_holeTutorialBitmap; }

LCDBitmap* getBitmapDither(void) { return m_ditherBitmap; }

LCDBitmap* getBitmapPeg(const struct Peg_t* p) {
  switch (p->m_shape) {
    case kPegShapeBall: return m_ballBitmap[p->m_type == 0 ? 0 : 1][p->m_size];
    case kPegShapeRect: return m_rectBitmap[p->m_type == 0 ? 0 : 1][p->m_size][p->m_iAngle % 128]; // Symmetry
    default: return NULL;
  }
  return NULL;
}

LCDBitmap* getBitmapBall(void) { return m_ballBitmap[0][0]; }

LCDBitmap* getBitmapNumeral(int8_t n) { return m_numeralBitmap[n % 10]; }

LCDBitmap* getStencilNumeral(void) { return m_numeralStencil; }

LCDBitmap* getStencilWipe(int8_t n) { return m_stencilWipeBitmap[n % STENCIL_WIPE_N]; }

LCDBitmap* getTitleSelectedBitmap() { return m_titleSelected; }

LCDBitmap* getBitmapTurretBody(void) { return m_turretBody; }

LCDBitmap* getBitmapHeader(void) { return m_header; }

LCDBitmap* getInfoTopperBitmap(void) { return m_infoTopperBitmap; }

LCDBitmap* getLevelSplashBitmap(void) { return m_levelSplashBitmap; }

LCDBitmap* getBitmapTurretBarrel(void) {
  return m_turretBarrel[(getFrameCount() % 32) / 4][ angToByte(getTurretBarrelAngle()) ];
}

void setRoobert10() { pd->graphics->setFont(m_fontRoobert10); }

void setRoobert24() { pd->graphics->setFont(m_fontRoobert24); }

void setGreatVibes24() { pd->graphics->setFont(m_fontGreatvibes24); }

void setGreatVibes109() { pd->graphics->setFont(m_fontGreatvibes109); }

LCDFont* getRoobert10(void) { return m_fontRoobert10; }

LCDFont* getRoobert24() { return m_fontRoobert24; }

LCDFont* getGreatVibes24() { return m_fontGreatvibes24; }

LCDFont* getGreatVibes109() { return m_fontGreatvibes109; }

float sizeToScale(uint8_t size) {
  switch (size) {
    case 0: return 1.0f;
    case 1: return 1.25f;
    case 2: return 1.50f;
    case 3: return 1.75f;
    case 4: return 2.0f;
    default: pd->system->error("Error sizeToScale called with unknown size %i", size);
  }
  return 1.0f;
}

LCDBitmap* getScoreHistogram(void) { return m_scoreHistogram; }

void drawRotatedRect(float x, float y, float w2, float h2, uint8_t iAngle, bool grey) {
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
    if (grey) {
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

void updateScoreHistogramBitmap(void) {
  pd->graphics->clearBitmap(m_scoreHistogram, kColorBlack);
  pd->graphics->pushContext(m_scoreHistogram);
  const int16_t maxHistoBalls = 12;
  const int16_t histoHeight = maxHistoBalls*2*BALL_RADIUS;
  const int16_t histoWidth = MAX_HOLES*3*BALL_RADIUS;
  const int16_t tick = 4;
  int16_t balls[MAX_HOLES] = {0};
  int16_t par[MAX_HOLES] = {0};
  for (int hole = 0; hole < MAX_HOLES;  ++hole) {
    balls[hole] = getScore(getCurrentLevel(), hole);
    par[hole] = getPar(getCurrentLevel(), hole);
  }
  // This will be animated in instead
  balls[getCurrentHole()] = 0;

  for (int i = 0; i < MAX_HOLES;  ++i) {
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

  setRoobert10();
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  char text[8];
  for (int i = 0; i < MAX_HOLES; ++i) {
    snprintf(text, 8, "%i", i+1);
    pd->graphics->drawText(text, 8, kUTF8Encoding, BUF + tick*2 + i*3*BALL_RADIUS, BUF+histoHeight+tick);
  }

  pd->graphics->setDrawMode(kDrawModeCopy);
  for (int i = 0; i < MAX_HOLES; ++i) {
    for (int j = 1; j <= balls[i]; j++) {
      pd->graphics->drawBitmap(m_ballBitmap[0][0], BUF + BALL_RADIUS/2 + i*3*BALL_RADIUS, BUF + (maxHistoBalls - j)*2*BALL_RADIUS, kBitmapUnflipped);
    }
  }

  pd->graphics->popContext();
}


void updateLevelStatsBitmap(void) {
  setRoobert24();
  char text[128];
  uint16_t score = 0;
  uint16_t par = 0;
  getLevelStatistics(getCurrentLevel(), &score, &par);
  int16_t displayScore = par - score;
  pd->graphics->clearBitmap(m_levelStatsBitmap, kColorClear);
  if (!score && !par) {
    // This means that the player hasn't finished any of the level yet
    return;
  }
  if (displayScore < 0) {
    snprintf(text, 128, "%i UNDER PAR", (int)displayScore*-1);
  } else if (displayScore > 0) {
    snprintf(text, 128, "%i OVER PAR", (int)displayScore);
  } else {
    snprintf(text, 128, "EQUAL TO PAR");
  }
  const int32_t w = pd->graphics->getTextWidth(getRoobert24(), text, 128, kUTF8Encoding, 0);
  pd->graphics->pushContext(m_levelStatsBitmap);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  drawOutlineText(text, 129, NUMERAL_PIX_X - w/2, 0, 2);
  pd->graphics->popContext();
}

void updateHoleStatsBitmap(void) {
  uint16_t score = 0;
  uint16_t par = 0;
  getHoleStatistics(getCurrentLevel(), getCurrentHole(), &score, &par);
  setRoobert24();
  char text[128];
  snprintf(text, 128, "PAR %i", par);
  const int32_t w1 = pd->graphics->getTextWidth(getRoobert24(), text, 128, kUTF8Encoding, 0);
  pd->graphics->pushContext(m_holeStatsBitmap[0]);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  drawOutlineText(text, 129, NUMERAL_PIX_X/2 - w1/2, 0, 2);
  pd->graphics->popContext();

  pd->graphics->clearBitmap(m_holeStatsBitmap[1], kColorClear);
  if (!score) { return; }
  int16_t displayScore = par - score;
  if (displayScore < 0) {
    snprintf(text, 128, "%i UNDER", (int)displayScore*-1);
  } else if (displayScore > 0) {
    snprintf(text, 128, "%i OVER", (int)displayScore);
  } else {
    snprintf(text, 128, "PAR");
  }
  const int32_t w2 = pd->graphics->getTextWidth(getRoobert24(), text, 128, kUTF8Encoding, 0);
  pd->graphics->pushContext(m_holeStatsBitmap[1]);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  drawOutlineText(text, 129, (NUMERAL_PIX_X+(NUMERAL_BUF*2))/2 - w2/2, 0, 2);
  pd->graphics->popContext();
}

void initBitmap() {
  pd->graphics->setDrawMode(kDrawModeCopy);
  m_titleSelected = loadImageAtPath("images/titleSelected");
  m_splash = loadImageAtPath("images/splash");
  m_useTheCrankBitmap = loadImageAtPath("images/useTheCrank");
  m_ditherBitmap = loadImageAtPath("images/dither");
  m_holeTutorialBitmap = loadImageAtPath("images/tutorial");

  m_header = loadImageAtPath("images/header");
  m_turretBody = loadImageAtPath("images/turretBody");
  m_turretBarrelTabel = loadImageTableAtPath("images/turretBarrel");
  for (int i = 0; i < 8; ++i) {
    m_turretBarrel[i][0] = pd->graphics->getTableBitmap(m_turretBarrelTabel, i);
    for (int32_t j = 1; j < 256; ++j) {
      const float angle = (365.0f / 256.0f) * j;
      m_turretBarrel[i][j] = pd->graphics->newBitmap(64, 64, kColorClear);
      pd->graphics->pushContext(m_turretBarrel[i][j]);
      pd->graphics->drawRotatedBitmap(m_turretBarrel[i][0], 32, 32, angle, 0.5f, 0.5f, 1.0f, 1.0f);
      pd->graphics->popContext();
    }
  }

  for (int32_t i = 0; i < N_WATERFALLS; ++i) {
    char text[128];
    snprintf(text, 128, "images/falls%i_bg", (int)i);
    m_wfBg[i] = loadImageAtPath(text);
    snprintf(text, 128, "images/falls%i_fg", (int)i);
    m_sheetWfFg[i] = loadImageTableAtPath(text);
  }
  m_wfPond = loadImageAtPath("images/falls_pond");

  for (int s = 0; s < MAX_PEG_SIZE; ++s) {
    const float scale = sizeToScale(s);
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

  for (int s = 0; s < MAX_PEG_SIZE; ++s) {
    const float scale = sizeToScale(s);
    for (int32_t iAngle = 0; iAngle < 128; ++iAngle) {
      m_rectBitmap[0][s][iAngle] = pd->graphics->newBitmap(BOX_MAX*2*scale, BOX_MAX*2*scale, kColorClear);
      pd->graphics->pushContext(m_rectBitmap[0][s][iAngle]);
      drawRotatedRect(BOX_MAX * scale, BOX_MAX * scale, (BOX_WIDTH/2) * scale, (BOX_HEIGHT/2) * scale, iAngle, false);
      pd->graphics->drawRect(0, 0, BOX_MAX*2*scale, BOX_MAX*2*scale, kColorWhite);
      pd->graphics->popContext();
      m_rectBitmap[1][s][iAngle] = pd->graphics->newBitmap(BOX_MAX*2*scale, BOX_MAX*2*scale, kColorClear);
      pd->graphics->pushContext(m_rectBitmap[1][s][iAngle]);
      drawRotatedRect(BOX_MAX * scale, BOX_MAX * scale, (BOX_WIDTH/2) * scale, (BOX_HEIGHT/2) * scale, iAngle, true);
      pd->graphics->drawRect(0, 0, BOX_MAX*2*scale, BOX_MAX*2*scale, kColorWhite);
      pd->graphics->popContext();
    }
  }
  
  m_infoTopperBitmap = pd->graphics->newBitmap(DEVICE_PIX_X, 32, kColorClear);
  m_levelSplashBitmap = pd->graphics->newBitmap(DEVICE_PIX_X, DEVICE_PIX_Y, kColorClear);
  m_scoreHistogram = pd->graphics->newBitmap(DEVICE_PIX_X, DEVICE_PIX_Y, kColorBlack);

  for (int i = 0; i < TURRET_RADIUS; ++i) {
    m_pootAnimation[i] = pd->graphics->newBitmap(TURRET_RADIUS*2, TURRET_RADIUS*2, kColorClear);
    pd->graphics->pushContext(m_pootAnimation[i]);
    pd->graphics->fillEllipse(TURRET_RADIUS-i, TURRET_RADIUS-i, 2*i, 2*i, 0.0f, 360.0f, kColorBlack);
    pd->graphics->popContext();
  }

  m_fontRoobert24 = loadFontAtPath("fonts/Roobert-24-Medium");
  m_fontRoobert10 = loadFontAtPath("fonts/Roobert-10-Bold");
  m_fontGreatvibes24 = loadFontAtPath("fonts/GreatVibes-Regular-24");
  m_fontGreatvibes109 = loadFontAtPath("fonts/GreatVibes-Regular-109");
  pd->graphics->setFont(m_fontGreatvibes24);

  for (int n = 0; n < 10; ++n) {
    m_numeralBitmap[n] = pd->graphics->newBitmap(NUMERAL_PIX_X, NUMERAL_PIX_Y, kColorBlack);
    pd->graphics->pushContext(m_numeralBitmap[n]);
    char text[128];
    snprintf(text, 128, "%i", n);
    const int32_t w = pd->graphics->getTextWidth(getGreatVibes109(), text, 128, kUTF8Encoding, 0);
    setGreatVibes109();
    pd->graphics->setDrawMode(kDrawModeFillBlack);
    drawOutlineText(text, 128, NUMERAL_PIX_X/2 - w/2, 0, 4);
    pd->graphics->popContext();
  }
  m_numeralStencil = pd->graphics->newBitmap(DEVICE_PIX_X, DEVICE_PIX_Y, kColorBlack);
  pd->graphics->pushContext(m_numeralStencil);
  pd->graphics->fillRect(0, (DEVICE_PIX_Y - NUMERAL_PIX_Y)/2, DEVICE_PIX_X, NUMERAL_PIX_Y, kColorWhite);
  pd->graphics->popContext();

  LCDBitmap* tempBitmap = pd->graphics->newBitmap(NUMERAL_PIX_Y, 32, kColorClear);
  pd->graphics->pushContext(tempBitmap);
  setRoobert24();
  const int32_t w1 = pd->graphics->getTextWidth(getRoobert24(), "LEVEL", 128, kUTF8Encoding, 0);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  drawOutlineText("LEVEL", 128, NUMERAL_PIX_Y/2 - w1/2, 0, 2);
  pd->graphics->popContext();

  m_levelBitmap = pd->graphics->newBitmap(32, NUMERAL_PIX_Y, kColorClear);
  m_levelStatsBitmap = pd->graphics->newBitmap(NUMERAL_PIX_X*2, 32, kColorClear);
  m_holeStatsBitmap[0] = pd->graphics->newBitmap(NUMERAL_PIX_X, 32, kColorClear);
  m_holeStatsBitmap[1] = pd->graphics->newBitmap(NUMERAL_PIX_X + (2*NUMERAL_BUF), 32, kColorClear);

  pd->graphics->pushContext(m_levelBitmap);
  pd->graphics->drawRotatedBitmap(tempBitmap, 32/2, NUMERAL_PIX_Y/2, 90.0f, 0.5f, 0.5f, 1.0f, 1.0f);
  pd->graphics->popContext();

  //

  pd->graphics->clearBitmap(tempBitmap, kColorClear);
  pd->graphics->pushContext(tempBitmap);
  setRoobert24();
  const int32_t w2 = pd->graphics->getTextWidth(getRoobert24(), "HOLE", 128, kUTF8Encoding, 0);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  drawOutlineText("HOLE", 128, NUMERAL_PIX_Y/2 - w2/2, 0, 2);
  pd->graphics->popContext();

  m_holeBitmap = pd->graphics->newBitmap(32, NUMERAL_PIX_Y, kColorClear);
  pd->graphics->pushContext(m_holeBitmap);
  pd->graphics->drawRotatedBitmap(tempBitmap, 32/2, NUMERAL_PIX_Y/2, 3*90.0f, 0.5f, 0.5f, 1.0f, 1.0f);
  pd->graphics->popContext();

  //

  pd->graphics->clearBitmap(tempBitmap, kColorClear);
  pd->graphics->pushContext(tempBitmap);
  setRoobert24();
  const int32_t w3 = pd->graphics->getTextWidth(getRoobert24(), "PLAYER", 128, kUTF8Encoding, 0);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  drawOutlineText("PLAYER", 128, NUMERAL_PIX_Y/2 - w3/2, 0, 2);
  pd->graphics->popContext();

  m_playerBitmap = pd->graphics->newBitmap(32, NUMERAL_PIX_Y, kColorClear);
  pd->graphics->pushContext(m_playerBitmap);
  pd->graphics->drawRotatedBitmap(tempBitmap, 32/2, NUMERAL_PIX_Y/2, 3*90.0f, 0.5f, 0.5f, 1.0f, 1.0f);
  pd->graphics->popContext();

  pd->graphics->freeBitmap(tempBitmap);
  tempBitmap = NULL;

  //
  const uint16_t stencilStep = (DEVICE_PIX_Y + (2*WF_DIVISION_PIX_Y)) / STENCIL_WIPE_N; // WF_DIVISION_PIX_Y is the height of the dither
  for (int i = 0; i < STENCIL_WIPE_N; ++i) {
    m_stencilWipeBitmap[i] = pd->graphics->newBitmap(DEVICE_PIX_X, DEVICE_PIX_Y, kColorClear);
    pd->graphics->pushContext(m_stencilWipeBitmap[i]);
    pd->graphics->setDrawMode(kDrawModeInverted); // White is used by the stencil
    pd->graphics->drawBitmap(getBitmapDither(), 0, -WF_DIVISION_PIX_Y + (stencilStep * i), kBitmapFlippedY);
    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->fillRect(0, -WF_DIVISION_PIX_Y, DEVICE_PIX_X, (stencilStep * i), kColorWhite);
    pd->graphics->popContext();
  }

}
