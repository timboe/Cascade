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
enum kFSM m_FSM = (enum kFSM)0;

////////////

int getFrameCount() { return m_frameCount; }

enum kFSM getFSM() { return m_FSM; }

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

enum kFSM doFSM(enum kFSM transitionTo) {

  bool newState = (m_FSM != transitionTo);
  if (newState) { 
    m_FSM = transitionTo; // Transition
  }

  ///

  if (m_FSM == kTitlesFSM_DisplayTitles) {

    //

  } else if (m_FSM == kTitlesFSM_TitlesToSplash) {
    
    if (newState) {
      setGameMode(kGameWindow);
      setScrollOffset(-DEVICE_PIX_Y - TURRET_RADIUS, true);
    }
    return doFSM(kGameFSM_DisplaySplash);

  } else if (m_FSM == kGameFSM_DisplaySplash) {

    static uint16_t timer = 0;
    if (newState) {
      timer = 50;
    }
    if (!--timer) return doFSM(kGameFSM_SplashToStart);

  } else if (m_FSM == kGameFSM_SplashToStart) {

    static uint16_t timer = 0;
    static const uint16_t max = 50;
    if (newState) {
      timer = 0;
    }
    ++timer;
    float progress = getEasing(EaseInOutQuint, 1.0f - (float)timer/max);
    setScrollOffset((-DEVICE_PIX_Y - TURRET_RADIUS) * progress, true);
    if (timer == max) {
      return doFSM(kGameFSM_AimMode);
    }

  } else if (m_FSM == kGameFSM_AimMode) {

    static uint16_t timer = 0;
    static const uint16_t max = 50;
    if (newState) {
      resetBallTrace();
      timer = 0;
    }
    float diffY = 0;
    if      (getPressed(kButtonUp)) diffY = -SCREEN_ACC;
    else if (getPressed(kButtonDown)) diffY =  SCREEN_ACC;
    modScrollVelocity(diffY);
    applyScrollEasing();
    //
    float progress = getEasing(EaseOutQuart, (float)timer/max);
    if ((timer >= max) || (progress > 0.5f && !getPressed(kButtonA))) { // Fire
      resetBall();
      launchBall(progress);
      return doFSM(kGameFSM_BallInPlay);
    } else if (getPressed(kButtonA)) { // Arm
      timer++;
    } else if (timer > 0) { // Reset
      timer = 0;
    }
    setBallPootCircle((uint16_t)(progress * TURRET_RADIUS));

  } else if (m_FSM == kGameFSM_BallInPlay) {

    float diffY = 0;
    if      (getPressed(kButtonUp)) diffY = -SCREEN_ACC;
    else if (getPressed(kButtonDown)) diffY =  SCREEN_ACC;
    modScrollVelocity(diffY);
    applyScrollEasing();
    //
    const float y = cpBodyGetPosition(getBall()).y;
    setScrollOffset(y - HALF_DEVICE_PIX_Y, false);
    //
    if (y > WFALL_PIX_Y + BALL_RADIUS) {
      return doFSM(kGameFSM_BallGutter);
    } 

  } else if (m_FSM == kGameFSM_BallStuck) {

    // TODO
    float diffY = 0;
    if      (getPressed(kButtonUp)) diffY = -SCREEN_ACC;
    else if (getPressed(kButtonDown)) diffY =  SCREEN_ACC;
    modScrollVelocity(diffY);
    applyScrollEasing();


  } else if (m_FSM == kGameFSM_CloseUp) {

    // TODO

  } else if (m_FSM == kGameFSM_BallGutter) {

    // A little extra downward camera motion
    const float y = cpBodyGetPosition(getBall()).y;
    if (y < WFALL_PIX_Y + 4*TURRET_RADIUS) {
      setScrollOffset(y - HALF_DEVICE_PIX_Y, false);
    } 

    // No more manual scroll accepted
    float vY = applyScrollEasing(); // Allow the over-scroll to correct itself
    if (!vY) {
      return doFSM(kGameFSM_GuttetToTurret);
    }

  } else if (m_FSM == kGameFSM_GuttetToTurret) {

    static float progress = 0.0f;
    if (newState) {
      progress = 0.0f;
      //
      int32_t minY = 10000;
      for (uint32_t i = 0; i < MAX_PEGS; ++i) {
        const struct Peg_t* peg = getPeg(i);
        if (peg->m_y < minY && peg->m_state == kPegStateActive) {
          minY = peg->m_y;
        }
      }
      if (minY > (DEVICE_PIX_Y/2)) setMinimumY(minY - (DEVICE_PIX_Y/2));
      pd->system->logToConsole("smallest y was %i, min y is now %i", minY, getMinimumY());
    }
    const int16_t minimumY = getMinimumY();
    // Take less time overall when we get lower down
    const float distance_mod = WFALL_PIX_Y / (float)(WFALL_PIX_Y - minimumY);

    progress += (TIMESTEP * END_SWEEP_SCALE * distance_mod);
    float easedProgress = getEasing(EaseInOutSine, progress);
    easedProgress = (1.0f - easedProgress);
    // // pd->system->logToConsole("end sweep active target %f, pop %f",target, m_popLevel);
    const float targetPartial = minimumY + ((SCROLL_OFFSET_MAX - minimumY) * easedProgress);
    setScrollOffset(targetPartial, true);
    const float popLevel = WFALL_PIX_Y * easedProgress;
    m_popLevel = popLevel; // TODO remove me
    popBoard(popLevel);
    // pd->system->logToConsole("end sweep active target %f, pop %f",target, m_popLevel);
    if (progress >= 1) {
      setScrollOffset(minimumY, true);
      return doFSM(kGameFSM_AimMode);
    }

  } else if (m_FSM == kGameFSM_GutterToScores) {

  } else if (m_FSM == kGameFSM_ScoresToTitle) {

  } else if (m_FSM == kGameFSM_ScoresToSplash) {

  } else {

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