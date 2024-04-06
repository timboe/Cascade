#include "input.h"
#include "bitmap.h"
#include "render.h"
#include "sound.h"
#include "ui.h"
#include "io.h"
#include "physics.h"
#include "board.h"

PDButtons m_current;

float m_crankAngle = 0;
float m_crankChanged = 0;

void clickHandleTitles(uint32_t buttonPressed);

void clickHandleGameWindow(uint32_t buttonPressed);

/// ///


bool getPressed(PDButtons b) { return m_current & b; }

bool getPressedAny(void) { return m_current; }

float getCrankAngle(void) { return m_crankAngle; }

float getCrankChanged(void) { return m_crankChanged; }

void gameClickConfigHandler(uint32_t buttonPressed) {
  switch (getGameMode()) {
    case kTitles: return clickHandleTitles(buttonPressed);
    case kGameWindow: return clickHandleGameWindow(buttonPressed);
    case kNGameModes: break;
  }
}

void clickHandleTitles(uint32_t buttonPressed) {
  if (kButtonA == buttonPressed) {
    switch (getFSM()) {
      case kTitlesFSM_DisplayTitles: doFSM(kTitlesFSM_PlayerSelectToLevelSelect); return;
      case kTitlesFSM_ChoosePlayer: doFSM(kTitlesFSM_PlayerSelectToLevelSelect); return;
      case kTitlesFSM_ChooseLevel: doFSM(kTitlesFSM_ChooseLevelToChooseHole); return;
      case kTitlesFSM_ChooseHole: doFSM(kTitlesFSM_ChooseHoleToSplash); return;
      default: break;
    }
  } else if (kButtonB == buttonPressed) {
    switch (getFSM()) {
      case kTitlesFSM_ChooseLevel: doFSM(kTitlesFSM_ChooseLevelToPlayerSelect); return;
      case kTitlesFSM_ChooseHole: doFSM(kTitlesFSM_ChooseHoleToLevelSelect); return;
      default: break;
    }
  }
}

void clickHandleGameWindow(uint32_t buttonPressed) {
  if (kButtonA == buttonPressed && getFSM() == kGameFSM_ScoresAnimation) {
    doFSM(kGameFSM_ScoresToSplash);
  } else if (kButtonB == buttonPressed && getFSM() == kGameFSM_ScoresAnimation) {
    doFSM(kGameFSM_ScoresToTitle);
  }
}

// void rotateHandleTitles(float _rotation) {
//  //
// }

// void rotateHandleGameWindow(float angle, float delta) {
//   static bool topLock = true;
//   static float revDetection = 180.0f;

//   const bool newRev = fabsf(angle - revDetection) > 180.0f;
//   revDetection = angle;

//   const float so = getScrollOffset();
//   if ((newRev && angle < 180.0f) || so > getMinimumY()) {
//     topLock = false;
//   } else if (so <= getMinimumY()) {
//     topLock = true;
//   }

//   if (!topLock) modScrollVelocity(delta*CRANK_SCROLL_MODIFIER);

//   if (angle > TURRET_ANGLE_MAX) { angle = TURRET_ANGLE_MAX; }
//   else if (angle < TURRET_ANGLE_MIN) { angle = TURRET_ANGLE_MIN; }
//   m_turretBarrelAngle = angle;

// } 


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

  m_crankChanged = pd->system->getCrankChange();
  m_crankAngle = pd->system->getCrankAngle();
  // if (cc) {
  //   switch (gm) {
  //     case kTitles: rotateHandleTitles(pd->system->getCrankChange()); break; 
  //     case kGameWindow: rotateHandleGameWindow(pd->system->getCrankAngle(), cc); break;
  //     default: break;
  //   }
  // }

}