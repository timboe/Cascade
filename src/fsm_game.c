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
#include "sound.h"
#include "patterns.h"

uint16_t m_ballStuckCounter[2] = {0}; // Keeping track of a stuck ball
uint16_t m_ballCount = 0; // Keeping track of the current level score

cpVect m_finalRequiredPos; // Position of the final required ped on the board

bool m_vetoCloseUp = false; // If we break out of close up, then don't do it again on this ball

void FSMCommonTurretScrollAndBounceBack(const bool allowScroll);

bool FSMCommonFocusOnLowestBallInPlay(const enum PegSpecial_t special);

/// ///

void FSMDoResetBallStuckCounter(void) { 
  m_ballStuckCounter[0] = 0;
  m_ballStuckCounter[1] = 0;
}

bool FSMCommonGetIsBallStuck(const enum PegSpecial_t special) {
  for (int i = 0; i < MAX_BALLS; ++i) {
    if (i == 1 && special != kPegSpecialMultiball) { break; }
    const cpVect v = cpBodyGetVelocity(physicsGetBall(i));
    if (cpvlengthsq(v) < BALL_IS_STUCK) {
      if (m_ballStuckCounter[i]++ > STUCK_TICKS) { return true; }
    } else {
      m_ballStuckCounter[i] = 0;
    }
  }
  return false;
}

bool FSMCommonFocusOnLowestBallInPlay(const enum PegSpecial_t special) {
  // Allow scrolling
  FSMCommonTurretScrollAndBounceBack(true);
  // But with strong focus on ball
  cpVect ballPos[2];
  ballPos[0] = cpBodyGetPosition(physicsGetBall(0));
  ballPos[1] = cpBodyGetPosition(physicsGetBall(1));
  const float y1 = ballPos[0].y;
  const float y2 = (special == kPegSpecialMultiball ? ballPos[1].y : 0);
  float y = MAX(y1, y2);
  if (special == kPegSpecialMultiball && physicsGetSecondBallInPlay() && y >= IOGetCurrentHoleHeight() + BALL_RADIUS) { 
    uint8_t whichBall = 0;
    if (y == ballPos[1].y) { whichBall = 1; }
    renderDoTriggerSplash(whichBall, ballPos[whichBall].x);
    //    
    y = MIN(y1,y2);
  }
  gameSetYOffset(y - HALF_DEVICE_PIX_Y, false);
  // Return true if no balls in play
  const bool gutterd = (y > IOGetCurrentHoleHeight() + BALL_RADIUS);
  if (gutterd) {
    uint8_t whichBall = 0;
    if (y == ballPos[1].y) { whichBall = 1; }
    if (special != kPegSpecialSecondTry) renderDoTriggerSplash(whichBall, ballPos[whichBall].x);
  }
  return gutterd;
}

void FSMCommonTurretScrollAndBounceBack(const bool allowScroll) {
  // Button based
  float diffY = 0;
  if      (inputGetPressed(kButtonUp)) diffY = -SCREEN_ACC;
  else if (inputGetPressed(kButtonDown)) diffY =  SCREEN_ACC;
  gameModYVelocity(diffY);
  // Crank based
  static float angle = 180.0f;
  static bool topLock = true;
  static float revDetection = 180.0f;
  // Backup: non-crank
  if (pd->system->isCrankDocked()) {
    if      (inputGetPressed(kButtonLeft)) angle += 1.0f;
    else if (inputGetPressed(kButtonRight)) angle -= 1.0f;
  } else {
    angle = inputGetCrankAngle();
  }

  const bool newRev = fabsf(angle - revDetection) > 180.0f;
  revDetection = angle;

  const float so = gameGetYOffset();
  if ((newRev && angle < 180.0f) || so > gameGetMinimumY()) {
    topLock = false;
  } else if (so <= gameGetMinimumY()) {
    topLock = true;
  }

  if (allowScroll) {
    if (!topLock) gameModYVelocity(inputGetCrankChanged() * CRANK_SCROLL_MODIFIER);
    gameDoApplyYEasing();
  }

  if (angle > TURRET_ANGLE_MAX) { angle = TURRET_ANGLE_MAX; }
  else if (angle < TURRET_ANGLE_MIN) { angle = TURRET_ANGLE_MIN; }
  gameSetTurretBarrelAngle(angle);
}

bool FSMCommonMarbleFire(uint16_t* timer) {
  float progress = getEasing(EASE_MARBLE_FIRE, (float)(*timer)/TIME_FIRE_MARBLE);
  static bool once = false;
  if ((*timer) >= TIME_FIRE_MARBLE || (progress > 0.5f && !inputGetPressed(kButtonB))) { // Fire
    physicsDoResetBall(0);
    physicsDoLaunchBall(progress);
    ++m_ballCount;
    if (boardGetCurrentSpecial() == kPegSpecialMultiball) { physicsDoAddSecondBall(); }
    once = false;
    soundStopSfx(kChargeSfx);
    soundDoSfx(kPootSfx);
    return true;
  } else if (inputGetPressed(kButtonB)) { // Arm
    (*timer)++;
    if (!once) {
      once = true;
      soundDoSfx(kChargeSfx);
    }
  } else if (*timer > 0) { // Reset
    (*timer) = 0;
    once = false;
    soundStopSfx(kChargeSfx);
  }
  renderSetMarblePootCircle((uint16_t)(progress * TURRET_RADIUS));
  return false;
}

////////////////////////////////        ////////////////////////////////

void FSMDisplayLevelTitle(const bool newState) {
  static uint16_t timer = 0;
  static uint16_t timeToWait = 0;
  if (newState) {
    const uint32_t before = pd->system->getCurrentTimeMilliseconds(); 
    timer = 0;
    timeToWait = IOIsCredits() ? TIME_DISPLAY_SPLASH*5 : TIME_DISPLAY_SPLASH;
    gameSetYOffset(-DEVICE_PIX_Y - TURRET_RADIUS, true);
    bitmapDoUpdateScoreHistogram();
    if (!IOIsCredits()) { gameDoPopulateMenuGame(); }
    boardDoClear();
    IODoLoadCurrentHole();
    physicsSetTimestepMultiplier(1.0f);
    gameDoResetPreviousWaterfall();
    renderDoUpdateBacklines();
    physicsDoResetBall(0);
    const uint32_t after = pd->system->getCurrentTimeMilliseconds();
    pd->system->logToConsole("Hole loading took %i ms", (int)(after - before));
  }
  if (timer++ == timeToWait) return FSMDo(kGameFSM_LevelTitleToStart);
}

void FSMLevelTitleToStart(const bool newState) {
  static uint16_t timer = 0;
  if (newState) {
    timer = 0;
    m_ballCount = 0;
    gameSetMinimumY(0);
    soundDoSfx(kWhooshSfx1);
  }
  float progress = getEasing(EASE_SPLASH_TO_GAME, 1.0f - (float)timer/TIME_SPLASH_TO_GAME);
  gameSetYOffset((-DEVICE_PIX_Y - TURRET_RADIUS) * progress, true);
  if (timer++ == TIME_SPLASH_TO_GAME) { 
    if (IOGetIsTutorial()) { return FSMDo(kGameFSM_TutorialScrollDown); }
    else if (IOIsCredits()) { return FSMDo(kGameFSM_PlayCredits); }
    else { return FSMDo(kGameFSM_AimMode); }
  }
}

void FSMTutorialScrollDown(const bool newState) {
  static uint16_t timer = 0;
  if (newState) { timer = 0; }

  FSMCommonTurretScrollAndBounceBack(true); // allow control = true
  const float scrollOffsetMax = IOGetCurrentHoleHeight() - DEVICE_PIX_Y; 
  if (gameGetYOffset() > scrollOffsetMax) { ++timer; }
  if (timer > TICK_FREQUENCY/2) { FSMDo(kGameFSM_TutorialScrollUp); }
}

void FSMTutorialScrollUp(const bool newState) {
  static uint16_t timer = 0;
  if (newState) { timer = 0; }

  FSMCommonTurretScrollAndBounceBack(true); // allow control = true
  if (gameGetYOffset() < 4) { ++timer; }
  if (timer > TICK_FREQUENCY/2) { FSMDo(kGameFSM_TutorialFireMarble); }
}

void FSMTutorialFireMarble(const bool newState) {
  static uint16_t timer = 0;
  if (newState) {
    gameSetMinimumY(0); // Tutorial only
    gameSetYOffset(0, true);  // Tutorial only
    renderDoResetMarbleTrace();
    gameDoResetFrameCount();
    renderDoResetTriggerSplash();
    timer = 0;
  }
  //
  FSMCommonTurretScrollAndBounceBack(false); // allow control = FALSE is tutorial only
  //
  const bool fired = FSMCommonMarbleFire(&timer); 
  if (fired) { FSMDo(kGameFSM_BallInPlay); }
}

void FSMTutorialGetSpecial(const bool newState) {
  return FSMTutorialFireMarble(newState); // Functionally identical
}

void FSMTutorialGetRequired(const bool newState) {
  return FSMTutorialFireMarble(newState); // Functionally identical
}

void FSMAimMode(const bool newState) {
  static uint16_t timer = 0;
  if (newState) {
    renderDoResetMarbleTrace();
    gameDoResetFrameCount();
    renderDoResetTriggerSplash();
    timer = 0;
    m_vetoCloseUp = false;
  }
  //
  FSMCommonTurretScrollAndBounceBack(true);
  //
  const bool fired = FSMCommonMarbleFire(&timer); 
  if (fired) { FSMDo(kGameFSM_BallInPlay); }
}

void FSMAimModeScrollToTop(const bool newState) {
  static uint16_t timer = 0;
  if (newState) { timer = 0; }
  const float progress = getEasing(EASE_AIM_SCROLL_TO_TOP, 1.0f - (float)timer/TIME_AIM_SCROLL_TO_TOP);
  gameSetYOffset(gameGetMinimumY() + ((gameGetYOffset() - gameGetMinimumY())*progress), true);
  FSMCommonTurretScrollAndBounceBack(false); // Just move turret, don't influence the scroll
  if (timer++ == TIME_AIM_SCROLL_TO_TOP) { 
    if (IOGetIsTutorial() && m_ballCount == 0) { return FSMDo(kGameFSM_TutorialFireMarble); }
    else return FSMDo(kGameFSM_AimMode);
  }
}

void FSMPlayCredits(const bool newState) {
  static float progress = 0.0f;
  static float pause = 0.0f;
  if (newState) {
    progress = gameGetYOffset();
    pause = 0.0f;
    renderDoResetMarbleTrace();
    gameDoResetFrameCount();
    renderDoResetTriggerSplash();
    physicsDoResetBall(0);
    gameSetTurretBarrelAngle(90.0f + 45.0f);
    physicsDoLaunchBall(1.0f);
  }

  if (progress >= gameGetMinimumY() + IOGetCurrentHoleHeight() - DEVICE_PIX_Y/3) {
    pause += 1.0f / TICK_FREQUENCY;
    physicsDoResetBall(0);
    if (pause >= 5.0f) {
      renderSetFadeLevel( ((int8_t)pause - 5.0f) < FADE_LEVELS-1 ? ((int8_t)pause - 5.0f) : FADE_LEVELS-1 );
      if (pause >= 5.0f + (float)FADE_LEVELS) {
        return FSMDo(kTitlesFSM_DisplayTitlesWFadeIn);
      }
    }
    return;
  }

  cpVect ballPos = cpBodyGetPosition(physicsGetBall(0));
  const bool gutterd = (ballPos.y > IOGetCurrentHoleHeight() + BALL_RADIUS);
  if (gutterd) { physicsDoSecondTryBall(); }

  progress += 0.25f;
  progress += inputGetCrankChanged() * CRANK_SCROLL_MODIFIER;
  if (progress < gameGetMinimumY()) { progress = gameGetMinimumY(); }
  gameSetYOffset(progress, false);
}

void FSMBallInPlay(const bool newState) {
  const enum PegSpecial_t special = boardGetCurrentSpecial();
  if (newState) {
    FSMDoResetBallStuckCounter();
    renderSetMarblePootCircle(0);
    m_finalRequiredPos.x = 0;
    m_finalRequiredPos.y = 0;
  }
  // Focus on ball, check gutter
  const bool guttered = FSMCommonFocusOnLowestBallInPlay(special);
  if (guttered) { return FSMDo(kGameFSM_BallGutter); }
  // Ball stuck -> to stuck
  const bool stuck = FSMCommonGetIsBallStuck(special);
  if (stuck) { FSMDo(kGameFSM_BallStuck); }
  // Ball close -> to close up
  const int16_t bgrpip = boardGetRequiredPegsInPlay();
  if (bgrpip == 1) {
    // Locate and cache final peg
    if (!m_finalRequiredPos.y) {
      for (int i = 0; i < boardGetNPegs(); ++i) {
        const struct Peg_t* p = boardGetPeg(i);
        if (p->type == kPegTypeRequired && p->state == kPegStateActive) {
          m_finalRequiredPos.x = p->x;
          m_finalRequiredPos.y = p->y;
          break;
        }
      }
    }
    // Check for distance to final peg
    if (!m_vetoCloseUp) {
      for (int i = 0; i < MAX_BALLS; ++i) {
        if (i == 1 && special != kPegSpecialMultiball) { break; }
        cpVect ballPos = cpBodyGetPosition(physicsGetBall(i));
        if (cpvdist(m_finalRequiredPos, ballPos) < FINAL_PEG_SLOWMO_RADIUS)  { return FSMDo(kGameFSM_CloseUp); }
      }
    }
  } else if (bgrpip == 0) { // Or straight to toast if zero left
    return FSMDo(kGameFSM_WinningToast);
  }
}

void FSMBallStuck(const bool newState) {
  FSMCommonTurretScrollAndBounceBack(true);
  //
  bool popped = true;
  if (gameGetFrameCount() % TIME_STUCK_POP == 0) {
    popped = boardDoBurstRandomPeg();
  }
  cpVect v = cpBodyGetVelocity(physicsGetBall(0));
  bool ballIsStuck = cpvlengthsq(v) < BALL_IS_STUCK;
  if (boardGetCurrentSpecial() == kPegSpecialMultiball) {
    v = cpBodyGetVelocity(physicsGetBall(1));
    ballIsStuck |= cpvlengthsq(v) < BALL_IS_STUCK;
  }
  if (!ballIsStuck || !popped) {
    return FSMDo(kGameFSM_BallInPlay);
  }
}

void FSMCloseUp(const bool newState) {
  const enum PegSpecial_t special = boardGetCurrentSpecial();
  static float timer = 0.0f;
  if (newState) {
    renderSetScale(2);
    physicsSetTimestepMultiplier(0.2f);
    soundDoSfx(kDrumRollSfx1);
    timer = 0.0f;
  }

  cpVect ballPos[2];
  ballPos[0] = cpBodyGetPosition(physicsGetBall(0));
  ballPos[1] = cpBodyGetPosition(physicsGetBall(1));
  timer += 1.0f / TICK_FREQUENCY;

  int8_t whichBall = -1;
  for (int i = 0; i < MAX_BALLS; ++i) {
    if (i == 1 && special != kPegSpecialMultiball) { break; }
    if (cpvdist(m_finalRequiredPos, ballPos[i]) < FINAL_PEG_SLOWMO_RADIUS) {
      whichBall = i;
      break;
    }
  }

  if (timer >= 10.0f) {
    m_vetoCloseUp = true;
  }

  const bool stuck = FSMCommonGetIsBallStuck(special) || m_vetoCloseUp;

  if (stuck || whichBall == -1 || !boardGetRequiredPegsInPlay()) {
    renderSetScale(1);
    gameSetXOffset(0);
    soundStopSfx(kDrumRollSfx1);
    if (stuck) {
      physicsSetTimestepMultiplier(1.0f);
      FSMDo(kGameFSM_BallStuck);
    } else if (whichBall == -1) {
      physicsSetTimestepMultiplier(1.0f);
      return FSMDo(kGameFSM_BallInPlay);
    } else {
      return FSMDo(kGameFSM_WinningToast);
    }
  }

  gameSetXOffset( roundf(ballPos[whichBall].x) - (HALF_DEVICE_PIX_X/2) );
  gameSetYOffset( roundf(ballPos[whichBall].y) - (HALF_DEVICE_PIX_Y/2), true );
}

void FSMWinningToast(const bool newState) {
  const enum PegSpecial_t special = boardGetCurrentSpecial();
  if (newState) {
    renderDoResetEndBlast();
    renderDoAddEndBlast(physicsGetBall(0));
    soundDoMusic();
    soundDoSfx(kDrumRollSfx2);
  }

  const float tsm = physicsGetTimestepMultiplier();
  if (tsm < 0.75f) { physicsSetTimestepMultiplier(tsm + 0.005f);  }

  const cpVect last = renderGetLastEndBlast();
  const cpVect ballPos = cpBodyGetPosition(physicsGetBall(0));
  if (cpvlengthsq( cpvsub(last, ballPos) ) > (16*16)) {
    renderDoAddEndBlast(physicsGetBall(0));
  }

  int fc = gameGetFrameCount();
  if (physicsGetSecondBallInPlay() && fc % (TICK_FREQUENCY/5) == 0) { renderDoAddEndBlast(physicsGetBall(1)); }

  const bool guttered = FSMCommonFocusOnLowestBallInPlay(special);
  if (guttered) { 
    physicsSetTimestepMultiplier(1.0f);
    return FSMDo(kGameFSM_BallGutter);
  }

}

void FSMBallGutter(const bool newState) {
  // A little extra downward camera motion. Note: No more manual scroll accepted
  static float vY = 0.0;
  static uint16_t pause = 0;
  const enum PegSpecial_t special = boardGetCurrentSpecial();
  if (newState) {
    if (special == kPegSpecialMultiball) {
      physicsDoRemoveSecondBall();
    } else if (special == kPegSpecialSecondTry && boardGetRequiredPegsInPlay()) {
      physicsDoSecondTryBall();
      boardDoClearSpecial();
      gameSetYOffset(gameGetMinimumY(), true);
      soundDoSfx(kTeleportSfx);
      return FSMDo(kGameFSM_BallInPlay);
    }
    boardDoClearSpecial();
    vY = cpBodyGetVelocity(physicsGetBall(0)).y;
    pause = TICK_FREQUENCY / 2;
  }
  // pd->system->logToConsole("v %f so %f", vY, gameGetYOffset());
  if (vY > 0.01f) {
    gameSetYOffset(gameGetYOffset() + (vY * TIMESTEP), true);
    vY *= 0.5f;
  } else if (pause) {
    --pause;
  } else {
    if (boardGetRequiredPegsInPlay()) { return FSMDo(kGameFSM_GutterToTurret); }
    else                              { return FSMDo(kGameFSM_GutterToScores); }
  }
}

void FSMGutterToTurret(const bool newState) {
  static float progress = 0.0f;
  static float startY = 0.0f;
  static float endY = 0.0f;
  static float distance_mod = 0.0f;
  static float height_mod = 0.0f;
  if (newState) {
    progress = 0.0f;
    startY = gameGetYOffset();
    endY = gameGetMinimumY();
    // Take less time overall when we get lower down
    distance_mod = startY / (float)(startY - endY);
    // Take less time on smaller levels
    height_mod = (float)(DEVICE_PIX_Y * WF_MAX_HEIGHT) / IOGetCurrentHoleHeight();
    //
    boardDoAddSpecial(true); // Activate
    physicsDoResetBall(0);
    soundDoSfx(kWhooshSfx1);
  }
  //
  FSMCommonTurretScrollAndBounceBack(false); // Just move turret, don't influence the scroll
  //
  progress += (TIMESTEP * END_SWEEP_SCALE * distance_mod * height_mod);
  float easedProgress = getEasing(EASE_GUTTER_TO_TOP, progress);
  easedProgress = (1.0f - easedProgress);
  const float tarIOGetPartial = endY + ((startY - endY) * easedProgress);
  gameSetYOffset(tarIOGetPartial, true);
  //
  // const float popLevel = (startY + DEVICE_PIX_Y) * easedProgress;
  const float popLevel = tarIOGetPartial + (DEVICE_PIX_Y * easedProgress);
  boardDoBurstPegs(popLevel);
  //
  // pd->system->logToConsole("end sweep active target %f, pop %f",target, popLevel);
  if (progress >= 1) {
    gameSetYOffset(endY, true);
    boardDoBurstPegs(0);
    FSMDo(kGameFSM_TurretLower);
  }
}

void FSMTurretLower(const bool newState) {
  static float progress = 0.0f;
  static float startY = 0.0f;
  static float endY = 0.0f;
  static float speed = 0.0f;
  if (newState) {
    progress = 0.0f;
    startY = gameGetYOffset();
    endY = gameGetMinimumY();
    //
    int32_t minY = 10000;
    // pd->system->logToConsole("req pegs hit = %i", boardGetRequiredPegsHit());
    // pd->system->logToConsole("req pegs total = %i", boardGetRequiredPegsTotal());
    // pd->system->logToConsole("req pegs in play = %i", boardGetRequiredPegsInPlay());
    if (boardGetRequiredPegsHit() == 0) {
      // No required pegs hit yet, don't move down yet
      minY = 0;
    } else {
      // At least one required peg hit, move down
      for (uint32_t i = 0; i < MAX_PEGS; ++i) {
        const struct Peg_t* p = boardGetPeg(i);
        //pd->system->logToConsole("peg %i state=%i (1==active), type=%i (1==req), shape=%i (rect,ball,tri), y=%f minY=%f", i, p->state, p->type, p->shape, p->y, p->minY);
        if (p->state == kPegStateActive && p->type == kPegTypeRequired && p->minY < minY) {
          minY = p->minY;
        }
      }
    }
    // Start from the top if the top peg is within the top 1/2 of the play area
    if (minY > (DEVICE_PIX_Y/2) && !IOGetIsTutorial()) { 
      endY = minY - (DEVICE_PIX_Y/2);
    }
    // Don't go below one screen height from the gutter
    const uint16_t maxY = IOGetCurrentHoleHeight() - DEVICE_PIX_Y;
    if (endY > maxY) {
      endY = maxY;
    }
    if (startY == endY) {
      progress = 1.0f;
    } else {
      speed = 100.0f / (endY - startY);
      if (speed < 1.0f) speed = 1.0f;
      soundDoSfx(kRelocateTurretSfx);
    }
    // pd->system->logToConsole("speed is %f", speed);
  }
  //
  FSMCommonTurretScrollAndBounceBack(false); // Just move turret, don't influence the scroll
  //
  progress += (TIMESTEP * speed);
  const float easedProgress = getEasing(EASE_GUTTER_TO_TOP, progress);
  const float progressY = startY + ((endY - startY) * easedProgress);
  gameSetYOffset(progressY, true);
  gameSetMinimumY(progressY);
  //
  // pd->system->logToConsole("end sweep active target %f, pop %f",target, popLevel);
  if (progress >= 1) {
    gameSetYOffset(endY, true);
    gameSetMinimumY(endY);
    if (IOGetIsTutorial() && boardGetSpecialPegsInPlay()) { return FSMDo(kGameFSM_TutorialGetSpecial); }
    else if (IOGetIsTutorial()) { return FSMDo(kGameFSM_TutorialGetRequired); }
    else { return FSMDo(kGameFSM_AimMode); }
  }
}

void FSMGutterToScores(const bool newState) {
  static int16_t timer = 0;
  static int16_t origin = 0;
  if (newState) { 
    timer = 0;
    origin = gameGetYOffset();
    renderSetMarbleFallN(0);
    soundDoSfx(kWhooshSfx1);
  }
  FSMDoCommonScrollTo(origin, DEVICE_PIX_Y*5, (float)timer/TIME_GUTTER_TO_SCORE, EASE_GUTTER_TO_SCORE);
  if (timer++ == TIME_GUTTER_TO_SCORE) { return FSMDo(kGameFSM_ScoresAnimation); }
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
    renderSetMarbleFallN(ballsToShow);
    renderSetMarbleFallX(IOGetCurrentHole());
    activeBalls = 1;
    for (int i = 0; i < ballsToShow; ++i) {
      py[i] = -2*BALL_RADIUS*(i + 1);
      vy[i] = 0;
      renderSetMarbleFallY(i, py[i]);
    }
    IOSetCurrentHoleScore(m_ballCount);
    // pd->system->logToConsole("kGameFSM_ScoresAnimation, ballsToShow %i. Current hole score is %i", ballsToShow, IOGetCurrentHoleScore());
  }
  //
  bool moving = false;
  for (int i = 0; i < activeBalls; ++i) {
    float floorY = BUF + ((maxBallsToShow - 1 - i)*2*BALL_RADIUS);
    if (py[i] > floorY) {
      py[i] = floorY;
      soundDoSfx(kBallClinkSfx);
    } else if (py[i] == floorY) {
      // noop
    } else {
      vy[i] += HISTO_BALL_ACCELERATION;
      py[i] += vy[i];
      moving = true;
    }
    renderSetMarbleFallY(i, py[i]);
    // pd->system->logToConsole("kGameFSM_ScoresAnimation, i %i y %f",i, py[i]);
  }
  if (++timer % TIME_BALL_DROP_DELAY == 0 && activeBalls < ballsToShow) { activeBalls++; }
  if (!moving) return FSMDo(kGameFSM_DisplayScores);
}


void FSMDisplayScores(const bool newState) {
  static float offset = 0;
  if (newState) { offset = 0; }

  float change = inputGetCrankChanged();
  if      (inputGetPressed(kButtonUp)) change -= 2.5f;
  else if (inputGetPressed(kButtonDown)) change += 2.5f;
  offset += change;
  offset *= 0.9f;
  gameSetYOffset(DEVICE_PIX_Y*5 + offset, true);

  if      (offset >  16.0f) return FSMDo(kGameFSM_ScoresToSplash);
  else if (offset < -16.0f) return FSMDo(kGameFSM_ScoresToTryAgain);
}

void FSMScoresToTryAgain(const bool newState) {
  static int16_t timer = 0;
  static float start = 0;
  if (newState) { 
    timer = 0;
    start = gameGetYOffset();
    soundDoSfx(kWhooshSfx1);
  }
  FSMDoCommonScrollTo(start, -DEVICE_PIX_Y - TURRET_RADIUS, (float)timer/TIME_SCORE_TO_TRY_AGAIN, EASE_SCORE_TO_TRY_AGAIN);
  if (timer++ == TIME_SCORE_TO_TRY_AGAIN) { return FSMDo(kGameFSM_DisplayLevelTitle); }
}

void FSMScoresToSplash(const bool newState) {
  static int16_t timer = 0;
  static float start = 0;
  if (newState) {
    timer = 0;
    start = gameGetYOffset();
    soundDoSfx(kWhooshSfx1);
    IODoNextHoleWithLevelWrap();
    if (!IOGetCurrentLevel() && !IOGetCurrentHole()) {
      // We've wrapped! Show credits!
      return FSMDo(kGameFSM_ToGameCreditsTitle);
    }
    bitmapDoUpdateLevelTitle();
    bitmapDoUpdateGameInfoTopper();
    // pd->system->logToConsole("kGameFSM_ScoresToSplash");
  }
  FSMDoCommonScrollTo(start, DEVICE_PIX_Y*6, (float)timer/TIME_SCORE_TO_LEVELTITLE, EASE_SCORE_TO_SPLASH);
  if (timer++ == TIME_SCORE_TO_LEVELTITLE) { return FSMDo(kGameFSM_DisplayLevelTitle); }
}

void FSMToGameCreditsTitle(const bool newState) { // Note: Separate Game and Title versions of this
  static int16_t timer = 0;
  static float start = 0;
  if (newState) {
    IOSetLevelHole(MAX_LEVELS, 0); // Enabled credits mode.
    bitmapDoUpdateGameInfoTopper();
    bitmapDoUpdateLevelTitle();
    timer = 0;
    start = gameGetYOffset();
    pd->system->removeAllMenuItems();
  }
  FSMDoCommonScrollTo(start, -DEVICE_PIX_Y - TURRET_RADIUS, (float)timer/TIME_SCORE_TO_TRY_AGAIN, EASE_SCORE_TO_TRY_AGAIN);
  if (timer++ == TIME_SCORE_TO_TRY_AGAIN) { 
    return FSMDo(kGameFSM_DisplayLevelTitle);
  }
}