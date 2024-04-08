#include <math.h>

#include "game.h"
#include "board.h"
#include "bitmap.h"
#include "render.h"
#include "sound.h"
#include "input.h"
#include "ui.h"
#include "io.h"
#include "physics.h"

int32_t m_frameCount = 0;
uint16_t m_ballCount = 0; // Keeping track of the current level score
uint16_t m_ballStuckCounter = 0; // Keeping track of a stuck ball
float m_turretBarrelAngle = 180.0f;

uint16_t m_previousWaterfall = 0;

enum kFSM m_FSM = (enum kFSM)0;

enum kFSM doFSM_Titles(bool newState);
enum kFSM doFSM_Game(bool newState);

////////////

int getFrameCount() { return m_frameCount; }

enum kFSM getFSM() { return m_FSM; }

void resetBallStuckCounter(void) { m_ballStuckCounter = 0; }

float getTurretBarrelAngle(void) { return m_turretBarrelAngle; }

uint16_t getPreviousWaterfall(void) { return m_previousWaterfall; }

void resetPreviousWaterfall(void) { m_previousWaterfall = getWaterfallForeground(getCurrentLevel(), 0); }

int gameLoop(void* _data) {
  ++m_frameCount;
  pd->graphics->setBackgroundColor(kColorWhite); // TODO make me black

  // if (IOOperationInProgress()) { 
  //   enactIO();
  //   return 1;
  // }

  const enum kGameMode gm = getGameMode();
  m_FSM = doFSM(m_FSM);

  clickHandlerReplacement();

  if (!getSubFreeze()) {

    if (gm == kGameWindow) { // TODO eliminate me

      updateBoard();
      updateSpace(m_frameCount, m_FSM);

    }

    render(m_frameCount, m_FSM);

  }

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

// Call prior to loading anything
void reset() {
  resetUI();
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

bool ballInPlay(void) {
  return (m_FSM >= kGameFSM_BallInPlay && m_FSM <= kGameFSM_BallGutter);
}

void commonScrollAndBounceBack(void) {
  // Button based
  float diffY = 0;
  if      (getPressed(kButtonUp)) diffY = -SCREEN_ACC;
  else if (getPressed(kButtonDown)) diffY =  SCREEN_ACC;
  modScrollVelocity(diffY);
  // Crank based
  static float angle = 180.0f;
  static bool topLock = true;
  static float revDetection = 180.0f;
  // Backup non-crank
  if (pd->system->isCrankDocked()) {
    if      (getPressed(kButtonLeft)) angle += 1.0f;
    else if (getPressed(kButtonRight)) angle -= 1.0f;
  } else {
    angle = getCrankAngle();
  }

  const bool newRev = fabsf(angle - revDetection) > 180.0f;
  revDetection = angle;

  const float so = getScrollOffset();
  if ((newRev && angle < 180.0f) || so > getMinimumY()) {
    topLock = false;
  } else if (so <= getMinimumY()) {
    topLock = true;
  }

  if (!topLock) modScrollVelocity(getCrankChanged() * CRANK_SCROLL_MODIFIER);

  if (angle > TURRET_ANGLE_MAX) { angle = TURRET_ANGLE_MAX; }
  else if (angle < TURRET_ANGLE_MIN) { angle = TURRET_ANGLE_MIN; }
  m_turretBarrelAngle = angle;

  // Apply
  applyScrollEasing();
}

void commonScrollTo(int16_t destination, float progress, enum EasingFunction_t e) {
  const float so = getScrollOffset();
  setScrollOffset(so + (destination - so)*getEasing(e, progress), true);
}

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

enum kFSM doFSM(enum kFSM transitionTo) {
  bool newState = (m_FSM != transitionTo);
  if (newState) { 
    m_FSM = transitionTo;
    pd->system->logToConsole("State change %i", (int)transitionTo);
  }

  if (m_FSM < kFSM_SPLIT_TitlesGame) return doFSM_Titles(newState);
  else return doFSM_Game(newState);
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

enum kFSM doFSM_Titles(bool newState) {

  if (m_FSM == kTitlesFSM_DisplayTitles) {

    if (newState) { 
      populateMenuTitles();
      setGameMode(kTitles);
    }
    if (!pd->system->isCrankDocked()) { return doFSM(kTitlesFSM_TitlesToChoosePlayer); }

  } else if (m_FSM == kTitlesFSM_TitlesToChoosePlayer) {

    static int16_t timer = 0;
    if (newState) { timer = 0; }
    commonScrollTo(DEVICE_PIX_Y, (float)timer/TIME_TITLE_TRANSITION, EaseInOutQuad);
    if (timer++ == TIME_TITLE_TRANSITION) { return doFSM(kTitlesFSM_ChoosePlayer); }

  } else if (m_FSM == kTitlesFSM_ChoosePlayer) {

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

  } else if (m_FSM == kTitlesFSM_ChoosePlayerToChooseLevel) {

    static int16_t timer = 0;
    if (newState) { 
      timer = 0;
      setNumeralOffset(0.0f);
      updateLevelStatsBitmap();
      resetPreviousWaterfall();
    }
    commonScrollTo(DEVICE_PIX_Y * 2, (float)timer/TIME_TITLE_TRANSITION, EaseInOutQuad);
    if (timer++ == TIME_TITLE_TRANSITION) { return doFSM(kTitlesFSM_ChooseLevel); }

  } else if (m_FSM == kTitlesFSM_ChooseLevel) {

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

  } else if (m_FSM == kTitlesFSM_ChooseLevelToChooseHole) {

    static int16_t timer = 0;
    if (newState) { 
      timer = 0;
      setNumeralOffset(0.0f);
      updateHoleStatsBitmap();
    }
    commonScrollTo(DEVICE_PIX_Y * 3, (float)timer/TIME_TITLE_TRANSITION, EaseInOutQuad);
    if (timer++ == TIME_TITLE_TRANSITION) { return doFSM(kTitlesFSM_ChooseHole); }

  } else if (m_FSM == kTitlesFSM_ChooseLevelToChoosePlayer) {

    static int16_t timer = 0;
    if (newState) {
      timer = 0;
      setNumeralOffset(0.0f);
    }
    commonScrollTo(DEVICE_PIX_Y, (float)timer/TIME_TITLE_TRANSITION, EaseInOutQuad);
    if (timer++ == TIME_TITLE_TRANSITION) { return doFSM(kTitlesFSM_ChoosePlayer); }

  } else if (m_FSM == kTitlesFSM_ChooseHole) {

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

  } else if (m_FSM == kTitlesFSM_ChooseHoleToSplash) {
    
    static int16_t timer = 0;
    if (newState) {
      timer = 0;
      setNumeralOffset(0.0f);
      updateInfoTopperBitmap();
      updateLevelSplashBitmap(); 
    }
    commonScrollTo(DEVICE_PIX_Y * 4, (float)timer/TIME_TITLE_TRANSITION, EaseInOutQuad);
    if (timer++ == TIME_TITLE_TRANSITION) { return doFSM(kGameFSM_DisplaySplash); }

  } else if (m_FSM == kTitlesFSM_ChooseHoleToChooseLevel) {

    static int16_t timer = 0;
    if (newState) { 
      timer = 0;
      setNumeralOffset(0.0f);
      resetPreviousWaterfall();
    }
    commonScrollTo(DEVICE_PIX_Y * 2, (float)timer/TIME_TITLE_TRANSITION, EaseInOutQuad);
    if (timer++ == TIME_TITLE_TRANSITION) { return doFSM(kTitlesFSM_ChooseLevel); }

  } else {

      pd->system->error("FSM error titles");

  }

  return m_FSM;
}

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

enum kFSM doFSM_Game(bool newState) {

  if (m_FSM == kGameFSM_DisplaySplash) {

    static uint16_t timer = 0;
    if (newState) { 
      timer = 0;
      setGameMode(kGameWindow);
      setScrollOffset(-DEVICE_PIX_Y - TURRET_RADIUS, true);
      updateScoreHistogramBitmap();
      populateMenuGame();
      // randomiseBoard(); // // TODO replace me
      clearBoard();
      loadCurrentHole();
    }
    if (timer++ == TIME_DISPLAY_SPLASH) return doFSM(kGameFSM_SplashToStart);

  } else if (m_FSM == kGameFSM_SplashToStart) {

    static uint16_t timer = 0;
    if (newState) {
      timer = 0;
      m_ballCount = 0;
      setMinimumY(0);
    }
    float progress = getEasing(EaseInOutQuint, 1.0f - (float)timer/TIME_SPLASH_TO_GAME);
    setScrollOffset((-DEVICE_PIX_Y - TURRET_RADIUS) * progress, true);
    if (timer++ == TIME_SPLASH_TO_GAME) {
      return doFSM(kGameFSM_AimMode);
    }

  } else if (m_FSM == kGameFSM_AimMode) {

    static uint16_t timer = 0;
    if (newState) {
      resetBallTrace();
      timer = 0;
    }
    commonScrollAndBounceBack();
    //
    float progress = getEasing(EaseOutQuart, (float)timer/TIME_FIRE_BALL);
    if ((timer >= TIME_FIRE_BALL) || (progress > 0.5f && !getPressed(kButtonA))) { // Fire
      resetBall();
      launchBall(progress);
      ++m_ballCount;
      return doFSM(kGameFSM_BallInPlay);
    } else if (getPressed(kButtonA)) { // Arm
      timer++;
    } else if (timer > 0) { // Reset
      timer = 0;
    }
    setBallPootCircle((uint16_t)(progress * TURRET_RADIUS));

  } else if (m_FSM == kGameFSM_BallInPlay) {

    if (newState) {
      m_ballStuckCounter = 0;
    }
    //
    commonScrollAndBounceBack();
    //
    const float y = cpBodyGetPosition(getBall()).y;
    setScrollOffset(y - HALF_DEVICE_PIX_Y, false);
    //
    if (y > WFALL_PIX_Y + BALL_RADIUS) {
      return doFSM(kGameFSM_BallGutter);
    } 
    //
    cpVect v = cpBodyGetVelocity(getBall());
    // pd->system->logToConsole("cpvlengthsq %f", cpvlengthsq(v));
    if (cpvlengthsq(v) < BALL_IS_STUCK) {
      if (m_ballStuckCounter++ > STUCK_TICKS) {
        return doFSM(kGameFSM_BallStuck);
      }
    } else {
      m_ballStuckCounter = 0;
    }

  } else if (m_FSM == kGameFSM_BallStuck) {

    commonScrollAndBounceBack();
    //
    bool popped = true;
    if (m_frameCount % TIME_STUCK_POP == 0) {
      popped = popRandom();
    }
    cpVect v = cpBodyGetVelocity(getBall());
    if (cpvlengthsq(v) > BALL_IS_STUCK || !popped) {
      return doFSM(kGameFSM_BallInPlay);
    }

  } else if (m_FSM == kGameFSM_CloseUp) {

    // TODO

  } else if (m_FSM == kGameFSM_WinningToast) {

    // TODO
    
  } else if (m_FSM == kGameFSM_BallGutter) {

    // A little extra downward camera motion. Note: No more manual scroll accepted
    static float vY = 0.0;
    static uint16_t pause = 0;
    if (newState) {
      vY = cpBodyGetVelocity(getBall()).y;
      pause = TICK_FREQUENCY / 2;
    }
    // pd->system->logToConsole("v %f so %f", vY, getScrollOffset());
    if (vY > 0.01f) {
      setScrollOffset(getScrollOffset() + (vY * TIMESTEP), true);
      vY *= 0.5f;
    } else if (pause) {
      --pause;
    } else {
      if (requiredPegsInPlay()) { return doFSM(kGameFSM_GuttetToTurret); }
      else                      { return doFSM(kGameFSM_GutterToScores); }
    }

  } else if (m_FSM == kGameFSM_GuttetToTurret) {

    static float progress = 0.0f;
    static float startY = 0.0f;
    if (newState) {
      progress = 0.0f;
      startY = getScrollOffset();
      //
      int32_t minY = 10000;
      for (uint32_t i = 0; i < MAX_PEGS; ++i) {
        const struct Peg_t* peg = getPeg(i);
        if (peg->m_minY < minY && peg->m_state == kPegStateActive) {
          minY = peg->m_minY;
        }
      }
      if (minY > (DEVICE_PIX_Y/2)) setMinimumY(minY - (DEVICE_PIX_Y/2));
      pd->system->logToConsole("smallest y was %i, min y is now %i", minY, getMinimumY());
      pd->system->logToConsole("kGameFSM_GuttetToTurret");
    }
    const int16_t minimumY = getMinimumY();
    // Take less time overall when we get lower down
    const float distance_mod = startY / (float)(startY - minimumY);
    //
    progress += (TIMESTEP * END_SWEEP_SCALE * distance_mod);
    float easedProgress = getEasing(EaseInOutSine, progress);
    easedProgress = (1.0f - easedProgress);
    // // pd->system->logToConsole("end sweep active target %f, pop %f",target, m_popLevel);
    const float targetPartial = minimumY + ((startY - minimumY) * easedProgress);
    setScrollOffset(targetPartial, true);
    //
    const float popLevel = (startY + DEVICE_PIX_Y) * easedProgress;
    m_popLevel = popLevel; // TODO remove me
    popBoard(popLevel);
    //
    // pd->system->logToConsole("end sweep active target %f, pop %f",target, m_popLevel);
    if (progress >= 1) {
      setScrollOffset(minimumY, true);
      return doFSM(kGameFSM_AimMode);
    }

  } else if (m_FSM == kGameFSM_GutterToScores) {

    static int16_t timer = 0;
    if (newState) {
      timer = 0;
      pd->system->logToConsole("kGameFSM_GuttetToTurret");
    }
    commonScrollTo(WFALL_PIX_Y + DEVICE_PIX_Y, (float)timer/TIME_GUTTER_TO_SCORE, EaseOutSine);
    if (timer++ == TIME_GUTTER_TO_SCORE) {
      setBallFallN(0);
      return doFSM(kGameFSM_ScoresAnimation);
    }


  } else if (m_FSM == kGameFSM_ScoresAnimation) {

    static float py[32] = {0};
    static float vy[32] = {0};
    static uint16_t timer = 0;
    static uint16_t ballsToShow = 0;
    static uint16_t activeBalls = 1; 
    static const uint16_t maxBallsToShow = 12;
    if (newState) {
      timer = 0;
      ballsToShow = m_ballCount;
      if (ballsToShow > maxBallsToShow) { ballsToShow = maxBallsToShow; }
      setBallFallN(ballsToShow);
      setBallFallX(getCurrentHole());
      activeBalls = 1;
      for (int i = 0; i < ballsToShow; ++i) {
        py[i] = -2*BALL_RADIUS*(i + 1);
        setBallFallY(i, py[i]);
      }
      pd->system->logToConsole("kGameFSM_ScoresAnimation, ballsToShow %i", ballsToShow);
      setHoleScore(m_ballCount);
    }
    //
    for (int i = 0; i < activeBalls; ++i) {
      const float floorY = BUF+(maxBallsToShow -1 - i)*2*BALL_RADIUS;
      if (py[i] >= floorY) {
        py[i] = floorY;
      } else {
        vy[i] += HISTO_BALL_ACCELERATION;
        py[i] += vy[i];
      }
      setBallFallY(i, py[i]);
      // pd->system->logToConsole("kGameFSM_ScoresAnimation, i %i y %f",i, py[i]);
    }
    if (timer++ % TIME_BALL_DROP_DELAY == 0 && activeBalls < ballsToShow) { activeBalls++; }

    if (getPressed(kButtonUp))   return doFSM(kGameFSM_ScoresToTitle);
    if (getPressed(kButtonDown)) return doFSM(kGameFSM_ScoresToSplash);

  } else if (m_FSM == kGameFSM_ScoresToTitle) {

    static int16_t timer = 0;
    if (newState) {
      timer = 0;
      pd->system->logToConsole("kGameFSM_ScoresToSplash");
    }
    commonScrollTo(WFALL_PIX_Y, (float)timer/TIME_SCORE_TO_TITLE, EaseInSine);
    if (timer++ == TIME_SCORE_TO_TITLE) { return doFSM(kTitlesFSM_DisplayTitles); }

  } else if (m_FSM == kGameFSM_ScoresToSplash) {

    static int16_t timer = 0;
    if (newState) {
      timer = 0;
      doNextHole();
      if (getCurrentHole() == 0) {
        doNextLevel();
      }
      updateLevelSplashBitmap();
      updateInfoTopperBitmap();
      pd->system->logToConsole("kGameFSM_ScoresToSplash");
    }
    commonScrollTo(WFALL_PIX_Y + 2*DEVICE_PIX_Y, (float)timer/TIME_SCORE_TO_SPLASH, EaseOutSine);
    if (timer++ == TIME_SCORE_TO_SPLASH) { return doFSM(kGameFSM_DisplaySplash); }

  } else {

      pd->system->error("FSM error game");

  }

  return m_FSM;
}

void initGame() {
}

char* ftos(float _value, int16_t _size, char* _dest) {
  char* tmpSign = (_value < 0) ? "-" : "";
  float tmpVal = (_value < 0) ? -_value : _value;

  int16_t tmpInt1 = tmpVal;
  float tmpFrac = tmpVal - tmpInt1;
  int16_t tmpInt2 = trunc(tmpFrac * 10000);

  char truncStr[8];
  snprintf (_dest, _size, "%02d", tmpInt2);
  snprintf (truncStr, 8, "%.2s", _dest);

  snprintf (_dest, _size, "%s%d.%s", tmpSign, tmpInt1, truncStr);
  return _dest;
}