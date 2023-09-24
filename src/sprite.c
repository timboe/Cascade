#include "sprite.h"
#include "input.h"


LCDBitmap* m_titleSelected;

LCDBitmap* m_splash;

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


// LCDBitmap* getSprite32(uint32_t _x, uint32_t _y) {
//   return getSprite32_byidx(SPRITE32_ID(_x, _y));
// }

// LCDBitmap* getSprite32_byidx(uint32_t _idx) {
//   return pd->graphics->getTableBitmap(m_sheet32, _idx);
// }

LCDBitmap* getTitleSelectedBitmap() {
  return m_titleSelected;
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

void initSprite() {
  pd->graphics->setDrawMode(kDrawModeCopy);
  m_titleSelected = loadImageAtPath("images/titleSelected");
  m_splash = loadImageAtPath("images/splash");

  m_fontRoobert24 = loadFontAtPath("fonts/Roobert-24-Medium");
  m_fontRoobert10 = loadFontAtPath("fonts/Roobert-10-Bold");
  pd->graphics->setFont(m_fontRoobert24);
}
