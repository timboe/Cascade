#include <math.h>

#include "game.h"
#include "board.h"
#include "bitmap.h"
#include "render.h"
#include "sound.h"
#include "input.h"
#include "fsm.h"
#include "io.h"
#include "physics.h"
#include "sshot.h"

int32_t m_frameCount = 0;
float m_turretBarrelAngle = 179.0f;

uint16_t m_previousWaterfall = 0;

int16_t m_minimumY = 0;

float m_scrollOffset = 0;
float m_vY = 0;

////////////

int getFrameCount() { return m_frameCount; }
void resetFrameCount(void) { m_frameCount = 0; }

void setTurretBarrelAngle(const float angle) { m_turretBarrelAngle = angle; }
float getTurretBarrelAngle(void) { return m_turretBarrelAngle; }

uint16_t getPreviousWaterfall(void) { return m_previousWaterfall; }
void resetPreviousWaterfall(void) { m_previousWaterfall = getWaterfallForeground(getCurrentLevel(), 0); }

int gameLoop(void* _data) {
  pd->graphics->setBackgroundColor(kColorWhite); // TODO make me black

  if (getScreenShotInProgress()) {
    doScreenShot();
    ++m_frameCount;
    return 1;
  }

  const enum FSM_t fsm = FSMUpdate();
  const enum GameMode_t gm = FSMGetGameMode();

  clickHandlerReplacement();

  if (!getSubFreeze()) {
    if (gm == kGameWindow) { // TODO eliminate me
      boardUpdate();
      updateSpace(m_frameCount, fsm);
    }
    render(m_frameCount, fsm, gm);
  }

  ++m_frameCount;
  return 1;
}

void menuOptionsCallbackResetSave(void* toReset) {
  pdxlog("menuOptionsCallbackResetSave");
  resetPlayerSave(*(uint16_t*)toReset);
}

void menuOptionsCallbackQuitHole(void* _unused) {
  pdxlog("menuOptionsCallbackQuitHole");
  // TODO
}

void menuOptionsCallbackAudio(void* userData) {
  int value = pd->system->getMenuItemValue((PDMenuItem*)userData);
  // if (value == 0) {
  //   music(true);
  //   sfx(true);
  // } else if (value == 1) {
  //   music(true);
  //   sfx(false);
  // } else if (value == 2) {
  //   music(false);
  //   sfx(true);
  // } else {
  //   music(false);
  //   sfx(false);
  // }
}

void populateMenuTitlesPlayer(void) {
  pd->system->removeAllMenuItems();
  pd->system->addMenuItem("reset slot 1", menuOptionsCallbackResetSave, (void*)0);
  pd->system->addMenuItem("reset slot 2", menuOptionsCallbackResetSave, (void*)1);
  pd->system->addMenuItem("reset slot 3", menuOptionsCallbackResetSave, (void*)2);
}

void populateMenuTitles(void) {
  pd->system->removeAllMenuItems();
  static const char* options[] = {"Music+SFX", "Music", "SFX", "None"};
  PDMenuItem* menu = pd->system->addOptionsMenuItem("audio", options, 4, menuOptionsCallbackAudio, NULL);
  pd->system->setMenuItemUserdata(menu, (void*) menu); // User data is a pointer to the menu itself
}

void populateMenuGame() {
  pd->system->removeAllMenuItems();
  static const char* options[] = {"Music+SFX", "Music", "SFX", "None"};
  PDMenuItem* menu = pd->system->addOptionsMenuItem("audio", options, 4, menuOptionsCallbackAudio, NULL);
  pd->system->setMenuItemUserdata(menu, (void*) menu); // User data is a pointer to the menu itself
  pd->system->addMenuItem("quit hole", menuOptionsCallbackQuitHole, NULL);
}

void modScrollVelocity(const float mod) { 
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

void setScrollOffset(float set, const bool force) {
  if (force) {
    m_scrollOffset = set;
    return;
  }

  if (set < m_minimumY) set = m_minimumY;
  float diff = set - m_scrollOffset;
  m_scrollOffset += diff * SCREEN_EASING;
  // pd->system->logToConsole("req %f, set %f", set, m_scrollOffset);
}

int16_t getMinimumY(void) { return m_minimumY; }

void setMinimumY(int16_t y) { 
  if (y > WFALL_PIX_Y - DEVICE_PIX_Y) {
    y = WFALL_PIX_Y - DEVICE_PIX_Y; // This is as low as we are allowed to go
  }
  m_minimumY = y;
}

float getParalaxFactorNear(void) { return m_scrollOffset * PARALAX_NEAR; }

float getParalaxFactorFar(void) { return m_scrollOffset * PARALAX_FAR; }

char* ftos(const float value, const int16_t size, char* dest) {
  const char* tmpSign = (value < 0) ? "-" : "";
  const float tmpVal = (value < 0) ? -value : value;

  const int16_t tmpInt1 = tmpVal;
  const float tmpFrac = tmpVal - tmpInt1;
  const int16_t tmpInt2 = trunc(tmpFrac * 10000);

  char truncStr[8];
  snprintf (dest, size, "%02d", tmpInt2);
  snprintf (truncStr, 8, "%.2s", dest);

  snprintf (dest, size, "%s%d.%s", tmpSign, tmpInt1, truncStr);
  return dest;
}

void snprintf_c(char* buf, const uint8_t bufSize, const int n) {
  if (n < 1000) {
    snprintf(buf+strlen(buf), bufSize, "%d", n);
    return;
  }
  snprintf_c(buf, bufSize, n / 1000);
  snprintf(buf+strlen(buf), bufSize, ",%03d", n %1000);
}