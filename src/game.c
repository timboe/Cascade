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
int16_t m_maximumY = 0;

float m_scrollOffset = 0;
float m_vY = 0;

/// ///

int gameGetFrameCount() { return m_frameCount; }
void gameDoResetFrameCount(void) { m_frameCount = 0; }

void gameSetTurretBarrelAngle(const float angle) { m_turretBarrelAngle = angle; }
float gameGetTurretBarrelAngle(void) { return m_turretBarrelAngle; }

uint16_t gameGetPreviousWaterfall(void) { return m_previousWaterfall; }
void gameDoResetPreviousWaterfall(void) { m_previousWaterfall = IOGetWaterfallForeground(IOGetCurrentLevel(), 0); }

int gameLoop(void* _data) {
  pd->graphics->setBackgroundColor(kColorWhite); // TODO make me black

  if (screenShotGetInProgress()) {
    screenShotDo();
    ++m_frameCount;
    return 1;
  }

  const enum FSM_t fsm = FSMUpdate();
  const enum GameMode_t gm = FSMGetGameMode();

  inputDoHandle(fsm, gm);

  if (!renderGetSubFreeze()) {
    if (gm == kGameWindow) { // TODO eliminate me
      boardDoUpdate();
      physicsDoUpdateSpace(m_frameCount, fsm);
    }
    renderDo(m_frameCount, fsm, gm);
  }

  ++m_frameCount;
  return 1;
}

void menuOptionsCallbackResetSave(void* toReset) {
  pdxlog("menuOptionsCallbackResetSave");
  IOResetPlayerSave(*(uint16_t*)toReset);
}

void menuOptionsCallbackQuitHole(void* _unused) {
  pdxlog("menuOptionsCallbackQuitHole");
  // TODO
}

void menuOptionsCallbackAudio(void* userData) {
  int value = pd->system->getMenuItemValue((PDMenuItem*)userData);
  // if (value == 0) {
  //   music(true);
  //   soundDoSfx(true);
  // } else if (value == 1) {
  //   music(true);
  //   soundDoSfx(false);
  // } else if (value == 2) {
  //   music(false);
  //   soundDoSfx(true);
  // } else {
  //   music(false);
  //   soundDoSfx(false);
  // }
}

void gameDoPopulateMenuTitlesPlayer(void) {
  pd->system->removeAllMenuItems();
  pd->system->addMenuItem("reset slot 1", menuOptionsCallbackResetSave, (void*)0);
  pd->system->addMenuItem("reset slot 2", menuOptionsCallbackResetSave, (void*)1);
  pd->system->addMenuItem("reset slot 3", menuOptionsCallbackResetSave, (void*)2);
}

void gameDoPopulateMenuTitles(void) {
  pd->system->removeAllMenuItems();
  static const char* options[] = {"Music+SFX", "Music", "SFX", "None"};
  PDMenuItem* menu = pd->system->addOptionsMenuItem("audio", options, 4, menuOptionsCallbackAudio, NULL);
  pd->system->setMenuItemUserdata(menu, (void*) menu); // User data is a pointer to the menu itself
}

void gameDoPopulateMenuGame() {
  pd->system->removeAllMenuItems();
  static const char* options[] = {"Music+SFX", "Music", "SFX", "None"};
  PDMenuItem* menu = pd->system->addOptionsMenuItem("audio", options, 4, menuOptionsCallbackAudio, NULL);
  pd->system->setMenuItemUserdata(menu, (void*) menu); // User data is a pointer to the menu itself
  pd->system->addMenuItem("quit hole", menuOptionsCallbackQuitHole, NULL);
}

void gameModScrollVelocity(const float mod) { 
  if (!mod) { return; }
  m_vY += mod;
}

float gameDoApplyScrollEasing(void) {
  m_vY *= SCREEN_FRIC;
  m_scrollOffset += m_vY;

  const float soDiff = SCROLL_OFFSET_MAX - m_scrollOffset;
  if (soDiff < 0) {
    const float toAdd = soDiff * SCREEN_BBACK;
    if (toAdd > -0.1f) { m_scrollOffset = SCROLL_OFFSET_MAX; m_vY = 0; }
    else               { m_scrollOffset += toAdd; }
    // pd->system->logToConsole("BBACK active (BOTTOM) %i from %f to %f by adding %f", gameGetFrameCount(), m_scrollOffset, m_scrollOffset + (soDiff * SCREEN_BBACK), toAdd);
  } else if (m_scrollOffset < m_minimumY) {
    const float toAdd = ((m_scrollOffset - m_minimumY) * -SCREEN_BBACK);
    // pd->system->logToConsole("BBACK active (top) %i from %f to %f by adding %f", gameGetFrameCount(), m_scrollOffset, m_scrollOffset + ((m_scrollOffset - m_minimumY) * -SCREEN_BBACK), toAdd);
    if (toAdd < 0.1f) { m_scrollOffset = m_minimumY; m_vY = 0; }
    else              { m_scrollOffset += toAdd; }
  }

  return m_vY;
}

float gameGetScrollOffset(void) { return m_scrollOffset; }

void gameSetScrollOffset(float set, const bool force) {
  if (force) {
    m_scrollOffset = set;
    return;
  }

  if (set < m_minimumY) set = m_minimumY;
  float diff = set - m_scrollOffset;
  m_scrollOffset += diff * SCREEN_EASING;
  // pd->system->logToConsole("req %f, set %f", set, m_scrollOffset);
}

int16_t gameGetMinimumY(void) { return m_minimumY; }
void gameSetMinimumY(int16_t y) { 
  if (y > WFALL_PIX_Y - DEVICE_PIX_Y) {
    y = WFALL_PIX_Y - DEVICE_PIX_Y; // This is as low as we are allowed to go
  }
  m_minimumY = y;
}

int16_t gameGetMaximumY(void) { return m_maximumY; }
void gameSetMaximimY(const int16_t y) { m_maximumY = y; }

float gameGetParalaxFactorNear(void) { return m_scrollOffset * PARALAX_NEAR; }
float gameGetParalaxFactorFar(void) { return m_scrollOffset * PARALAX_FAR; }

