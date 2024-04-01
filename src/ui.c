#include <limits.h>
#include <math.h>
#include "ui.h"
#include "bitmap.h"
#include "input.h"
#include "render.h"
#include "sound.h"
#include "io.h"
#include "board.h"

enum kGameMode m_mode = 0;

float m_scrollOffset = 0;
int16_t m_minimumY = 0;
float m_vY = 0;
bool m_goToTop = false;
bool m_ballEndSweep = false;
float m_endSweepProgress = 0;
float m_popLevel = 0;

// LCDSprite* m_UISpriteSave = NULL;
// LCDSprite* m_UISpriteSaveLoadProgress = NULL;
// LCDSprite* m_UISpriteLoad = NULL;
// LCDSprite* m_UISpriteGen = NULL;

LCDBitmap* m_UIBitmapSave = NULL;
LCDBitmap* m_UIBitmapSaveLoadProgress = NULL;
LCDBitmap* m_UIBitmapLoad = NULL;
LCDBitmap* m_UIBitmapGen = NULL;

// Title Screen
// LCDSprite* m_UISpriteSplash = NULL;
// LCDSprite* m_UISpriteTitleVersion = NULL;
LCDBitmap* m_UIBitmapTitleVersion = NULL;
// LCDSprite* m_UISpriteTitleSelected = NULL;
int16_t m_UITitleSelected = 0;
// LCDSprite* m_UISpriteTitleNew[3] = {NULL};
// LCDSprite* m_UISpriteTitleCont[3] = {NULL};
LCDBitmap* m_UIBitmapTitleNew[3] = {NULL};
LCDBitmap* m_UIBitmapTitleCont[3] = {NULL};

/// ///

int16_t getMinimumY(void) { return m_minimumY; }

void setMinimumY(int16_t y) { m_minimumY = y; }

enum kGameMode getGameMode() {
  return m_mode;
}

LCDBitmap* getTitleNewGameBitmap(uint8_t _i) {
  return m_UIBitmapTitleNew[_i];
}

// LCDSprite* getSaveSprite() { return m_UISpriteSave; } 

// LCDSprite* getLoadSprite() { return m_UISpriteLoad; }

// LCDSprite* getGenSprite() { return m_UISpriteGen; }

void snprintf_c(char* _buf, uint8_t _bufSize, int _n) {
    if (_n < 1000) {
        snprintf(_buf+strlen(_buf), _bufSize, "%d", _n);
        return;
    }
    snprintf_c(_buf, _bufSize, _n / 1000);
    snprintf(_buf+strlen(_buf), _bufSize, ",%03d", _n %1000);
}

void modTitleCursor(bool _increment) {
  //sfx(kSfxD);
  // if (_increment) {
  //   m_UITitleSelected = (m_UITitleSelected == N_SAVES-1 ? 0 : m_UITitleSelected + 1);
  // } else {
  //   m_UITitleSelected = (m_UITitleSelected == 0 ? N_SAVES-1 : m_UITitleSelected - 1);
  // }  
}


uint16_t getTitleCursorSelected() {
  return m_UITitleSelected;
}

void updateUITitles(int fc) {
  // pd->sprite->setDrawMode(m_UISpriteSplash, kDrawModeCopy);

  // pd->sprite->setVisible(m_UISpriteTitleSelected, _fc % (TICK_FREQUENCY/2) < TICK_FREQUENCY/4);
  // pd->sprite->moveTo(m_UISpriteTitleSelected, 
  //   DEVICE_PIX_X/2, 
  //   DEVICE_PIX_Y - TILE_PIX*2);
  // pd->sprite->moveTo(m_UISpriteSplash, DEVICE_PIX_X/2, DEVICE_PIX_Y/2 - (3*TILE_PIX)/4 );
}

void updateUI(int fc) {

}


void setGameMode(enum kGameMode _mode) {
  m_mode = _mode;
  if (m_mode == kTitles) return;

}

void resetUI() {
  //#ifdef DEMO
  // XXX CAUTION
  m_UITitleSelected = 1;
  //#else
  //m_UITitleSelected = 0;
  //#endif
}

void initiUI() {


  // m_UISpriteSave = pd->sprite->newSprite();
  // m_UISpriteSaveLoadProgress = pd->sprite->newSprite();
  // m_UISpriteLoad = pd->sprite->newSprite();
  // m_UISpriteGen = pd->sprite->newSprite();

  m_UIBitmapSave = pd->graphics->newBitmap(DEVICE_PIX_X/2, TILE_PIX*2, kColorClear);
  m_UIBitmapSaveLoadProgress = pd->graphics->newBitmap(TILE_PIX*6, TILE_PIX*1, kColorClear);
  m_UIBitmapGen = pd->graphics->newBitmap(DEVICE_PIX_X/2, TILE_PIX*2, kColorClear);
  m_UIBitmapLoad = pd->graphics->newBitmap(DEVICE_PIX_X/2, TILE_PIX*2, kColorClear);

  // PDRect boundTopB = {.x = 0, .y = 0, .width = DEVICE_PIX_X/2, .height = TILE_PIX*2};
  // PDRect boundSpriteSave = {.x = 0, .y = 0, .width = TILE_PIX*6, .height = TILE_PIX};


  // pd->sprite->setBounds(m_UISpriteSave, boundTopB);
  // pd->sprite->setImage(m_UISpriteSave, m_UIBitmapSave, kBitmapUnflipped);
  // pd->sprite->moveTo(m_UISpriteSave, DEVICE_PIX_X/2, DEVICE_PIX_Y/2);
  // pd->sprite->setZIndex(m_UISpriteSave, 128);
  // pd->sprite->setIgnoresDrawOffset(m_UISpriteSave, 1);
  // pd->sprite->setVisible(m_UISpriteSave, 1);

  // pd->sprite->setBounds(m_UISpriteSaveLoadProgress, boundSpriteSave);
  // pd->sprite->setImage(m_UISpriteSaveLoadProgress, m_UIBitmapSaveLoadProgress, kBitmapUnflipped);
  // pd->sprite->moveTo(m_UISpriteSaveLoadProgress, DEVICE_PIX_X/2, DEVICE_PIX_Y/2 + 2*TILE_PIX);
  // pd->sprite->setZIndex(m_UISpriteSaveLoadProgress, Z_INDEX_UI_TT);
  // pd->sprite->setIgnoresDrawOffset(m_UISpriteSaveLoadProgress, 1);
  // pd->sprite->setVisible(m_UISpriteSaveLoadProgress, 1);

  pd->graphics->pushContext(m_UIBitmapSave);
  pd->graphics->setLineCapStyle(kLineCapStyleRound);
  pd->graphics->drawLine(TILE_PIX, TILE_PIX, DEVICE_PIX_X/2 - TILE_PIX, TILE_PIX, TILE_PIX*2, kColorWhite);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  setRoobert24();
  int32_t tlen = pd->graphics->getTextWidth(getRoobert24(), "SAVING", 16, kASCIIEncoding, 0);
  pd->graphics->drawText("SAVING", 16, kASCIIEncoding, (DEVICE_PIX_X/2 - tlen)/2, 0);
  pd->graphics->popContext();

  // pd->sprite->setBounds(m_UISpriteGen, boundTopB);
  // pd->sprite->setImage(m_UISpriteGen, m_UIBitmapGen, kBitmapUnflipped);
  // pd->sprite->moveTo(m_UISpriteGen, DEVICE_PIX_X/2, DEVICE_PIX_Y/2);
  // pd->sprite->setZIndex(m_UISpriteGen, Z_INDEX_UI_TT);
  // pd->sprite->setIgnoresDrawOffset(m_UISpriteGen, 1);
  // pd->sprite->setVisible(m_UISpriteGen, 1);

  pd->graphics->pushContext(m_UIBitmapGen);
  pd->graphics->setLineCapStyle(kLineCapStyleRound);
  pd->graphics->drawLine(TILE_PIX, TILE_PIX, DEVICE_PIX_X/2 - TILE_PIX, TILE_PIX, TILE_PIX*2, kColorBlack);
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  setRoobert24();
  tlen = pd->graphics->getTextWidth(getRoobert24(), "GENERATING", 16, kASCIIEncoding, 0);
  pd->graphics->drawText("GENERATING", 16, kASCIIEncoding, (DEVICE_PIX_X/2 - tlen)/2, 0);
  pd->graphics->popContext();

  // pd->sprite->setBounds(m_UISpriteLoad, boundTopB);
  // pd->sprite->setImage(m_UISpriteLoad, m_UIBitmapLoad, kBitmapUnflipped);
  // pd->sprite->moveTo(m_UISpriteLoad, DEVICE_PIX_X/2, DEVICE_PIX_Y/2);
  // pd->sprite->setZIndex(m_UISpriteLoad, Z_INDEX_UI_TT);
  // pd->sprite->setIgnoresDrawOffset(m_UISpriteLoad, 1);
  // pd->sprite->setVisible(m_UISpriteLoad, 1);

  pd->graphics->pushContext(m_UIBitmapLoad);
  pd->graphics->setLineCapStyle(kLineCapStyleRound);
  pd->graphics->drawLine(TILE_PIX, TILE_PIX, DEVICE_PIX_X/2 - TILE_PIX, TILE_PIX, TILE_PIX*2, kColorWhite);
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  setRoobert24();
  tlen = pd->graphics->getTextWidth(getRoobert24(), "LOADING", 16, kASCIIEncoding, 0);
  pd->graphics->drawText("LOADING", 16, kASCIIEncoding, (DEVICE_PIX_X/2 - tlen)/2, 0);
  pd->graphics->popContext();
  pd->graphics->setDrawMode(kDrawModeCopy);


  // // Setup menu screens

  // PDRect splashBound = {.x = 0, .y = 0, .width = TILE_PIX*8, .height = TILE_PIX*8};
  // PDRect deviceBound = {.x = 0, .y = 0, .width = DEVICE_PIX_X, .height = DEVICE_PIX_Y};
  // PDRect buttonBound = {.x = 0, .y = 0, .width = TILE_PIX*7, .height = TILE_PIX};

  // // Titles

  // m_UISpriteSplash = pd->sprite->newSprite();
  // pd->sprite->setBounds(m_UISpriteSplash, deviceBound);
  // pd->sprite->setImage(m_UISpriteSplash, getSpriteSplash(), kBitmapUnflipped);
  // pd->sprite->setZIndex(m_UISpriteSplash, Z_INDEX_UI_M);
  // pd->sprite->setIgnoresDrawOffset(m_UISpriteSplash, 1);  
  // pd->sprite->moveTo(m_UISpriteSplash, DEVICE_PIX_X/2, DEVICE_PIX_Y/2);

  m_UIBitmapTitleVersion = pd->graphics->newBitmap(TILE_PIX*2, TILE_PIX*1, kColorWhite);
  pd->graphics->pushContext(m_UIBitmapTitleVersion);
  setRoobert10();
  int32_t width = pd->graphics->getTextWidth(getRoobert10(), VERSION, 5, kASCIIEncoding, 0);
  pd->graphics->drawText(VERSION, 5, kASCIIEncoding, TILE_PIX - width/2, 0);
  pd->graphics->popContext();

  // m_UISpriteTitleVersion = pd->sprite->newSprite();
  // PDRect vBound = {.x = 0, .y = 0, .width = TILE_PIX*2, .height = TILE_PIX*1};
  // pd->sprite->setBounds(m_UISpriteTitleVersion, buttonBound);
  // pd->sprite->setImage(m_UISpriteTitleVersion, m_UIBitmapTitleVersion, kBitmapUnflipped);
  // pd->sprite->setZIndex(m_UISpriteTitleVersion, Z_INDEX_UI_T);
  // pd->sprite->setIgnoresDrawOffset(m_UISpriteTitleVersion, 1);
  // pd->sprite->moveTo(m_UISpriteTitleVersion, 6*TILE_PIX, TILE_PIX/2);

  // m_UISpriteTitleSelected = pd->sprite->newSprite();
  // pd->sprite->setBounds(m_UISpriteTitleSelected, buttonBound);
  // pd->sprite->setImage(m_UISpriteTitleSelected, getTitleSelectedBitmap(), kBitmapUnflipped);
  // pd->sprite->setZIndex(m_UISpriteTitleSelected, Z_INDEX_UI_M);
  // pd->sprite->setIgnoresDrawOffset(m_UISpriteTitleSelected, 1);  
  // pd->sprite->moveTo(m_UISpriteTitleSelected, (7*TILE_PIX)/2 + TILE_PIX, DEVICE_PIX_Y + TILE_PIX/2);

  setRoobert10();
  for (int32_t i = 0; i < 3; ++i) {
    m_UIBitmapTitleNew[i] = pd->graphics->newBitmap(TILE_PIX*7, TILE_PIX*1, kColorClear);
    // m_UISpriteTitleNew[i] = pd->sprite->newSprite();
    // pd->sprite->setBounds(m_UISpriteTitleNew[i], buttonBound);
    // pd->sprite->setImage(m_UISpriteTitleNew[i], m_UIBitmapTitleNew[i], kBitmapUnflipped);
    // pd->sprite->setZIndex(m_UISpriteTitleNew[i], Z_INDEX_UI_M);
    // pd->sprite->setIgnoresDrawOffset(m_UISpriteTitleNew[i], 1);  
    // pd->sprite->moveTo(m_UISpriteTitleNew[i], i*TILE_PIX*7 + (7*TILE_PIX)/2 + (i+1)*TILE_PIX, DEVICE_PIX_Y - 2*TILE_PIX);
    
    m_UIBitmapTitleCont[i] = pd->graphics->newBitmap(TILE_PIX*7, TILE_PIX*1, kColorClear);
    // m_UISpriteTitleCont[i] = pd->sprite->newSprite();
    // pd->sprite->setBounds(m_UISpriteTitleCont[i], buttonBound);
    // pd->sprite->setImage(m_UISpriteTitleCont[i], m_UIBitmapTitleCont[i], kBitmapUnflipped);
    // pd->sprite->setZIndex(m_UISpriteTitleCont[i], Z_INDEX_UI_M);
    // pd->sprite->setIgnoresDrawOffset(m_UISpriteTitleCont[i], 1);  
    // pd->sprite->moveTo(m_UISpriteTitleCont[i], i*TILE_PIX*7 + (7*TILE_PIX)/2 + (i+1)*TILE_PIX, DEVICE_PIX_Y - 2*TILE_PIX);

    pd->graphics->pushContext(m_UIBitmapTitleNew[i]);
    pd->graphics->setDrawMode(kDrawModeFillBlack);
    char text[32];
    snprintf(text, 32, "New Game");
    int16_t len = strlen(text);
    int32_t width = pd->graphics->getTextWidth(getRoobert10(), text, len, kASCIIEncoding, 0);
    pd->graphics->drawText(text, len, kASCIIEncoding, (7*TILE_PIX)/2 - width/2, 0);
    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->popContext();

    pd->graphics->pushContext(m_UIBitmapTitleCont[i]);
    pd->graphics->setDrawMode(kDrawModeFillWhite);
    #ifdef DEMO
    snprintf(text, 32, "Load Demo");
    #else
    snprintf(text, 32, "Continue");
    #endif
    len = strlen(text);
    width = pd->graphics->getTextWidth(getRoobert10(), text, len, kASCIIEncoding, 0);
    pd->graphics->drawText(text, len, kASCIIEncoding, (7*TILE_PIX)/2 - width/2, 0);
    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->popContext();
  }

}

void modScrollVelocity(float mod) { 
  if (!mod) { return; }
  m_vY += mod;
  m_goToTop = false;
}

void setScrollToTop(bool stt) { m_goToTop = stt; }

void applyScrollEasing(void) {
  m_vY *= SCREEN_FRIC;
  m_scrollOffset += m_vY;

  if (m_goToTop && !m_ballEndSweep) {
    pd->system->logToConsole("go to top active %i", getFrameCount());
    setScrollOffset(m_minimumY);
    if (fabsf(m_scrollOffset - m_minimumY) <= 1.0f) {
      m_goToTop = false;
    }
  }

  const float soDiff = SCROLL_OFFSET_MAX - m_scrollOffset;
  if (soDiff < 0) {
    const float toAdd = soDiff * SCREEN_BBACK;
    if (toAdd > -0.1f) { m_scrollOffset = SCROLL_OFFSET_MAX; m_vY = 0; }
    else               { m_scrollOffset += toAdd; }
    pd->system->logToConsole("BBACK active (BOTTOM) %i from %f to %f by adding %f", getFrameCount(), m_scrollOffset, m_scrollOffset + (soDiff * SCREEN_BBACK), toAdd);
  } else if (m_scrollOffset < m_minimumY) {
    const float toAdd = ((m_scrollOffset - m_minimumY) * -SCREEN_BBACK);
    pd->system->logToConsole("BBACK active (top) %i from %f to %f by adding %f", getFrameCount(), m_scrollOffset, m_scrollOffset + ((m_scrollOffset - m_minimumY) * -SCREEN_BBACK), toAdd);
    if (toAdd < 0.1f) { m_scrollOffset = m_minimumY; m_vY = 0; }
    else              { m_scrollOffset += toAdd; }
  }
}

float getScrollOffset(void) { return m_scrollOffset; }

void setScrollOffset(float set) {
  if (set < m_minimumY) set = m_minimumY;
  float diff = set - m_scrollOffset;
  m_scrollOffset += diff * SCREEN_EASING;
  // pd->system->logToConsole("req %f, set %f", set, m_scrollOffset);
}

void activateBallEndSweep(void) {
  m_ballEndSweep = true;
  m_endSweepProgress = 0.0f;

  int32_t minY = 10000;
  for (uint32_t i = 0; i < MAX_PEGS; ++i) {
    const struct Peg_t* peg = getPeg(i);
    if (peg->m_y < minY && peg->m_state == kPegStateActive) {
      minY = peg->m_y;
    }
  }
  if (minY > (DEVICE_PIX_Y/2)) {
    m_minimumY = minY - (DEVICE_PIX_Y/2);
  }
  pd->system->logToConsole("smallest y was %i, min y is now %i", minY, m_minimumY);

}

void doBallEndSweep(void) {
  if (!m_ballEndSweep) return;
  // Take less time overall when we get lower down
  const float distance_mod = WFALL_PIX_Y / (float)(WFALL_PIX_Y - m_minimumY);

  m_endSweepProgress += (TIMESTEP * END_SWEEP_SCALE * distance_mod);
  float progress = getEasing(EaseInOutSine, m_endSweepProgress);
  progress = (1.0f - progress);

  // const float target = SCROLL_OFFSET_MAX * progress;
  // setScrollOffset(target);

  // m_popLevel = target + (progress * DEVICE_PIX_Y);
  // popBoard(m_popLevel);
  // // pd->system->logToConsole("end sweep active target %f, pop %f",target, m_popLevel);

  const float target_partial = m_minimumY + ((SCROLL_OFFSET_MAX - m_minimumY) * progress);
  setScrollOffset(target_partial);

  const float m_popLevel = WFALL_PIX_Y * progress;
  popBoard(m_popLevel);
  // pd->system->logToConsole("end sweep active target %f, pop %f",target, m_popLevel);

  if (m_endSweepProgress >= 1) {
    m_ballEndSweep = false;
    m_scrollOffset = m_minimumY;
  }
}

void renderBallEndSweep(void) {
  if (!m_ballEndSweep) return;
  pd->graphics->drawLine(0, m_popLevel, DEVICE_PIX_X, m_popLevel, 4, kColorWhite);
}

float getParalaxFactorNear() { return m_scrollOffset * PARALAX_NEAR; }

float getParalaxFactorFar() { return m_scrollOffset * PARALAX_FAR; }