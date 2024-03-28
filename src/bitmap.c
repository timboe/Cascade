#include "bitmap.h"
#include "input.h"
#include "physics.h"

LCDBitmap* m_titleSelected;
LCDBitmap* m_splash;

LCDBitmap* m_header;
LCDBitmap* m_turretBody;
LCDBitmap* m_turretBarrel[8][256];
LCDBitmapTable* m_turretBarrelTabel;

LCDBitmap* m_ballBitmap[MAX_PEG_SIZE];
LCDBitmap* m_rectBitmap[MAX_PEG_SIZE][256];

LCDBitmap* m_wfBg[N_WATERFALLS];
LCDBitmapTable* m_sheetWfFg[N_WATERFALLS];

LCDFont* m_fontRoobert24;
LCDFont* m_fontRoobert10;

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

LCDBitmap* getSpriteSplash() { return m_splash; }

LCDBitmap* getBitmapWfBg(uint8_t wf) { return m_wfBg[wf]; }

LCDBitmap* getBitmapWfFg(uint8_t _wf, uint32_t _x, uint32_t _y) {
  return getBitmapWfFg_byidx(_wf, WF_ID(_x, _y));
}

LCDBitmap* getBitmapWfFg_byidx(uint8_t _wf, uint32_t _idx) {
  return pd->graphics->getTableBitmap(m_sheetWfFg[_wf], _idx);
}

LCDBitmap* getBitmapPeg(const struct Peg_t* p) {
  switch (p->m_shape) {
    case kPegShapeBall: return m_ballBitmap[p->m_size];
    case kPegShapeRect: return m_rectBitmap[p->m_size][p->m_iAngle];
    default: return NULL;
  }
  return NULL;
}

LCDBitmap* getBitmapBall(void) { return m_ballBitmap[0]; }

LCDBitmap* getTitleSelectedBitmap() { return m_titleSelected; }

LCDBitmap* getBitmapTurretBody(void) { return m_turretBody; }

LCDBitmap* getBitmapHeader(void) { return m_header; }

LCDBitmap* getBitmapTurretBarrel(void) {
  return m_turretBarrel[(getFrameCount() % 32) / 4][ angToByte(getTurretBarrelAngle()) ];
}

void setRoobert10() {
  pd->graphics->setFont(m_fontRoobert10);
}

void setRoobert24() {
  pd->graphics->setFont(m_fontRoobert24); 
}

LCDFont* getRoobert24() { return m_fontRoobert24; }

LCDFont* getRoobert10(void) { return m_fontRoobert10; }

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
    char buf[128];
    snprintf(buf, 128, "images/falls%i_bg", (int)i);
    m_wfBg[i] = loadImageAtPath(buf);
    snprintf(buf, 128, "images/falls%i_fg", (int)i);
    m_sheetWfFg[i] = loadImageTableAtPath(buf);
    // TODO - temp
    break;
  }

  for (int s = 0; s < MAX_PEG_SIZE; ++s) {
    const float scale = sizeToScale(s);
    const float ballSize = BALL_RADIUS * 2.0f * scale;
    m_ballBitmap[s] = pd->graphics->newBitmap(ballSize, ballSize, kColorClear);
    pd->graphics->pushContext(m_ballBitmap[s]);
    pd->graphics->fillEllipse(0, 0, ballSize, ballSize, 0.0f, 360.0f, kColorWhite);
    pd->graphics->drawEllipse(0, 0, ballSize, ballSize, 2, 0.0f, 360.0f, kColorBlack);
    pd->graphics->popContext();
  }
  // for (int32_t i = 1; i < 256; ++i) {
  //   const float angle = (365.0f / 256.0f) * i;
  //   m_ballBitmap[i] = pd->graphics->newBitmap(ballSize, ballSize, kColorClear);
  //   pd->graphics->pushContext(m_ballBitmap[i]);
  //   pd->graphics->drawRotatedBitmap(m_ballBitmap[0], ballSize/2, ballSize/2, angle, 0.5f, 0.5f, 1.0f, 1.0f);
  //   pd->graphics->popContext();
  // }

  for (int s = 0; s < MAX_PEG_SIZE; ++s) {
    const float scale = sizeToScale(s);
    m_rectBitmap[s][0] = pd->graphics->newBitmap(BOX_MAX*2*scale, BOX_MAX*2*scale, kColorClear);
    pd->graphics->pushContext(m_rectBitmap[s][0]);
    pd->graphics->fillRect((BOX_MAX - BOX_WIDTH/2)*scale, (BOX_MAX - BOX_HEIGHT/2)*scale, BOX_WIDTH*scale, BOX_HEIGHT*scale, kColorWhite);
    pd->graphics->drawRect((BOX_MAX - BOX_WIDTH/2)*scale, (BOX_MAX - BOX_HEIGHT/2)*scale, BOX_WIDTH*scale, BOX_HEIGHT*scale, kColorBlack);
    // There is no line width option for draw rect
    pd->graphics->drawRect((BOX_MAX - BOX_WIDTH/2)*scale + 1, (BOX_MAX - BOX_HEIGHT/2)*scale + 1, BOX_WIDTH*scale - 2, BOX_HEIGHT*scale - 2, kColorBlack);
    pd->graphics->popContext();
    for (int32_t i = 1; i < 256; ++i) {
      const float angle = (365.0f / 256.0f) * i;
      m_rectBitmap[s][i] = pd->graphics->newBitmap(BOX_MAX*2*scale, BOX_MAX*2*scale, kColorClear);
      pd->graphics->pushContext(m_rectBitmap[s][i]);
      pd->graphics->drawRotatedBitmap(m_rectBitmap[s][0], BOX_MAX*scale, BOX_MAX*scale, angle, 0.5f, 0.5f, 1.0f, 1.0f);
      pd->graphics->popContext();
    }
  }
  
  m_fontRoobert24 = loadFontAtPath("fonts/Roobert-24-Medium");
  m_fontRoobert10 = loadFontAtPath("fonts/Roobert-10-Bold");
  pd->graphics->setFont(m_fontRoobert24);
}