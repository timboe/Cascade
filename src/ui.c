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
float m_popLevel = 0; // TODO remove me

/// ///

int16_t getMinimumY(void) { return m_minimumY; }

void setMinimumY(int16_t y) { 
  if (y > WFALL_PIX_Y - DEVICE_PIX_Y) {
    y = WFALL_PIX_Y - DEVICE_PIX_Y; // This is as low as we are allowed to go
  }
  m_minimumY = y;
}

enum kGameMode getGameMode() {
  return m_mode;
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


void updateUI(int fc) {

}


void setGameMode(enum kGameMode _mode) {
  m_mode = _mode;
  if (m_mode == kTitles) return;

}

void resetUI() {
}

void initiUI() {


}

void modScrollVelocity(float mod) { 
  if (!mod) { return; }
  m_vY += mod;
}

float applyScrollEasing(void) {
  m_vY *= SCREEN_FRIC;
  m_scrollOffset += m_vY;

  const float soDiff = SCROLL_OFFSET_MAX - m_scrollOffset;
  if (soDiff < 0) {
    const float toAdd = soDiff * SCREEN_BBACK;
    if (toAdd > -0.1f) { m_scrollOffset = SCROLL_OFFSET_MAX; m_vY = 0; }
    else               { m_scrollOffset += toAdd; }
    // pd->system->logToConsole("BBACK active (BOTTOM) %i from %f to %f by adding %f", getFrameCount(), m_scrollOffset, m_scrollOffset + (soDiff * SCREEN_BBACK), toAdd);
  } else if (m_scrollOffset < m_minimumY) {
    const float toAdd = ((m_scrollOffset - m_minimumY) * -SCREEN_BBACK);
    // pd->system->logToConsole("BBACK active (top) %i from %f to %f by adding %f", getFrameCount(), m_scrollOffset, m_scrollOffset + ((m_scrollOffset - m_minimumY) * -SCREEN_BBACK), toAdd);
    if (toAdd < 0.1f) { m_scrollOffset = m_minimumY; m_vY = 0; }
    else              { m_scrollOffset += toAdd; }
  }

  return m_vY;
}

float getScrollOffset(void) { return m_scrollOffset; }

void setScrollOffset(float set, bool force) {
  if (force) {
    m_scrollOffset = set;
    return;
  }

  if (set < m_minimumY) set = m_minimumY;
  float diff = set - m_scrollOffset;
  m_scrollOffset += diff * SCREEN_EASING;
  // pd->system->logToConsole("req %f, set %f", set, m_scrollOffset);
}

void renderDebug(void) {
  pd->graphics->drawLine(0, 0, DEVICE_PIX_X, 0, 4, kColorBlack);
  pd->graphics->drawLine(0, 0, DEVICE_PIX_X, 0, 2, kColorWhite);

  pd->graphics->drawLine(0, WFALL_PIX_Y, DEVICE_PIX_X, WFALL_PIX_Y, 4, kColorBlack);
  pd->graphics->drawLine(0, WFALL_PIX_Y, DEVICE_PIX_X, WFALL_PIX_Y, 2, kColorWhite);

  pd->graphics->drawLine(0, m_popLevel, DEVICE_PIX_X, m_popLevel, 4, kColorWhite);
}

float getParalaxFactorNear() { return m_scrollOffset * PARALAX_NEAR; }

float getParalaxFactorFar() { return m_scrollOffset * PARALAX_FAR; }