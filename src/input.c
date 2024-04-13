#include "input.h"
#include "bitmap.h"
#include "render.h"
#include "sound.h"
#include "fsm.h"
#include "io.h"
#include "physics.h"
#include "board.h"
#include "sshot.h"

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
  if (getFSM() == kTitlesFSM_DisplayTitles) { // Any button
    doFSM(kTitlesFSM_TitlesToChoosePlayer);
    return;
  }
  if (kButtonA == buttonPressed) {
    switch (getFSM()) {
      case kTitlesFSM_ChoosePlayer: doFSM(kTitlesFSM_ChoosePlayerToChooseLevel); return;
      case kTitlesFSM_ChooseLevel: doFSM(kTitlesFSM_ChooseLevelToChooseHole); return;
      case kTitlesFSM_ChooseHole: doFSM(kTitlesFSM_ChooseHoleToSplash); return;
      default: break;
    }
  } else if (kButtonB == buttonPressed) {
    switch (getFSM()) {
      case kTitlesFSM_ChooseLevel: doFSM(kTitlesFSM_ChooseLevelToChoosePlayer); return;
      case kTitlesFSM_ChooseHole: doFSM(kTitlesFSM_ChooseHoleToChooseLevel); return;
      default: break;
    }
  }
}

void clickHandleGameWindow(uint32_t buttonPressed) {
  if (kButtonB == buttonPressed  && getFSM() == kGameFSM_AimMode) {
    doScreenShot();
    doFSM(kGameFSM_AimModeScrollToTop);
  } else if ((kButtonA == buttonPressed || kButtonDown == buttonPressed) && getFSM() == kGameFSM_ScoresAnimation) {
    doFSM(kGameFSM_ScoresToSplash);
  } else if ((kButtonB == buttonPressed || kButtonUp == buttonPressed) && getFSM() == kGameFSM_ScoresAnimation) {
    doFSM(kGameFSM_ScoresToTitle);
  }
}

void clickHandlerReplacement() {
  enum kGameMode gm = getGameMode();
  PDButtons current, pushed, released = 0;
  pd->system->getButtonState(&m_current, &pushed, &released);

  if (released & kButtonUp) gameClickConfigHandler(kButtonUp);
  if (released & kButtonRight) gameClickConfigHandler(kButtonRight);
  if (released & kButtonDown) gameClickConfigHandler(kButtonDown);
  if (released & kButtonLeft) gameClickConfigHandler(kButtonLeft);
  if (released & kButtonB) gameClickConfigHandler(kButtonB);
  if (released & kButtonA) gameClickConfigHandler(kButtonA);

  m_crankChanged = pd->system->getCrankChange();
  m_crankAngle = pd->system->getCrankAngle();
}