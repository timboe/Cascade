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
enum kFSM m_FSM = (enum kFSM)0;

////////////

int getFrameCount() { return m_frameCount; }

enum kFSM getFSM() { return m_FSM; }

void resetBallStuckCounter(void) { m_ballStuckCounter = 0; }

int gameLoop(void* _data) {
  ++m_frameCount;
  pd->graphics->setBackgroundColor(kColorWhite);

  if (IOOperationInProgress()) { 
    enactIO();
    return 1;
  }

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

void menuOptionsCallbackMenu(void* blank) {
  if (IOOperationInProgress()) { return; }
  pdxlog("menuOptionsCallbackMenu");
}

// Call prior to loading anything
void reset() {
  resetUI();
}

void populateMenuTitle() {
  pd->system->removeAllMenuItems();
  //pd->system->addMenuItem("delete save 3", menuOptionsCallbackDelete, (void*)2);
}

void populateMenuGame() {
  pd->system->removeAllMenuItems();
  //pd->system->addMenuItem("menu", menuOptionsCallbackMenu, NULL);
}

bool ballInPlay(void) {
  return (m_FSM >= kGameFSM_BallInPlay && m_FSM <= kGameFSM_BallGutter);
}

void commonArrowScrollAndBounceBack(void) {
  float diffY = 0;
  if      (getPressed(kButtonUp)) diffY = -SCREEN_ACC;
  else if (getPressed(kButtonDown)) diffY =  SCREEN_ACC;
  modScrollVelocity(diffY);
  applyScrollEasing();
}

enum kFSM doFSM(enum kFSM transitionTo) {

  bool newState = (m_FSM != transitionTo);
  if (newState) { 
    m_FSM = transitionTo; // Transition
  }

  ///

  if (m_FSM == kTitlesFSM_DisplayTitles) {

    if (newState) {
      setGameMode(kTitles);
    }

  } else if (m_FSM == kTitlesFSM_TitlesToSplash) {
    
    if (newState) {
      setGameMode(kGameWindow);
      setScrollOffset(-DEVICE_PIX_Y - TURRET_RADIUS, true);
      randomiseBoard();
      updateInfoTopperBitmap();
      updateLevelSplashBitmap(); 
    }
    return doFSM(kGameFSM_DisplaySplash);

  } else if (m_FSM == kGameFSM_DisplaySplash) {

    static uint16_t timer = 0;
    static const uint16_t max = TICK_FREQUENCY;
    if (newState) {
      timer = 0;
    }
    if (++timer == max) return doFSM(kGameFSM_SplashToStart);

  } else if (m_FSM == kGameFSM_SplashToStart) {

    static uint16_t timer = 0;
    static const uint16_t max = TICK_FREQUENCY/2;
    if (newState) {
      timer = 0;
      m_ballCount = 0;
    }
    ++timer;
    float progress = getEasing(EaseInOutQuint, 1.0f - (float)timer/max);
    setScrollOffset((-DEVICE_PIX_Y - TURRET_RADIUS) * progress, true);
    if (timer == max) {
      return doFSM(kGameFSM_AimMode);
    }

  } else if (m_FSM == kGameFSM_AimMode) {

    static uint16_t timer = 0;
    static const uint16_t max = TICK_FREQUENCY;
    if (newState) {
      resetBallTrace();
      timer = 0;
    }
    commonArrowScrollAndBounceBack();
    //
    float progress = getEasing(EaseOutQuart, (float)timer/max);
    if ((timer >= max) || (progress > 0.5f && !getPressed(kButtonA))) { // Fire
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
    commonArrowScrollAndBounceBack();
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

    commonArrowScrollAndBounceBack();
    //
    bool popped = true;
    if (m_frameCount % (TICK_FREQUENCY/5) == 0) {
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
      pd->system->logToConsole("kGameFSM_BallGutter");
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
    static int16_t max = TICK_FREQUENCY/2;
    if (newState) {
      timer = 0;
      pd->system->logToConsole("kGameFSM_GuttetToTurret");
    }
    const float so = getScrollOffset();
    const float progress = getEasing(EaseOutSine, (float)timer/max);
    float targetY = WFALL_PIX_Y + DEVICE_PIX_Y;
    setScrollOffset(so + (targetY - so)*progress, true);
    if (timer++ == max) {
      setBallFallN(0);
      return doFSM(kGameFSM_ScoresAnimation);
    }


  } else if (m_FSM == kGameFSM_ScoresAnimation) {

    static float py[32] = {0};
    static float vy[32] = {0};
    static uint16_t timer = 0;
    static uint16_t ballsToShow = 0;
    static uint16_t activeBalls = 1; 
    static const float ballAcceleration = 0.05f;
    static const uint16_t maxBallsToShow = 12;
    static const uint16_t ballDropDelayFrames = 8;
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
      nextHole(m_ballCount); // Giving current score count
      updateInfoTopperBitmap();
      updateLevelSplashBitmap(); 
    }
    //
    for (int i = 0; i < activeBalls; ++i) {
      const float floorY = BUF+(maxBallsToShow -1 - i)*2*BALL_RADIUS;
      if (py[i] >= floorY) {
        py[i] = floorY;
      } else {
        vy[i] += ballAcceleration;
        py[i] += vy[i];
      }
      setBallFallY(i, py[i]);
      // pd->system->logToConsole("kGameFSM_ScoresAnimation, i %i y %f",i, py[i]);
    }
    if (++timer % 8 == 0 && activeBalls < ballsToShow) { activeBalls++; }

    if (getPressed(kButtonUp))   return doFSM(kGameFSM_ScoresToTitle);
    if (getPressed(kButtonDown)) return doFSM(kGameFSM_ScoresToSplash);

  } else if (m_FSM == kGameFSM_ScoresToTitle) {

    static int16_t timer = 0;
    static int16_t max = TICK_FREQUENCY/2;
    if (newState) {
      timer = 0;
      pd->system->logToConsole("kGameFSM_ScoresToSplash");
    }
    const float so = getScrollOffset();
    const float progress = getEasing(EaseInSine, (float)timer/max);
    float targetY = WFALL_PIX_Y;
    setScrollOffset(so + (targetY - so)*progress, true);
    if (timer++ == max) {
      return doFSM(kTitlesFSM_DisplayTitles);
    }

  } else if (m_FSM == kGameFSM_ScoresToSplash) {

    static int16_t timer = 0;
    static int16_t max = TICK_FREQUENCY/2;
    if (newState) {
      timer = 0;
      pd->system->logToConsole("kGameFSM_ScoresToSplash");
    }
    const float so = getScrollOffset();
    const float progress = getEasing(EaseInSine, (float)timer/max);
    float targetY = WFALL_PIX_Y + 2*DEVICE_PIX_Y;
    setScrollOffset(so + (targetY - so)*progress, true);
    if (timer++ == max) {
      return doFSM(kGameFSM_SplashToStart);
    }

  } else {

      pd->system->error("FSM error");

  }

  return m_FSM;
}

void initGame() {
  initSpace();
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