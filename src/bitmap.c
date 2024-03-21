#include "bitmap.h"
#include "input.h"
#include "physics.h"

LCDBitmap* m_titleSelected;

LCDBitmap* m_splash;

LCDBitmap* m_turretBody;

LCDBitmap* m_turretBarrel[256];

LCDBitmap* m_ballBitmap[256];

LCDBitmap* m_boxBitmap[256];

LCDBitmapTable* m_sheetWfBg[N_WATERFALLS];

LCDBitmapTable* m_sheetWfFg[N_WATERFALLS];

LCDFont* m_fontRoobert24;

LCDFont* m_fontRoobert10;

LCDBitmap* loadImageAtPath(const char* _path);

LCDBitmapTable* loadImageTableAtPath(const char* _path);

LCDFont* loadFontAtPath(const char* _path);

uint8_t radToByte(float _rad);

/// ///

uint8_t radToByte(float _rad) {
  // /pd->system->logToConsole("angle %f", _rad);
  //return 0;
  return (_rad / (M_PIf * 2.0f)) * 128.0f;
}

uint8_t angToByte(float _ang) {
  return (_ang / 360.0f) * 256.0f;
}

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

LCDBitmap* getSpriteSplash() { return m_splash; }


LCDBitmap* getBitmapWfBg(uint8_t _wf, uint32_t _x, uint32_t _y) {
  return getBitmapWfBg_byidx(_wf, WF_ID(_x, _y));
}

LCDBitmap* getBitmapWfBg_byidx(uint8_t _wf, uint32_t _idx) {
  return pd->graphics->getTableBitmap(m_sheetWfBg[_wf], _idx);
}

LCDBitmap* getBitmapWfFg(uint8_t _wf, uint32_t _x, uint32_t _y) {
  return getBitmapWfFg_byidx(_wf, WF_ID(_x, _y));
}

LCDBitmap* getBitmapWfFg_byidx(uint8_t _wf, uint32_t _idx) {
  return pd->graphics->getTableBitmap(m_sheetWfFg[_wf], _idx);
}

LCDBitmap* getBitmapBall(float _angle) {
  return m_ballBitmap[radToByte(_angle)];
}

LCDBitmap* getBitmapBox(float _angle) {
  return m_boxBitmap[radToByte(_angle)];
}

LCDBitmap* getTitleSelectedBitmap() {
  return m_titleSelected;
}

LCDBitmap* getBitmapTurretBody(void) {
  return m_turretBody;
}

LCDBitmap* getBitmapTurretBarrel(void) {
  return m_turretBarrel[ angToByte(getTurretBarrelAngle()) ];
}

void setRoobert10() {
  pd->graphics->setFont(m_fontRoobert10);
}

void setRoobert24() {
  pd->graphics->setFont(m_fontRoobert24); 
}

LCDFont* getRoobert24() {
  return m_fontRoobert24;
}

LCDFont* getRoobert10(void) {
  return m_fontRoobert10;
}

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

void initSprite() {
  pd->graphics->setDrawMode(kDrawModeCopy);
  m_titleSelected = loadImageAtPath("images/titleSelected");
  m_splash = loadImageAtPath("images/splash");

  m_turretBody = loadImageAtPath("images/turretBody");
  m_turretBarrel[0] = loadImageAtPath("images/turretBarrel");
  // m_turretBarrelRotated = pd->graphics->newBitmap(64, 64, kColorClear);
  for (int32_t i = 1; i < 256; ++i) {
    const float angle = (365.0f / 256.0f) * i;
    m_turretBarrel[i] = pd->graphics->newBitmap(64, 64, kColorClear);
    pd->graphics->pushContext(m_turretBarrel[i]);
    pd->graphics->drawRotatedBitmap(m_turretBarrel[0], 32, 32, angle, 0.5f, 0.5f, 1.0f, 1.0f);
    pd->graphics->popContext();
  }

  for (int32_t i = 0; i < N_WATERFALLS; ++i) {
    char buf[128];
    snprintf(buf, 128, "images/falls%i_bg", (int)i);
    m_sheetWfBg[i] = loadImageTableAtPath(buf);
    snprintf(buf, 128, "images/falls%i_fg", (int)i);
    m_sheetWfFg[i] = loadImageTableAtPath(buf);
    // TODO - temp
    break;
  }

  const float ballSize = BALL_RADIUS * 2.0f;
  m_ballBitmap[0] = pd->graphics->newBitmap(ballSize, ballSize, kColorClear);
  pd->graphics->pushContext(m_ballBitmap[0]);
  pd->graphics->fillEllipse(0, 0, ballSize, ballSize, 0.0f, 360.0f, kColorWhite);
  pd->graphics->drawEllipse(0, 0, ballSize, ballSize, 1, 0.0f,          360.0f,        kColorBlack);
  pd->graphics->drawEllipse(0, 0, ballSize, ballSize, 1, 0.0f,          360.0f,        kColorBlack);
  pd->graphics->drawEllipse(0, 0, ballSize, ballSize, 3, 0.0f,          0.0f + 10.0f, kColorBlack);
  pd->graphics->drawEllipse(0, 0, ballSize, ballSize, 3, 0.0f + 180.0f, 0.0f + 190,   kColorBlack);
  pd->graphics->popContext();
  for (int32_t i = 1; i < 256; ++i) {
    const float angle = (365.0f / 256.0f) * i;
    m_ballBitmap[i] = pd->graphics->newBitmap(ballSize, ballSize, kColorClear);
    pd->graphics->pushContext(m_ballBitmap[i]);
    pd->graphics->drawRotatedBitmap(m_ballBitmap[0], ballSize/2, ballSize/2, angle, 0.5f, 0.5f, 1.0f, 1.0f);
    pd->graphics->popContext();
  }

  const float BOX_MAX = MAX(BOX_WIDTH, BOX_HEIGHT) * SQRT_HALF;
  m_boxBitmap[0] = pd->graphics->newBitmap(BOX_MAX*2, BOX_MAX*2, kColorWhite);
  pd->graphics->pushContext(m_boxBitmap[0]);
  // cpVect p1 = cpv(-BOX_WIDTH / 2.0f, -BOX_HEIGHT / 2.0f);
  // cpVect p2 = cpv(-BOX_WIDTH / 2.0f, BOX_HEIGHT / 2.0f);
  // cpVect p3 = cpv(BOX_WIDTH / 2.0f, BOX_HEIGHT / 2.0f);
  // cpVect p4 = cpv(BOX_WIDTH / 2.0f, -BOX_HEIGHT / 2.0f);
  // const float p1x = p1.x - p1.y;
  // const float p1y = p1.x + p1.y;
  // const float p2x = p2.x - p2.y;
  // const float p2y = p2.x + p2.y;
  // const float p3x = p3.x - p3.y;
  // const float p3y = p3.x + p3.y;
  // const float p4x = p4.x - p4.y;
  // const float p4y = p4.x + p4.y;
  // pd->graphics->setLineCapStyle(kLineCapStyleSquare);
  // pd->graphics->drawLine(p1x, p1y, p2x, p2y, 1, kColorBlack);
  // pd->graphics->drawLine(p2x, p2y, p3x, p3y, 1, kColorBlack);
  // pd->graphics->drawLine(p3x, p3y, p4x, p4y, 1, kColorBlack);
  // pd->graphics->drawLine(p4x, p4y, p1x, p1y, 1, kColorBlack);
  pd->graphics->fillRect(BOX_MAX - BOX_WIDTH/2, BOX_MAX - BOX_HEIGHT/2, BOX_WIDTH, BOX_HEIGHT, kColorWhite);
  pd->graphics->drawRect(BOX_MAX - BOX_WIDTH/2, BOX_MAX - BOX_HEIGHT/2, BOX_WIDTH, BOX_HEIGHT, kColorBlack);
  pd->graphics->popContext();
  for (int32_t i = 1; i < 256; ++i) {
    const float angle = (365.0f / 256.0f) * i;
    m_boxBitmap[i] = pd->graphics->newBitmap(BOX_MAX*2, BOX_MAX*2, kColorClear);
    pd->graphics->pushContext(m_boxBitmap[i]);
    pd->graphics->drawRotatedBitmap(m_boxBitmap[0], BOX_MAX, BOX_MAX, angle, 0.5f, 0.5f, 1.0f, 1.0f);
    pd->graphics->popContext();
  }
  
  m_fontRoobert24 = loadFontAtPath("fonts/Roobert-24-Medium");
  m_fontRoobert10 = loadFontAtPath("fonts/Roobert-10-Bold");
  pd->graphics->setFont(m_fontRoobert24);
}