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
int16_t m_finalPegID = -1;

bool m_vetoCloseUp = false; // If we break out of close up, then don't do it again on this ball
uint16_t m_fizzleTimer = 0; // Prevent going straight back in to close up

void FSMCommonTurretScrollAndBounceBack(const bool allowScroll);

bool FSMCommonFocusOnLowestBallInPlay(const enum PegSpecial_t special);

uint8_t FSMCommonMarbleFire(uint16_t* timer);

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
  ballPos[0] = physicsGetBallPosition(0);
  ballPos[1] = physicsGetBallPosition(1);
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
  gameSetYOffset(y - HALF_DEVICE_PIX_Y, /*force = */ false); // Not forcec, applies y-crush
  // Return true if no balls in play
  const bool gutterd = (y > IOGetCurrentHoleHeight() + BALL_RADIUS);
  if (gutterd) {
    uint8_t whichBall = 0;
    if (y == ballPos[1].y) { whichBall = 1; }
    if (special != kPegSpecialSecondTry) { renderDoTriggerSplash(whichBall, ballPos[whichBall].x); }
  }
  return gutterd;
}

void FSMCommonTurretScrollAndBounceBack(const bool allowScroll) {
  // Button based
  float diffY = 0;
  if      (inputGetPressed(kButtonUp)) diffY = -DPAD_SCREENSCROLL_MODIFIER;
  else if (inputGetPressed(kButtonDown)) diffY =  DPAD_SCREENSCROLL_MODIFIER;
  gameModYVelocity(diffY);
  // Crank based
  static float angle = 180.0f;
  static bool topLock = true;
  static float revDetection = 180.0f;
  // Backup: non-crank
  if (pd->system->isCrankDocked()) {
    static float acc = 0.0f;
    if (inputGetPressed(kButtonLeft)) {
      acc += DPAD_ANGLESCROLL_MODIFIER / (float)TICK_FREQUENCY;
      angle += (DPAD_ANGLESCROLL_MODIFIER + acc);
    } else if (inputGetPressed(kButtonRight)) {
      acc += DPAD_ANGLESCROLL_MODIFIER / (float)TICK_FREQUENCY;
      angle -= (DPAD_ANGLESCROLL_MODIFIER + acc);
    } else {
      acc = 0;
    }
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

uint8_t FSMCommonMarbleFire(uint16_t* timer) {
  float progress = getEasing(EASE_MARBLE_FIRE, (float)(*timer)/TIME_FIRE_MARBLE);
  static bool once = false;
  const bool buttonPressed = (inputGetPressed(kButtonB) || inputGetPressed(kButtonA));
  if ((*timer) >= TIME_FIRE_MARBLE || (progress > MIN_TURRET_CHARGE_TO_FIRE && !buttonPressed)) { // Fire
    gameMenuStateSafetyReset(); // Redundency
    physicsDoResetBall(0);
    physicsDoLaunchBall(progress);
    ++m_ballCount;
    if (boardGetCurrentSpecial() == kPegSpecialMultiball) { physicsDoAddSecondBall(); }
    once = false;
    soundStopSfx(kChargeSfx);
    soundDoSfx(kPootSfx);
    const float trauma = ((*timer) >= TIME_FIRE_MARBLE ? TRAUMA_SPECIAL_HIT : TRAUMA_PEG_HIT);
    renderAddTrauma(trauma);
    renderAddFreeze(trauma);
    return 1;
  } else if (buttonPressed) { // Arm
    (*timer)++;
    if (!once) {
      once = true;
      soundDoSfx(kChargeSfx);
    }
  } else if (*timer > 0) { // Reset
    (*timer) = 0;
    once = false;
    soundStopSfx(kChargeSfx);
    // If we were holding A then we want to scroll to the top, different return value
    if (inputGetReleased(kButtonA) && gameGetYOffset() > gameGetMinimumY()) { return 2; }
  }
  renderSetMarblePootCircle((uint16_t)(progress * TURRET_RADIUS));
  return 0;
}

////////////////////////////////        ////////////////////////////////

void FSMDisplayLevelTitle(const bool newState) {
  static uint16_t timer = 0;
  static uint16_t timeToWait = 0;
  if (newState) {
    const uint32_t before = pd->system->getCurrentTimeMilliseconds(); 
    timer = 0;
    timeToWait = IOIsCredits() ? TIME_DISPLAY_SPLASH*5 : TIME_DISPLAY_SPLASH;
    gameSetYOffset(-DEVICE_PIX_Y - TURRET_RADIUS, /*force = */true);
    bitmapDoUpdateScoreHistogram();
    if (!IOIsCredits()) { gameDoPopulateMenuGame(); }
    boardDoClear();
    IODoLoadCurrentHole();
    gameMenuStateSafetyReset(); // Redundency
    gameDoResetPreviousWaterfall();
    renderDoUpdateBacklines();
    physicsDoResetBall(0);
    #ifdef DEV
    const uint32_t after = pd->system->getCurrentTimeMilliseconds();
    pd->system->logToConsole("Hole loading took %i ms", (int)(after - before));
    #endif
  }
  if (timer++ == timeToWait) return FSMDo(kGameFSM_LevelTitleToStart);
}

void FSMDisplayLevelTitleWFadeIn(const bool newState) {
  static int8_t progress;
  if (newState) {
    gameSetYOffset(-DEVICE_PIX_Y - TURRET_RADIUS, /*force = */true);
    progress = FADE_LEVELS + 2;
    soundDoSfx(kWhooshSfx1);
  }
  if (gameGetFrameCount() % ( (TICK_FREQUENCY/2) / FADE_LEVELS) == 0) { --progress; }
  renderSetFadeLevel(progress < FADE_LEVELS ? progress : FADE_LEVELS-1);
  if (progress == -1) { return FSMDo(kGameFSM_DisplayLevelTitle); }
}

void FSMLevelTitleToStart(const bool newState) {
  static uint16_t timer = 0;
  if (newState) {
    timer = 0;
    m_ballCount = 0;
    gameSetMinimumY(0);
    // soundDoSfx(kWhooshSfx1); // Doesn't fit here
  }
  FSMDoCommonScrollTo(-DEVICE_PIX_Y - TURRET_RADIUS, 0, (float)timer/TIME_SPLASH_TO_GAME, EASE_SPLASH_TO_GAME);
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

  if (inputGetReleased(kButtonA)) { return FSMDo(kGameFSM_AimModeScrollToTop); }

  FSMCommonTurretScrollAndBounceBack(true); // allow control = true
  if (gameGetYOffset() < 4) { ++timer; }
  if (timer > TICK_FREQUENCY/2) { FSMDo(kGameFSM_TutorialFireMarble); }
}

void FSMTutorialFireMarble(const bool newState) {
  static uint16_t timer = 0;
  if (newState) {
    gameSetMinimumY(0); // Tutorial only
    gameSetYOffset(0, /*force = */true);  // Tutorial only
    renderDoResetMarbleTrace();
    gameDoResetFrameCount();
    renderDoResetTriggerSplash();
    timer = 0;
  }
  //
  FSMCommonTurretScrollAndBounceBack(false); // allow control = FALSE is tutorial only
  //
  const uint8_t fired = FSMCommonMarbleFire(&timer); // Cannot scroll down, so don't need to deal with return code 2
  if      (fired == 1) { FSMDo(kGameFSM_BallInPlay); }
}

void FSMTutorialGetSpecial(const bool newState) {
  return FSMTutorialFireMarble(newState); // Functionally identical
}

void FSMTutorialGetRequired(const bool newState) {
  return FSMTutorialFireMarble(newState); // Functionally identical
}

void FSMGameFadeOutQuit(const bool newState) {
  static int8_t progress = 0;
  if (newState) { progress = 0; }
  if (gameGetFrameCount() % ( (TICK_FREQUENCY/2) / FADE_LEVELS) == 0) { ++progress; }
  if (progress == FADE_LEVELS) { 
    soundStopSfx(kFountainSfx);
    return FSMDo(kTitlesFSM_ChooseHoleWFadeIn);
  }
  renderSetFadeLevel(progress);
}

void FSMGameFadeOutReset(const bool newState) {
  static int8_t progress = 0;
  if (newState) { progress = 0; }
  if (gameGetFrameCount() % ( (TICK_FREQUENCY/2) / FADE_LEVELS) == 0) { ++progress; }
  if (progress == FADE_LEVELS) { 
    soundStopSfx(kFountainSfx);
    return FSMDo(kGameFSM_DisplayLevelTitleWFadeIn);
  }
  renderSetFadeLevel(progress);
}

void FSMAimMode(const bool newState) {
  static uint16_t timer = 0;
  if (newState) {
    renderDoResetMarbleTrace();
    gameDoResetFrameCount();
    renderDoResetTriggerSplash();
    timer = 0;
    m_vetoCloseUp = false;
    m_fizzleTimer = 0;
  }
  //
  FSMCommonTurretScrollAndBounceBack(true);
  //
  const uint8_t fired = FSMCommonMarbleFire(&timer); 
  if      (fired == 1) { FSMDo(kGameFSM_BallInPlay); }
  else if (fired == 2) { FSMDo(kGameFSM_AimModeScrollToTop); }
}

void FSMAimModeScrollToTop(const bool newState) {
  static uint16_t timer = 0;
  if (newState) { 
    timer = 0;
    soundDoSfx(kWhooshSfx1);
  }
  FSMDoCommonScrollTo(gameGetMinimumY(), gameGetYOffset(), 1.0f - (float)timer/TIME_AIM_SCROLL_TO_TOP, EASE_AIM_SCROLL_TO_TOP);
  FSMCommonTurretScrollAndBounceBack(false); // Just move turret, don't influence the scroll
  if (timer++ == TIME_AIM_SCROLL_TO_TOP) {
    gameSetYOffset(gameGetMinimumY(), /*force = */true); 
    return FSMDo(kGameFSM_AimMode);
  }
}

void FSMPlayCredits(const bool newState) {
  static float progress = 0.0f;
  static float pause = 0.0f;
  static uint16_t initialWait = 0;
  if (newState) {
    progress = gameGetYOffset();
    pause = 0.0f;
    initialWait = 0;
    renderDoResetMarbleTrace();
    gameDoResetFrameCount();
    renderDoResetTriggerSplash();
    physicsDoResetBall(0);
    gameSetTurretBarrelAngle(90.0f + 45.0f);
    physicsDoLaunchBall(1.0f);
  }

  if (progress >= gameGetMinimumY() + IOGetCurrentHoleHeight() - DEVICE_PIX_Y/3 + 4) {
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

  cpVect ballPos = physicsGetBallPosition(0);
  const bool gutterd = (ballPos.y > IOGetCurrentHoleHeight() + BALL_RADIUS);
  if (gutterd) { physicsDoSecondTryBall(); }

  if (++initialWait > TICK_FREQUENCY * 1) { progress += 0.25f; }
  progress += inputGetCrankChanged() * CRANK_SCROLL_MODIFIER;
  if (progress < gameGetMinimumY()) { progress = gameGetMinimumY(); }
  gameSetYOffset(progress, /*force = */false); // Y crush will be set by force=false, but not used here
}

void FSMBallInPlay(const bool newState) {
  const enum PegSpecial_t special = boardGetCurrentSpecial();
  static float distanceToFinalPeg = 0.0f;
  if (newState) {
    FSMDoResetBallStuckCounter();
    renderSetMarblePootCircle(0);
    m_finalRequiredPos.x = 0;
    m_finalRequiredPos.y = 0;
    m_finalPegID = -1;
    distanceToFinalPeg = 0.0f;
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
    if (m_fizzleTimer) { --m_fizzleTimer; }
    // Locate final peg (it may be moving)
    if (m_finalPegID == -1) { // Cache this
      for (int i = 0; i < boardGetNPegs(); ++i) {
        const struct Peg_t* p = boardGetPeg(i);
        if (p->type == kPegTypeRequired && p->state == kPegStateActive) {
          m_finalPegID = i;
          break;
        }
      }
    }
    const struct Peg_t* p = boardGetPeg(m_finalPegID);
    m_finalRequiredPos.x = p->x;
    m_finalRequiredPos.y = p->y;
    // Check for distance to final peg
    if (!m_vetoCloseUp && !m_fizzleTimer) {
      for (int i = 0; i < MAX_BALLS; ++i) {
        if (i == 1 && special != kPegSpecialMultiball) { break; }
        cpVect ballPos = physicsGetBallPosition(i);
        const float dist = cpvdist(m_finalRequiredPos, ballPos);
        // The ball must be travelling towards the final peg to get slowmo.
        // I.e. the distance in this frame must be smaller than the distance in the previous frame
        if (dist < FINAL_PEG_SLOWMO_RADIUS && dist < distanceToFinalPeg) {
          return FSMDo(kGameFSM_CloseUp);
        }
        distanceToFinalPeg = dist;
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
    soundDoSfx(kDrumRollSfx1);
    timer = 0.0f;
  }

  cpVect ballPos[2];
  ballPos[0] = physicsGetBallPosition(0);
  ballPos[1] = physicsGetBallPosition(1);
  timer += 1.0f / TICK_FREQUENCY;

  const struct Peg_t* p = boardGetPeg(m_finalPegID);
  m_finalRequiredPos.x = p->x;
  m_finalRequiredPos.y = p->y;

  int8_t whichBall = -1;
  for (int i = 0; i < MAX_BALLS; ++i) {
    if (i == 1 && special != kPegSpecialMultiball) { break; }
    if (cpvdist(m_finalRequiredPos, ballPos[i]) < FINAL_PEG_SLOWMO_RADIUS) {
      whichBall = i;
      break;
    }
  }

  if (timer >= 10.0f) { m_vetoCloseUp = true; }
  const bool stuck = FSMCommonGetIsBallStuck(special);

  if (stuck || m_vetoCloseUp || whichBall == -1 || !boardGetRequiredPegsInPlay()) {
    renderSetScale(1);
    gameSetXOffset(0);
    soundStopSfx(kDrumRollSfx1);
    m_fizzleTimer = FIZZLE_TIME; // Irrelevent if going to wining toast
    if (stuck) {
      soundDoSfx(kFizzleSfx);
      return FSMDo(kGameFSM_BallStuck);
    } else if (whichBall == -1 || m_vetoCloseUp) {
      soundDoSfx(kFizzleSfx);
      return FSMDo(kGameFSM_BallInPlay);
    } else {
      return FSMDo(kGameFSM_WinningToast);
    }
  }

  gameSetXOffset( roundf(ballPos[whichBall].x) - (HALF_DEVICE_PIX_X/2) );
  gameSetYOffset( roundf(ballPos[whichBall].y) - (HALF_DEVICE_PIX_Y/2), true );
  gameSetYNotClamped(); // Explicit apply y-crush
}

void FSMWinningToast(const bool newState) {
  const enum PegSpecial_t special = boardGetCurrentSpecial();
  if (newState) {
    renderDoResetEndBlast();
    renderDoAddEndBlast(physicsGetBallPosition(0));
    soundDoMusic();
    soundDoSfx(kDrumRollSfx2);
    soundDoSfx(kFountainSfx);
    physicsSetTimestepMultiplier(TSM_TARGET_TOAST);
  }

  const cpVect last = renderGetLastEndBlast();
  const cpVect ballPos = physicsGetBallPosition(0);
  if (cpvlengthsq( cpvsub(last, ballPos) ) > (16*16)) {
    renderDoAddEndBlast(physicsGetBallPosition(0));
  }

  int fc = gameGetFrameCount();
  if (physicsGetSecondBallInPlay() && fc % (TICK_FREQUENCY/5) == 0) { renderDoAddEndBlast(physicsGetBallPosition(1)); }

  const bool stuck = FSMCommonGetIsBallStuck(special);
  const bool guttered = FSMCommonFocusOnLowestBallInPlay(special);
  if (guttered || stuck) { 
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
      gameSetYOffset(gameGetMinimumY(), /*force = */true);
      soundDoSfx(kTeleportSfx);
      return FSMDo(kGameFSM_BallInPlay);
    }
    boardDoClearSpecial();
    vY = cpBodyGetVelocity(physicsGetBall(0)).y;
    pause = TICK_FREQUENCY / 2;
  }
  if (vY > 0.01f) {
    gameSetYOffset(gameGetYOffset() + (vY * TIMESTEP), true);
    gameSetYNotClamped(); // Explit apply y-crush
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
    // soundDoSfx(kWhooshSfx1); // better without?
    startY = gameGetYOffset();
    endY = gameGetMinimumY();
    // Take less time overall when we get lower down
    distance_mod = startY / (float)(startY - endY);
    // Take less time on smaller levels
    height_mod = (float)(DEVICE_PIX_Y * WF_MAX_HEIGHT) / IOGetCurrentHoleHeight();
    //
    boardDoAddSpecial(true); // Activate
    physicsDoResetBall(0);
  }
  //
  FSMCommonTurretScrollAndBounceBack(false); // Just move turret, don't influence the scroll
  //
  progress += (TIMESTEP * END_SWEEP_SCALE * distance_mod * height_mod);
  const float y = FSMDoCommonScrollTo(startY, endY, progress, EASE_GUTTER_TO_TOP);
  const float popLevel = y + (DEVICE_PIX_Y * (1.0f - getEasing(EASE_GUTTER_TO_TOP, progress)));
  // pd->system->logToConsole("FSMGutterToTurret FC %i: boardDoBurstPegs pop level %f", gameGetFrameCount(), popLevel);
  boardDoBurstPegs(popLevel);
  //
  if (progress >= 1) {
    gameSetYOffset(endY, /*force = */true);
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
    if (boardGetRequiredPegsHit() == 0) {
      // No required pegs hit yet, mave down the top active peg (required, or not)
      for (uint32_t i = 0; i < MAX_PEGS; ++i) {
        const struct Peg_t* p = boardGetPeg(i);
        if (p->state == kPegStateActive && p->minY < minY) { minY = p->minY; }
      }
    } else {
      // At least one required peg hit, move down to next required peg
      for (uint32_t i = 0; i < MAX_PEGS; ++i) {
        const struct Peg_t* p = boardGetPeg(i);
        if (p->state == kPegStateActive && p->type == kPegTypeRequired && p->minY < minY) { minY = p->minY; }
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
  }
  //
  FSMCommonTurretScrollAndBounceBack(false); // Just move turret, don't influence the scroll
  //
  progress += (TIMESTEP * speed);
  const float progressY = FSMDoCommonScrollTo(startY, endY, progress, EASE_GUTTER_TO_TOP);
  gameSetMinimumY(progressY);
  //
  if (progress >= 1) {
    gameSetYOffset(endY, /*force = */true);
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
    IOSetCurrentHoleScore(m_ballCount);
    ballsToShow = IOGetCurrentHoleScore(); // Will display the best score
    if (ballsToShow > maxBallsToShow) { ballsToShow = maxBallsToShow; }
    renderSetMarbleFallN(ballsToShow);
    renderSetMarbleFallX(IOGetCurrentHole());
    activeBalls = 1;
    for (int i = 0; i < ballsToShow; ++i) {
      py[i] = -2*BALL_RADIUS*(i + 1);
      vy[i] = 0;
      renderSetMarbleFallY(i, py[i]);
    }
    soundStopSfx(kFountainSfx);
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
  gameSetYOffset(DEVICE_PIX_Y*5 + offset, true); // Y crush is not relevent here - no background

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
    soundDoSfx(kWhooshSfx1);
    start = gameGetYOffset();
    pd->system->removeAllMenuItems();
  }
  FSMDoCommonScrollTo(start, -DEVICE_PIX_Y - TURRET_RADIUS, (float)timer/TIME_SCORE_TO_TRY_AGAIN, EASE_SCORE_TO_TRY_AGAIN);
  if (timer++ == TIME_SCORE_TO_TRY_AGAIN) { 
    return FSMDo(kGameFSM_DisplayLevelTitle);
  }
}