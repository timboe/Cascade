#include "game.h"
#include "fsm.h"
#include "easing.h"
#include "render.h"
#include "bitmap.h"
#include "io.h"
#include "input.h"

float commonCrankNumeral(float* progress);

///

float commonCrankNumeral(float* progress) {
  float ret = 0.0f;
  *progress -= getCrankChanged() * CRANK_NUMBERSCROLL_MODIFIER;
  if      (getPressed(kButtonUp)) *progress += 10 * CRANK_NUMBERSCROLL_MODIFIER;
  else if (getPressed(kButtonDown)) *progress -=  10 * CRANK_NUMBERSCROLL_MODIFIER;    
  *progress *= 0.99f;
  if (*progress >= 1.0f) {
    *progress -= 2.0f;
    ret = 1.0f;
  } else if (*progress < -1.0f) {
    *progress += 2.0f;
    ret = -1.0f;
  }
  setNumeralOffset(*progress);
  return ret;
}

void FSMDisplayTitles(const bool newState) {
  if (newState) { 
    populateMenuTitles();
  }
  if (!pd->system->isCrankDocked()) { return FSMDo(kTitlesFSM_TitlesToChoosePlayer); }
}

void FSMTitlesToChoosePlayer(const bool newState) {
  static int16_t timer = 0;
  if (newState) { timer = 0; }
  FSMCommonScrollTo(DEVICE_PIX_Y, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChoosePlayer); }
}

void FSMChoosePlayer(const bool newState) {
  static float progress = 0.0f;
  if (newState) {
    progress = 0.0f;
    populateMenuTitlesPlayer();
  }
  const float status = commonCrankNumeral(&progress);
  if      (status > 0) {
    resetPreviousWaterfall();
    doPreviousPlayer();
    goToNextUnplayedLevel();
  } else if (status < 0) {
    resetPreviousWaterfall();
    doNextPlayer();
    goToNextUnplayedLevel();
  }
}

void FSMChoosePlayerToChooseLevel(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    setNumeralOffset(0.0f);
    updateLevelStatsBitmap();
    resetPreviousWaterfall();
  }
  FSMCommonScrollTo(DEVICE_PIX_Y * 2, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChooseLevel); }
}

void FSMChooseLevel(const bool newState) {
  static float progress = 0.0f;
  if (newState) {
    progress = 0.0f;
    populateMenuTitles();
  }
  const float status = commonCrankNumeral(&progress);
  if (status > 0) {
    resetPreviousWaterfall(); // Ordering important
    doPreviousLevel();
    updateLevelStatsBitmap();
  } else if (status < 0) {
    resetPreviousWaterfall();
    doNextLevel();
    updateLevelStatsBitmap();
  }
}

void FSMChooseLevelToChooseHole(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    setNumeralOffset(0.0f);
    updateHoleStatsBitmap();
  }
  FSMCommonScrollTo(DEVICE_PIX_Y * 3, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChooseHole); }
}

void FSMChooseLevelToChoosePlayer(const bool newState) {
  static int16_t timer = 0;
  if (newState) {
    timer = 0;
    setNumeralOffset(0.0f);
  }
  FSMCommonScrollTo(DEVICE_PIX_Y, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChoosePlayer); }
}

void FSMChooseHole(const bool newState) {
  static float progress = 0.0f;
  if (newState) progress = 0.0f;
  const float status = commonCrankNumeral(&progress);
  if (status > 0) {
    doPreviousHole(); // Ordering important
    updateHoleStatsBitmap();
  } else if (status < 0) {
    doNextHole();
    updateHoleStatsBitmap();
  }
}

void FSMChooseHoleToSplash(const bool newState) {
  static int16_t timer = 0;
  if (newState) {
    timer = 0;
    setNumeralOffset(0.0f);
    updateInfoTopperBitmap();
    updateLevelSplashBitmap(); 
  }
  FSMCommonScrollTo(DEVICE_PIX_Y * 4, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kGameFSM_DisplaySplash); }
}

void FSMChooseHoleToChooseLevel(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    setNumeralOffset(0.0f);
    resetPreviousWaterfall();
  }
  FSMCommonScrollTo(DEVICE_PIX_Y * 2, (float)timer/TIME_TITLE_TRANSITION, kEaseInOutQuad);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChooseLevel); }
}