#include "game.h"
#include "fsm.h"
#include "easing.h"
#include "render.h"
#include "bitmap.h"
#include "io.h"
#include "input.h"

float FSMCommonCrankNumeral(float* progress);

/// ///

float FSMCommonCrankNumeral(float* progress) {
  static uint16_t crankNotMoved = 0;
  const float cc = inputGetCrankChanged();
  float ret = 0.0f;
  *progress -= cc * CRANK_NUMBERSCROLL_MODIFIER;
  if      (inputGetPressed(kButtonUp)) *progress += 10 * CRANK_NUMBERSCROLL_MODIFIER;
  else if (inputGetPressed(kButtonDown)) *progress -=  10 * CRANK_NUMBERSCROLL_MODIFIER;
  if (fabsf(cc) < 1.0f) { ++crankNotMoved; }
  else { crankNotMoved = 0; }
  if (crankNotMoved > TICK_FREQUENCY/4) { *progress *= 0.9f; }
  if (*progress >= 1.0f) {
    *progress -= 2.0f;
    ret = 1.0f;
  } else if (*progress < -1.0f) {
    *progress += 2.0f;
    ret = -1.0f;
  }
  renderSetNumeralOffset(*progress);
  return ret;
}

void FSMDisplayTitles(const bool newState) {
  if (newState) { 
    gameDoPopulateMenuTitles();
  }
  if (!pd->system->isCrankDocked() && !IOGetIsPreloading()) {
    return FSMDo(kTitlesFSM_TitlesToChoosePlayer);
  }
}

void FSMTitlesToChoosePlayer(const bool newState) {
  static int16_t timer = 0;
  if (newState) { timer = 0; }
  FSMDoCommonScrollTo(DEVICE_PIX_Y, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChoosePlayer); }
}

void FSMChoosePlayer(const bool newState) {
  static float progress = 0.0f;
  if (newState) {
    progress = 0.0f;
    gameDoPopulateMenuTitlesPlayer();
  }
  const float status = FSMCommonCrankNumeral(&progress);
  if      (status > 0) {
    gameDoResetPreviousWaterfall();
    IODoPreviousPlayer();
    IODoGoToNextUnplayedLevel();
  } else if (status < 0) {
    gameDoResetPreviousWaterfall();
    IODoNextPlayer();
    IODoGoToNextUnplayedLevel();
  }
}

void FSMChoosePlayerToChooseLevel(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    renderSetNumeralOffset(0.0f);
    bitmapDoUpdateLevelStatsBitmap();
    gameDoResetPreviousWaterfall();
  }
  FSMDoCommonScrollTo(DEVICE_PIX_Y * 2, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChooseLevel); }
}

void FSMChooseLevel(const bool newState) {
  static float progress = 0.0f;
  if (newState) {
    progress = 0.0f;
    gameDoPopulateMenuTitles();
  }
  const float status = FSMCommonCrankNumeral(&progress);
  if (status > 0) {
    gameDoResetPreviousWaterfall(); // Ordering important
    IODoPreviousLevel();
    bitmapDoUpdateLevelStatsBitmap();
  } else if (status < 0) {
    gameDoResetPreviousWaterfall();
    IODoNextLevel();
    bitmapDoUpdateLevelStatsBitmap();
  }
}

void FSMChooseLevelToChooseHole(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    renderSetNumeralOffset(0.0f);
    bitmapDoUpdateHoleStatsBitmap();
  }
  FSMDoCommonScrollTo(DEVICE_PIX_Y * 3, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChooseHole); }
}

void FSMChooseLevelToChoosePlayer(const bool newState) {
  static int16_t timer = 0;
  if (newState) {
    timer = 0;
    renderSetNumeralOffset(0.0f);
  }
  FSMDoCommonScrollTo(DEVICE_PIX_Y, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChoosePlayer); }
}

void FSMChooseHole(const bool newState) {
  static float progress = 0.0f;
  if (newState) progress = 0.0f;
  const float status = FSMCommonCrankNumeral(&progress);
  if (status > 0) {
    IODoPreviousHole(); // Ordering important
    bitmapDoUpdateHoleStatsBitmap();
  } else if (status < 0) {
    IODoNextHole();
    bitmapDoUpdateHoleStatsBitmap();
  }
}

void FSMChooseHoleToSplash(const bool newState) {
  static int16_t timer = 0;
  if (newState) {
    timer = 0;
    renderSetNumeralOffset(0.0f);
    bitmapDoUpdateGameInfoTopper();
    bitmapDoUpdateLevelSplash(); 
  }
  FSMDoCommonScrollTo(DEVICE_PIX_Y * 4, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kGameFSM_DisplaySplash); }
}

void FSMChooseHoleToChooseLevel(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    renderSetNumeralOffset(0.0f);
    gameDoResetPreviousWaterfall();
  }
  FSMDoCommonScrollTo(DEVICE_PIX_Y * 2, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChooseLevel); }
}