#include "game.h"
#include "fsm.h"
#include "easing.h"
#include "render.h"
#include "physics.h"
#include "board.h"
#include "peg.h"
#include "bitmap.h"
#include "io.h"
#include "input.h"

uint16_t m_ballStuckCounter[2] = {0}; // Keeping track of a stuck ball
uint16_t m_ballCount = 0; // Keeping track of the current level score

void commonTurretScrollAndBounceBack(const bool allowScroll);

///

void FSMDoResetBallStuckCounter(void) { 
  m_ballStuckCounter[0] = 0;
  m_ballStuckCounter[1] = 0;
}

void commonTurretScrollAndBounceBack(const bool allowScroll) {
  // Button based
  float diffY = 0;
  if      (getPressed(kButtonUp)) diffY = -SCREEN_ACC;
  else if (getPressed(kButtonDown)) diffY =  SCREEN_ACC;
  modScrollVelocity(diffY);
  // Crank based
  static float angle = 179.0f;
  static bool topLock = true;
  static float revDetection = 180.0f;
  // Backup: non-crank
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

  if (allowScroll) {
    if (!topLock) modScrollVelocity(getCrankChanged() * CRANK_SCROLL_MODIFIER);
    applyScrollEasing();
  }

  if (angle > TURRET_ANGLE_MAX) { angle = TURRET_ANGLE_MAX; }
  else if (angle < TURRET_ANGLE_MIN) { angle = TURRET_ANGLE_MIN; }
  setTurretBarrelAngle(angle);
}


void FSMDisplaySplash(const bool newState) {
  static uint16_t timer = 0;
  if (newState) { 
    timer = 0;
    setScrollOffset(-DEVICE_PIX_Y - TURRET_RADIUS, true);
    updateScoreHistogramBitmap();
    populateMenuGame();
    boardRandomise(); // // TODO replace me
    //boardClear();
    //loadCurrentHole();
  }
  if (timer++ == TIME_DISPLAY_SPLASH) return FSMDo(kGameFSM_SplashToStart);
}

void FSMSplashToStart(const bool newState) {
  static uint16_t timer = 0;
  if (newState) {
    timer = 0;
    m_ballCount = 0;
    setMinimumY(0);
  }
  float progress = getEasing(kEaseInOutQuint, 1.0f - (float)timer/TIME_SPLASH_TO_GAME);
  setScrollOffset((-DEVICE_PIX_Y - TURRET_RADIUS) * progress, true);
  if (timer++ == TIME_SPLASH_TO_GAME) { return FSMDo(kGameFSM_AimMode); }
}

void FSMAimModeScrollToTop(const bool newState) {
  static uint16_t timer = 0;
  if (newState) { timer = 0; }
  const float progress = getEasing(kEaseInOutQuint, 1.0f - (float)timer/TIME_AIM_SCROLL_TO_TOP);
  setScrollOffset(getMinimumY() + ((getScrollOffset() - getMinimumY())*progress), true);
  commonTurretScrollAndBounceBack(false); // Just move turret, don't influence the scroll
  if (timer++ == TIME_AIM_SCROLL_TO_TOP) { return FSMDo(kGameFSM_AimMode); }

}

void FSMAimMode(const bool newState) {
  static uint16_t timer = 0;
  if (newState) {
    resetBallTrace();
    resetFrameCount();
    timer = 0;
  }
  commonTurretScrollAndBounceBack(true);
  //
  float progress = getEasing(kEaseOutQuart, (float)timer/TIME_FIRE_BALL);
  if ((timer >= TIME_FIRE_BALL) || (progress > 0.5f && !getPressed(kButtonA))) { // Fire
    resetBall(0);
    launchBall(progress);
    ++m_ballCount;
    if (boardGetCurrentSpecial() == kPegSpecialMultiball) { addSecondBall(); }
    return FSMDo(kGameFSM_BallInPlay);
  } else if (getPressed(kButtonA)) { // Arm
    timer++;
  } else if (timer > 0) { // Reset
    timer = 0;
  }
  setBallPootCircle((uint16_t)(progress * TURRET_RADIUS));
}

void FSMBallInPlay(const bool newState) {
  const enum PegSpecial_t special = boardGetCurrentSpecial();
  if (newState) { FSMDoResetBallStuckCounter(); }
  //
  commonTurretScrollAndBounceBack(true);
  //
  const float y1 = cpBodyGetPosition(getBall(0)).y;
  const float y2 = (special == kPegSpecialMultiball ? cpBodyGetPosition(getBall(1)).y : 0);
  float y = MAX(y1, y2);
  if (special == kPegSpecialMultiball && getSecondBallInPlay() && y >= WFALL_PIX_Y + BALL_RADIUS) { y = MIN(y1,y2); }
  setScrollOffset(y - HALF_DEVICE_PIX_Y, false);
  //
  if (y > WFALL_PIX_Y + BALL_RADIUS) {
    return FSMDo(kGameFSM_BallGutter);
  } 
  //
  for (int i = 0; i < 2; ++i) {
    if (i == 1 && special != kPegSpecialMultiball) { continue; }
    const cpVect v = cpBodyGetVelocity(getBall(i));
    if (cpvlengthsq(v) < BALL_IS_STUCK) {
      if (m_ballStuckCounter[i]++ > STUCK_TICKS) {
        return FSMDo(kGameFSM_BallStuck);
      }
    } else {
      m_ballStuckCounter[i] = 0;
    }
  }
}

void FSMBallStuck(const bool newState) {
  commonTurretScrollAndBounceBack(true);
  //
  bool popped = true;
  if (getFrameCount() % TIME_STUCK_POP == 0) {
    popped = boardBurstRandomPeg();
  }
  cpVect v = cpBodyGetVelocity(getBall(0));
  bool ballIsStuck = cpvlengthsq(v) < BALL_IS_STUCK;
  if (boardGetCurrentSpecial() == kPegSpecialMultiball) {
    v = cpBodyGetVelocity(getBall(1));
    ballIsStuck |= cpvlengthsq(v) < BALL_IS_STUCK;
  }
  if (!ballIsStuck || !popped) {
    return FSMDo(kGameFSM_BallInPlay);
  }
}

void FSMCloseUp(const bool newState) {
  // TODO
}

void FSMWinningToast(const bool newState) {
  // TODO
}

void FSMBallGutter(const bool newState) {
  // A little extra downward camera motion. Note: No more manual scroll accepted
  static float vY = 0.0;
  static uint16_t pause = 0;
  const enum PegSpecial_t special = boardGetCurrentSpecial();
  if (newState) {
    if (special == kPegSpecialMultiball) {
      removeSecondBall();
    } else if (special == kPegSpecialSecondTry) {
      secondTryBall();
      boardClearSpecial();
      return FSMDo(kGameFSM_BallInPlay);
    }
    boardClearSpecial();
    vY = cpBodyGetVelocity(getBall(0)).y;
    pause = TICK_FREQUENCY / 2;
  }
  // pd->system->logToConsole("v %f so %f", vY, getScrollOffset());
  if (vY > 0.01f) {
    setScrollOffset(getScrollOffset() + (vY * TIMESTEP), true);
    vY *= 0.5f;
  } else if (pause) {
    --pause;
  } else {
    if (boardGetRequiredPegsInPlay()) { return FSMDo(kGameFSM_GutterToTurret); }
    else                      { return FSMDo(kGameFSM_GutterToScores); }
  }
}

void FSMGutterToTurret(const bool newState) {
  static float progress = 0.0f;
  static float startY = 0.0f;
  if (newState) {
    progress = 0.0f;
    startY = getScrollOffset();
    //
    int32_t minY = 10000;
    for (uint32_t i = 0; i < MAX_PEGS; ++i) {
      const struct Peg_t* p = boardGetPeg(i);
      if (p->minY < minY && p->state == kPegStateActive) {
        minY = p->minY;
      }
    }
    if (minY > (DEVICE_PIX_Y/2)) setMinimumY(minY - (DEVICE_PIX_Y/2));
    pd->system->logToConsole("kGameFSM_GutterToTurret smallest y was %i, min y is now %i", minY, getMinimumY());
  }
  const int16_t minimumY = getMinimumY();
  // Take less time overall when we get lower down
  const float distance_mod = startY / (float)(startY - minimumY);
  //
  commonTurretScrollAndBounceBack(false); // Just move turret, don't influence the scroll
  //
  progress += (TIMESTEP * END_SWEEP_SCALE * distance_mod);
  float easedProgress = getEasing(kEaseInOutSine, progress);
  easedProgress = (1.0f - easedProgress);
  const float targetPartial = minimumY + ((startY - minimumY) * easedProgress);
  setScrollOffset(targetPartial, true);
  //
  const float popLevel = (startY + DEVICE_PIX_Y) * easedProgress;
  boardBurstPegs(popLevel);
  //
  // pd->system->logToConsole("end sweep active target %f, pop %f",target, popLevel);
  if (progress >= 1) {
    setScrollOffset(minimumY, true);
    return FSMDo(kGameFSM_AimMode);
  }
}

void FSMGutterToScores(const bool newState) {
  static int16_t timer = 0;
  if (newState) { timer = 0; }
  FSMCommonScrollTo(WFALL_PIX_Y + DEVICE_PIX_Y, (float)timer/TIME_GUTTER_TO_SCORE, kEaseOutSine);
  if (timer++ == TIME_GUTTER_TO_SCORE) {
    setBallFallN(0);
    return FSMDo(kGameFSM_ScoresAnimation);
  }
}

void FSMScoresAnimation(const bool newState) {
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

  if (getPressed(kButtonUp))   return FSMDo(kGameFSM_ScoresToTitle); // TODO apply some friction here, else move to Inputs
  if (getPressed(kButtonDown)) return FSMDo(kGameFSM_ScoresToSplash);
}

void FSMScoresToTitle(const bool newState) {
  static int16_t timer = 0;
  if (newState) { timer = 0; }
  FSMCommonScrollTo(WFALL_PIX_Y, (float)timer/TIME_SCORE_TO_TITLE, kEaseInSine);
  if (timer++ == TIME_SCORE_TO_TITLE) { return FSMDo(kTitlesFSM_DisplayTitles); }
}

void FSMScoresToSplash(const bool newState) {
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
  FSMCommonScrollTo(WFALL_PIX_Y + 2*DEVICE_PIX_Y, (float)timer/TIME_SCORE_TO_SPLASH, kEaseOutSine);
  if (timer++ == TIME_SCORE_TO_SPLASH) { return FSMDo(kGameFSM_DisplaySplash); }
}

