#include "game.h"
#include "fsm.h"
#include "easing.h"
#include "render.h"
#include "bitmap.h"
#include "io.h"
#include "input.h"
#include "sound.h"
#include "patterns.h"

float FSMCommonCrankNumeral(float* progress);

/// ///

float FSMCommonCrankNumeral(float* progress) {
  static uint16_t crankNotMoved = 0;
  const float cc = inputGetCrankChanged();
  float ret = 0.0f;
  *progress -= cc * CRANK_NUMBERSCROLL_MODIFIER;
  //
  if (inputGetPressed(kButtonUp)) {
    *progress += 10 * DPAD_NUMBERSCROLL_MODIFIER;
    crankNotMoved = 0;
  } else if (inputGetPressed(kButtonDown)) {
    *progress -=  10 * DPAD_NUMBERSCROLL_MODIFIER;
    crankNotMoved = 0;
  }
  //
  if (fabsf(cc) < 1.0f) { ++crankNotMoved; }
  else { crankNotMoved = 0; }
  //
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
    // Don't set level & hole here - it will trigger the music too early (in preloading)
    // Good to keep this a noop if possible
    // Reset can go in FSMDisplayTitlesWFadeIn
  }
  if (!pd->system->isCrankDocked() && !IOGetIsPreloading()) {
    return FSMDo(kTitlesFSM_TitlesToChoosePlayer);
  }
}

void FSMDisplayTitlesWFadeIn(const bool newState) {
  static int8_t progress;
  if (newState) {
    IOSetLevelHole(0, 0);
    IOSetPlayer(0);
    gameSetYOffset(0, /*force = */true);
    progress = FADE_LEVELS + 2; // +2 to give extra pause 
  }
  if (gameGetFrameCount() % TICK_FREQUENCY / 2 == 0) { --progress; }
  renderSetFadeLevel(progress < FADE_LEVELS ? progress : FADE_LEVELS-1);
  if (progress == -1) {
    gameDoPopulateMenuTitles();
    return FSMDo(kTitlesFSM_DisplayTitles);
  }
}

void FSMTitlesToChoosePlayer(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    soundDoSfx(kWhooshSfx1);
  }
  FSMDoCommonScrollTo(0, DEVICE_PIX_Y, (float)timer/TIME_TITLE_TRANSITION, EASE_TITLE_DOWNWARDS);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChoosePlayer); }
}

void FSMChoosePlayer(const bool newState) {
  static float progress = 0.0f;
  if (newState) {
    progress = 0.0f;
    gameDoResetPreviousWaterfall();
    IODoGoToNextUnplayedLevel();
    gameDoPopulateMenuTitlesPlayer();
    renderDoUpdateBacklines();
  }
  const float status = FSMCommonCrankNumeral(&progress);
  if      (status > 0) {
    gameDoResetPreviousWaterfall();
    #ifdef DEV
    pd->system->logToConsole("^^^^^^^^^^^^^^");
    #endif
    IODoPreviousPlayer();
    soundDoSfx(kTumblerClickSfx);
  } else if (status < 0) {
    gameDoResetPreviousWaterfall();
    #ifdef DEV
    pd->system->logToConsole("vvvvvvvvvvvvv");
    #endif
    IODoNextPlayer();
    soundDoSfx(kTumblerClickSfx);
  }
}

void FSMChoosePlayerToChooseLevel(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    renderSetNumeralOffset(0.0f);
    bitmapDoUpdateLevelStatsBitmap();
    gameDoResetPreviousWaterfall();
    gameDoPopulateMenuTitles();
    soundDoSfx(kWhooshSfx1);
  }
  FSMDoCommonScrollTo(DEVICE_PIX_Y, DEVICE_PIX_Y*2, (float)timer/TIME_TITLE_TRANSITION, EASE_TITLE_DOWNWARDS);
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
    // IODoPreviousLevel();
    IOSetLevelHole(IOGetPreviousLevel(), 0);
    bitmapDoUpdateLevelStatsBitmap();
    soundDoSfx(kTumblerClickSfx);
  } else if (status < 0) {
    gameDoResetPreviousWaterfall();
    // IODoNextLevel();
    IOSetLevelHole(IOGetNextLevel(), 0);
    bitmapDoUpdateLevelStatsBitmap();
    soundDoSfx(kTumblerClickSfx);
  }
}

void FSMChooseLevelToChooseHole(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    renderSetNumeralOffset(0.0f);
    bitmapDoUpdateHoleStatsBitmap();
    soundDoSfx(kWhooshSfx1);
  }
  FSMDoCommonScrollTo(DEVICE_PIX_Y*2, DEVICE_PIX_Y*3, (float)timer/TIME_TITLE_TRANSITION, EASE_TITLE_DOWNWARDS);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChooseHole); }
}

void FSMChooseLevelToChoosePlayer(const bool newState) {
  static int16_t timer = 0;
  if (newState) {
    timer = 0;
    renderSetNumeralOffset(0.0f);
    soundDoSfx(kWhooshSfx1);
  }
  FSMDoCommonScrollTo(DEVICE_PIX_Y*2, DEVICE_PIX_Y, (float)timer/TIME_TITLE_TRANSITION, EASE_TITLE_UPWARDS);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChoosePlayer); }
}

void FSMChooseHole(const bool newState) {
  static float progress = 0.0f;
  if (newState) {
    progress = 0.0f;
    gameDoPopulateMenuTitles();
  }
  const float status = FSMCommonCrankNumeral(&progress);
  if (status > 0) {
    IODoPreviousHole(); // Ordering important
    bitmapDoUpdateHoleStatsBitmap();
    soundDoSfx(kTumblerClickSfx);
  } else if (status < 0) {
    IODoNextHole();
    bitmapDoUpdateHoleStatsBitmap();
    soundDoSfx(kTumblerClickSfx);
  }
}

void FSMChooseHoleWFadeIn(const bool newState) {
  static int8_t progress = FADE_LEVELS - 1;
  if (newState) {
    gameSetYOffset(DEVICE_PIX_Y*3, /*force = */true);
    renderSetNumeralOffset(0.0f);
    bitmapDoUpdateHoleStatsBitmap();
    progress = FADE_LEVELS + 4; // Start a bit higher to let the background transition before we fade in
    soundDoSfx(kWhooshSfx1);
  }
  if (gameGetFrameCount() % ( (TICK_FREQUENCY/2) / FADE_LEVELS) == 0) { --progress; }
  renderSetFadeLevel(progress < FADE_LEVELS ? progress : FADE_LEVELS - 1);
  if (progress == -1) { return FSMDo(kTitlesFSM_ChooseHole); }
}

void FSMChooseHoleToLevelTitle(const bool newState) {
  static int16_t timer = 0;
  if (newState) {
    timer = 0;
    renderSetNumeralOffset(0.0f);
    bitmapDoUpdateGameInfoTopper();
    bitmapDoUpdateLevelTitle();
    soundDoSfx(kWhooshSfx1);
  }
  FSMDoCommonScrollTo(DEVICE_PIX_Y*3, DEVICE_PIX_Y*5, (float)timer/TIME_TITLE_HOLE_TO_LEVELTITLE, EASE_TITLE_HOLE_TO_SPLASH);
  if (timer++ == TIME_TITLE_HOLE_TO_LEVELTITLE) { return FSMDo(kGameFSM_DisplayLevelTitle); }
}

void FSMChooseHoleToChooseLevel(const bool newState) {
  static int16_t timer = 0;
  if (newState) { 
    timer = 0;
    renderSetNumeralOffset(0.0f);
    gameDoResetPreviousWaterfall();
    soundDoSfx(kWhooshSfx1);
  }
  FSMDoCommonScrollTo(DEVICE_PIX_Y*3, DEVICE_PIX_Y*2, (float)timer/TIME_TITLE_TRANSITION, EASE_TITLE_UPWARDS);
  if (timer++ == TIME_TITLE_TRANSITION) { return FSMDo(kTitlesFSM_ChooseLevel); }
}

void FSMToTitlesCreditsTitle(const bool newState) {  // Note: Separate Game and Title versions of this
  static int16_t timer = 0;
  static int16_t origin = 0;
  static bool once = false;
  if (newState) {
    timer = 0;
    soundDoSfx(kWhooshSfx1);
    once = false;
    origin = gameGetYOffset(); // Allow for a custom origin as "do credits" might be called from either screen
  }
  if (!once && gameGetYOffset() > DEVICE_PIX_Y*4) {
    once = true;
    IOSetLevelHole(MAX_LEVELS, 0); // Enabled credits mode. Delay this until the level select is no longer visible
    renderSetNumeralOffset(0.0f);
    bitmapDoUpdateGameInfoTopper();
    bitmapDoUpdateLevelTitle(); 
  }
  FSMDoCommonScrollTo(origin, DEVICE_PIX_Y*5, (float)timer/TIME_TITLE_HOLE_TO_LEVELTITLE, EASE_TITLE_HOLE_TO_SPLASH);
  if (timer++ == TIME_TITLE_HOLE_TO_LEVELTITLE) { 
    soundPlayMusic(10);
    return FSMDo(kGameFSM_DisplayLevelTitle);
  }
}