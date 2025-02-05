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
float m_turretBarrelAngle = 180.0f;

uint16_t m_previousWaterfallFg = 1;
uint16_t m_previousWaterfallBg = 1;

int16_t m_minimumY = 0;

float m_xOffset = 0.0f;
float m_yOffset = 0.0f;
float m_vY = 0;

bool m_yClamped = true;

/// ///

int32_t gameGetFrameCount() { return m_frameCount; }

void gameDoResetFrameCount(void) { 
  m_frameCount = 0;
  soundResetPling();
}

void gameSetTurretBarrelAngle(const float angle) { m_turretBarrelAngle = angle; }
float gameGetTurretBarrelAngle(void) { return m_turretBarrelAngle; }

uint16_t gameGetPreviousWaterfallFg(void) { return m_previousWaterfallFg; }
uint16_t gameGetPreviousWaterfallBg(void) { return m_previousWaterfallBg; }

void gameDoResetPreviousWaterfall(void) { 
  m_previousWaterfallFg = IOGetCurrentHoleWaterfallForeground(FSMGetGameMode());
  m_previousWaterfallBg = IOGetCurrentHoleWaterfallBackground(FSMGetGameMode());
}

int gameLoop(void* _data) {
  pd->graphics->setBackgroundColor(kColorBlack);
  // if (FSMGet() == kGameFSM_GutterToTurret) pd->system->logToConsole(">>> gameLoop FC (loop start) %i", gameGetFrameCount());


#ifdef TAKE_SCREENSHOTS
  if (screenShotGetInProgress()) {
    screenShotDo();
    ++m_frameCount;
    return 1;
  }

  static bool doingScreenshots = true;
  if (doingScreenshots && !IOGetIsPreloading()) {
    static bool first = true; // Don't increment the level on the first call
    if (first) { 
      first = false;
      IOSetLevelHole(0, 0);
    } else {
      IODoNextHoleWithLevelWrap();
      if (IOGetCurrentHole() == 0 && IOGetCurrentLevel() == 0) {
        doingScreenshots = false;
        FSMDo(kTitlesFSM_DisplayTitles); // Reset
        gameSetYOffset(0, true);
        return 1;
      }
    } 
    inputSetCrankAngle(180.0f);
    FSMDo(kGameFSM_DisplayLevelTitle); // To load the level
    FSMDo(kGameFSM_AimMode); // To be rendering the level
    boardDoUpdate(); // To have called update at least once on elliptic and line peg paths
    screenShotInit();
    return 1;
  }
#endif

  IODoUpdatePreloading();

  m_yClamped = true; // Default assumption, may be invalidated by FSM (prior to render)
  const enum FSM_t fsm = FSMUpdate();
  const enum GameMode_t gm = FSMGetGameMode();

  inputDoHandle(fsm, gm);

  if (!renderGetSubFreeze()) {
    if (gm == kGameWindow) {
      boardDoUpdate();
      physicsDoUpdate(m_frameCount, fsm);
    }
    renderDo(m_frameCount, fsm, gm);
    soundDoVolumes(fsm, gm);
    if (rand() % BIRDCALL_SFX_CHANCE == 0 && IOGetCurrentHoleWaterfallBackground(gm) > 0) { 
      soundDoSfx(kBirdSfx1);
    }
    ++m_frameCount;
  }

  // if (FSMGet() == kGameFSM_GutterToTurret) pd->system->logToConsole("<<< gameLoop FC (loop end) %i", gameGetFrameCount());
  return 1;
}

void gameMenuStateSafetyReset(void) {
  physicsSetTimestepMultiplier(1.0f);
  renderSetScale(1);
  gameSetXOffset(0);
  soundStopSfx(kDrumRollSfx1);
}

void menuOptionsCallbackResetSave(void* toReset) {
  #ifdef DEV
  pd->system->logToConsole("menuOptionsCallbackResetSave %i", (uintptr_t)toReset);
  #endif
  IOResetPlayerSave((uintptr_t)toReset);
}

void menuOptionsCallbackQuitHole(void* _unused) {
  #ifdef DEV
  pd->system->logToConsole("menuOptionsCallbackQuitHole");
  #endif
  gameMenuStateSafetyReset();
  pd->system->removeAllMenuItems();
  FSMDo(kGameFSM_GameFadeOutQuit);
}

void menuOptionsCallbackResetHole(void* _unused) {
  #ifdef DEV
  pd->system->logToConsole("menuOptionsCallbackResetHole");
  #endif
  gameMenuStateSafetyReset();
  pd->system->removeAllMenuItems();
  FSMDo(kGameFSM_GameFadeOutReset);
}

void menuOptionsCallbackCredits(void* _unused) {
  #ifdef DEV
  pd->system->logToConsole("menuOptionsCallbackCredits");
  #endif
  gameMenuStateSafetyReset();
  pd->system->removeAllMenuItems();
  FSMDo(kTitlesFSM_ToTitleCreditsTitle);
}

void menuOptionsCallbackAudio(void* userData) {
  int32_t value = pd->system->getMenuItemValue((PDMenuItem*)userData);
  #ifdef DEV
  pd->system->logToConsole("menuOptionsCallbackAudio %i", value);
  #endif
  soundSetSetting(value);
}

void gameDoPopulateMenuTitlesPlayer(void) {
  pd->system->removeAllMenuItems();
  pd->system->addMenuItem("reset slot 1", menuOptionsCallbackResetSave, (void*)0);
  pd->system->addMenuItem("reset slot 2", menuOptionsCallbackResetSave, (void*)1);
  pd->system->addMenuItem("reset slot 3", menuOptionsCallbackResetSave, (void*)2);
}

void gameDoPopulateMenuTitles(void) {
  pd->system->removeAllMenuItems();
  static const char* options[] = {"water+sfx", "water", "sfx", "none"};
  PDMenuItem* menu = pd->system->addOptionsMenuItem("audio", options, 4, menuOptionsCallbackAudio, NULL);
  pd->system->setMenuItemUserdata(menu, (void*) menu); // User data is a pointer to the menu itself
  pd->system->setMenuItemValue(menu, soundGetSetting());
  pd->system->addMenuItem("credits", menuOptionsCallbackCredits, NULL);
}

void gameDoPopulateMenuGame() {
  pd->system->removeAllMenuItems();
  static const char* options[] = {"water+sfx", "water", "sfx", "none"};
  PDMenuItem* menu = pd->system->addOptionsMenuItem("audio", options, 4, menuOptionsCallbackAudio, NULL);
  pd->system->setMenuItemUserdata(menu, (void*) menu); // User data is a pointer to the menu itself
  pd->system->setMenuItemValue(menu, soundGetSetting());
  pd->system->addMenuItem("quit hole", menuOptionsCallbackQuitHole, NULL);
  pd->system->addMenuItem("reset hole", menuOptionsCallbackResetHole, NULL);
}

void gameModYVelocity(const float mod) { 
  if (!mod) { return; }
  m_vY += mod;
}

float gameDoApplyYEasing(void) {
  m_vY *= SCREEN_FRIC;
  m_yOffset += m_vY;

  const float scrollOffsetMax = IOGetCurrentHoleHeight() - DEVICE_PIX_Y + OVERSCROLL_HEIGHT;
  const float soDiff = scrollOffsetMax - m_yOffset;
  if (soDiff < 0) {
    const float toAdd = soDiff * SCREEN_BBACK;
    if (toAdd > -0.1f) { m_yOffset = scrollOffsetMax; m_vY = 0; }
    else               { m_yOffset += toAdd; }
    // pd->system->logToConsole("BBACK active (BOTTOM) %i from %f to %f by adding %f", gameGetFrameCount(), m_yOffset, m_yOffset + (soDiff * SCREEN_BBACK), toAdd);
  } else if (m_yOffset < m_minimumY) {
    const float toAdd = ((m_yOffset - m_minimumY) * -SCREEN_BBACK);
    // pd->system->logToConsole("BBACK active (top) %i from %f to %f by adding %f", gameGetFrameCount(), m_yOffset, m_yOffset + ((m_yOffset - m_minimumY) * -SCREEN_BBACK), toAdd);
    if (toAdd < 0.1f) { m_yOffset = m_minimumY; m_vY = 0; }
    else              { m_yOffset += toAdd; }
  }

  if (fabsf(m_vY) < 0.1f) { m_vY = 0.0f; }
  if (m_vY) { m_yClamped = false; }

  return m_vY;
}

float gameGetYOffset(void) { return m_yOffset; }

void gameSetYOffset(float set, const bool force) {
  if (force) {
    m_yOffset = set;
    return;
  }

  if (set < m_minimumY) set = m_minimumY;
  float diff = set - m_yOffset;
  m_yOffset += diff * SCREEN_EASING;
  m_yClamped = false;
}

bool gameGetYClamped(void) { return m_yClamped; }
void gameSetYNotClamped(void) { m_yClamped = false; };

void gameSetXOffset(const float set) { m_xOffset = set; }
float gameGetXOffset(void) { return m_xOffset; }

int16_t gameGetMinimumY(void) { return m_minimumY; }
void gameSetMinimumY(int16_t y) { 
  if (y > IOGetCurrentHoleHeight() - DEVICE_PIX_Y) {
    y = IOGetCurrentHoleHeight() - DEVICE_PIX_Y; // This is as low as we are allowed to go
  }
  m_minimumY = y;
}

float gameGetParalaxFactorNearForY(const bool hard, const float y) { return y * (hard ? PARALLAX_HARD_NEAR : PARALLAX_GENTLE_NEAR); }

float gameGetParalaxFactorNear(const bool hard) { return m_yOffset * (hard ? PARALLAX_HARD_NEAR : PARALLAX_GENTLE_NEAR); }
float gameGetParalaxFactorFar(const bool hard) { return m_yOffset * (hard ? PARALLAX_HARD_FAR : PARALLAX_GENTLE_FAR); }
