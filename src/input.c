#include "input.h"
#include "bitmap.h"
#include "render.h"
#include "sound.h"
#include "ui.h"
#include "io.h"
#include "physics.h"
#include "board.h"

// kButtonLeft
// kButtonRight
// kButtonUp
// kButtonDown
// kButtonB
// kButtonA

PDButtons m_current;

float m_turretBarrelAngle = 180.0f;

void clickHandleTitles(uint32_t _buttonPressed);

void clickHandleGameWindow(uint32_t _buttonPressed);

void rotateHandleGameWindow(float angle, float delta);

void rotateHandleTitles(float _rotation);

/// ///


bool getPressed(PDButtons b) {
  return m_current & b;
}

bool getPressedAny() {
  return m_current;
}


void gameClickConfigHandler(uint32_t _buttonPressed) {
  switch (getGameMode()) {
    case kTitles: return clickHandleTitles(_buttonPressed);
    case kGameWindow: return clickHandleGameWindow(_buttonPressed);
    case kNGameModes: break;
  }
}

void clickHandleTitles(uint32_t _buttonPressed) {
  if (kButtonA == _buttonPressed) {
    doFSM(kTitlesFSM_TitlesToSplash);
  } else if (kButtonB == _buttonPressed) {
    //
  }
}

void clickHandleGameWindow(uint32_t _buttonPressed) {
  // if (kButtonA == _buttonPressed) {
    
  //   // if (ballInPlay()) {
  //   //   setBallInPlay(false);
  //   //   setScrollToTop(true);
  //   // } else {
  //   //   resetBall();
  //   //   setBallInPlay(true);
  //   //   setScrollToTop(false);
  //   //   launchBall();
  //   // }

  // } else if (kButtonB == _buttonPressed) {

  //   // setBallInPlay(false);
  //   // randomiseBoard();
  //   // setScrollToTop(true);

  // }

  if (kButtonB == _buttonPressed) randomiseBoard();
}

void rotateHandleTitles(float _rotation) {
 //
}

void rotateHandleGameWindow(float angle, float delta) {
  static bool topLock = true;
  static float revDetection = 180.0f;

  const bool newRev = fabsf(angle - revDetection) > 180.0f;
  revDetection = angle;

  const float so = getScrollOffset();
  if ((newRev && angle < 180.0f) || so > getMinimumY()) {
    topLock = false;
  } else if (so <= getMinimumY()) {
    topLock = true;
  }

  if (!topLock) modScrollVelocity(delta*CRANK_SCROLL_MODIFIER);

  if (angle > TURRET_ANGLE_MAX) { angle = TURRET_ANGLE_MAX; }
  else if (angle < TURRET_ANGLE_MIN) { angle = TURRET_ANGLE_MIN; }
  m_turretBarrelAngle = angle;

} 

float getTurretBarrelAngle(void) {
  return m_turretBarrelAngle;
}

void clickHandlerReplacement() {
  enum kGameMode gm = getGameMode();
  PDButtons current, pushed, released = 0;
  pd->system->getButtonState(&m_current, &pushed, &released);

  if (pushed & kButtonUp) gameClickConfigHandler(kButtonUp);
  if (pushed & kButtonRight) gameClickConfigHandler(kButtonRight);
  if (pushed & kButtonDown) gameClickConfigHandler(kButtonDown);
  if (pushed & kButtonLeft) gameClickConfigHandler(kButtonLeft);
  if (released & kButtonB) gameClickConfigHandler(kButtonB);
  if (released & kButtonA) gameClickConfigHandler(kButtonA);

  const float cc = pd->system->getCrankChange();
  if (cc) {
    switch (gm) {
      case kTitles: rotateHandleTitles(pd->system->getCrankChange()); break; 
      case kGameWindow: rotateHandleGameWindow(pd->system->getCrankAngle(), cc); break;
      default: break;
    }
  }

}