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
  float ret = 0.0f;
  *progress -= inputGetCrankChanged() * CRANK_NUMBERSCROLL_MODIFIER;
  if      (inputGetPressed(kButtonUp)) *progress += 10 * CRANK_NUMBERSCROLL_MODIFIER;
  else if (inputGetPressed(kButtonDown)) *progress -=  10 * CRANK_NUMBERSCROLL_MODIFIER;    
  *progress *= 0.99f;
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
  if (!pd->system->isCrankDocked()) { return FSMDo(kTitlesFSM_TitlesToChoosePlayer); }
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
    updateLevelStatsBitmap();
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
    updateLevelStatsBitmap();
  } else if (status < 0) {
    gameDoResetPreviousWaterfall();
    IODoNextLevel();
    updateLevelStatsBitmap();
  }
}

void FSMChooseLevelToChooseHole(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    renderSetNumeralOffset(0.0f);
    updateHoleStatsBitmap();
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
    updateHoleStatsBitmap();
  } else if (status < 0) {
    IODoNextHole();
    updateHoleStatsBitmap();
  }
}

void FSMChooseHoleToSplash(const bool newState) {
  static int16_t timer = 0;
  if (newState) {
    timer = 0;
    renderSetNumeralOffset(0.0f);
    updateInfoTopperBitmap();
    updateLevelSplashBitmap(); 
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