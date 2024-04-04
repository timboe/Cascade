#include "bitmap.h"
#include "input.h"
#include "physics.h"
#include "io.h"

LCDBitmap* m_titleSelected;
LCDBitmap* m_splash;

LCDBitmap* m_header;
LCDBitmap* m_turretBody;
LCDBitmap* m_turretBarrel[8][256];
LCDBitmapTable* m_turretBarrelTabel;
LCDBitmap* m_infoTopperBitmap;
LCDBitmap* m_levelSplashBitmap;

LCDBitmap* m_scoreHistogram = NULL;

LCDBitmap* m_pootAnimation[TURRET_RADIUS];

LCDBitmap* m_ballBitmap[2][MAX_PEG_SIZE];
LCDBitmap* m_rectBitmap[2][MAX_PEG_SIZE][128];

LCDBitmap* m_wfFront;
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
  snprintf(text, 128, "Par %i", (int)getCurrentLevelPar());
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
  snprintf(text, 128, "%i9~%i9", (int)getCurrentLevel(), (int)getCurrentHole());
  setGreatVibes24();
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  pd->graphics->drawText(text, 128, kUTF8Encoding, 0, 8);
  setRoobert24();
  const int32_t width = pd->graphics->getTextWidth(getRoobert24(), "By Tim Martin", 128, kUTF8Encoding, 0);
  pd->graphics->drawText("By Tim Martin", 128, kUTF8Encoding, DEVICE_PIX_X - width, 2);
  pd->graphics->popContext();
}

LCDBitmap* getBitmapAnimPoot(uint8_t i) { return m_pootAnimation[i]; }

LCDBitmap* getSpriteSplash() { return m_splash; }

LCDBitmap* getBitmapWfFront() { return m_wfFront; }

LCDBitmap* getBitmapWfBg(uint8_t wf) { return m_wfBg[wf]; }

LCDBitmap* getBitmapWfFg(uint8_t _wf, uint32_t _x, uint32_t _y) {
  return getBitmapWfFg_byidx(_wf, WF_ID(_x, _y));
}

LCDBitmap* getBitmapWfFg_byidx(uint8_t _wf, uint32_t _idx) {
  return pd->graphics->getTableBitmap(m_sheetWfFg[_wf], _idx);
}

LCDBitmap* getBitmapPeg(const struct Peg_t* p) {
  switch (p->m_shape) {
    case kPegShapeBall: return m_ballBitmap[p->m_type == 0 ? 0 : 1][p->m_size];
    case kPegShapeRect: return m_rectBitmap[p->m_type == 0 ? 0 : 1][p->m_size][p->m_iAngle % 128]; // Symmetry
    default: return NULL;
  }
  return NULL;
}

LCDBitmap* getBitmapBall(void) { return m_ballBitmap[0][0]; }

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

void initBitmap() {
  pd->graphics->setDrawMode(kDrawModeCopy);
  m_titleSelected = loadImageAtPath("images/titleSelected");
  m_splash = loadImageAtPath("images/splash");

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
    // TODO - temp
    break;
  }
  m_wfFront = loadImageAtPath("images/falls_fg");

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
    m_rectBitmap[0][s][0] = pd->graphics->newBitmap(BOX_MAX*2*scale, BOX_MAX*2*scale, kColorClear);
    pd->graphics->pushContext(m_rectBitmap[0][s][0]);
    pd->graphics->fillRect((BOX_MAX - BOX_WIDTH/2)*scale, (BOX_MAX - BOX_HEIGHT/2)*scale, BOX_WIDTH*scale, BOX_HEIGHT*scale, kColorWhite);
    pd->graphics->drawRect((BOX_MAX - BOX_WIDTH/2)*scale, (BOX_MAX - BOX_HEIGHT/2)*scale, BOX_WIDTH*scale, BOX_HEIGHT*scale, kColorBlack);
    // There is no line width option for draw rect
    pd->graphics->drawRect((BOX_MAX - BOX_WIDTH/2)*scale + 1, (BOX_MAX - BOX_HEIGHT/2)*scale + 1, BOX_WIDTH*scale - 2, BOX_HEIGHT*scale - 2, kColorBlack);
    pd->graphics->popContext();
    m_rectBitmap[1][s][0] = pd->graphics->newBitmap(BOX_MAX*2*scale, BOX_MAX*2*scale, kColorClear);
    pd->graphics->pushContext(m_rectBitmap[1][s][0]);
    pd->graphics->fillRect((BOX_MAX - BOX_WIDTH/2)*scale, (BOX_MAX - BOX_HEIGHT/2)*scale, BOX_WIDTH*scale, BOX_HEIGHT*scale, kColorWhite);
    pd->graphics->fillRect((BOX_MAX - BOX_WIDTH/2)*scale, (BOX_MAX - BOX_HEIGHT/2)*scale, BOX_WIDTH*scale, BOX_HEIGHT*scale, (uintptr_t)kGreyPattern);
    pd->graphics->drawRect((BOX_MAX - BOX_WIDTH/2)*scale, (BOX_MAX - BOX_HEIGHT/2)*scale, BOX_WIDTH*scale, BOX_HEIGHT*scale, kColorBlack);
    pd->graphics->drawRect((BOX_MAX - BOX_WIDTH/2)*scale + 1, (BOX_MAX - BOX_HEIGHT/2)*scale + 1, BOX_WIDTH*scale - 2, BOX_HEIGHT*scale - 2, kColorBlack);
    pd->graphics->popContext();
    for (int32_t i = 1; i < 128; ++i) {
      const float angle = (180.0f / 128.0f) * i;
      m_rectBitmap[0][s][i] = pd->graphics->newBitmap(BOX_MAX*2*scale, BOX_MAX*2*scale, kColorClear);
      pd->graphics->pushContext(m_rectBitmap[0][s][i]);
      pd->graphics->drawRotatedBitmap(m_rectBitmap[0][s][0], BOX_MAX*scale, BOX_MAX*scale, angle, 0.5f, 0.5f, 1.0f, 1.0f);
      pd->graphics->popContext();
      m_rectBitmap[1][s][i] = pd->graphics->newBitmap(BOX_MAX*2*scale, BOX_MAX*2*scale, kColorClear);
      pd->graphics->pushContext(m_rectBitmap[1][s][i]);
      pd->graphics->drawRotatedBitmap(m_rectBitmap[1][s][0], BOX_MAX*scale, BOX_MAX*scale, angle, 0.5f, 0.5f, 1.0f, 1.0f);
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

  updateInfoTopperBitmap();
  updateLevelSplashBitmap();
  updateScoreHistogramBitmap();
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