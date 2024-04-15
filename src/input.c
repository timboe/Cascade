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

void inputHandleTitles(const enum FSM_t fsm, const uint32_t buttonPressed);
void inputHandleGame(const enum FSM_t fsm, const uint32_t buttonPressed);

void inputHandlerClick(const enum FSM_t fsm, const enum GameMode_t gm, uint32_t buttonPressed);

/// ///

bool inputGetPressed(const PDButtons b) { return m_current & b; }
bool inputGetPressedAny(void) { return m_current; }

void inputSetCrankAngle(const float ca) { m_crankAngle = ca; }
float inputGetCrankAngle(void) { return m_crankAngle; }
float inputGetCrankChanged(void) { return m_crankChanged; }

void inputHandlerClick(const enum FSM_t fsm, const enum GameMode_t gm, const uint32_t buttonPressed) {
  switch (gm) {
    case kTitles: return inputHandleTitles(fsm, buttonPressed);
    case kGameWindow: return inputHandleGame(fsm, buttonPressed);
    case kNGameModes: break;
  }
}

void inputHandleTitles(const enum FSM_t fsm, const uint32_t buttonPressed) {
  if (fsm == kTitlesFSM_DisplayTitles && !IOGetIsPreloading()) { // Any button
    FSMDo(kTitlesFSM_TitlesToChoosePlayer);
    return;
  }
  if (kButtonA == buttonPressed) {
    switch (fsm) {
      case kTitlesFSM_ChoosePlayer: FSMDo(kTitlesFSM_ChoosePlayerToChooseLevel); return;
      case kTitlesFSM_ChooseLevel: FSMDo(kTitlesFSM_ChooseLevelToChooseHole); return;
      case kTitlesFSM_ChooseHole: FSMDo(kTitlesFSM_ChooseHoleToSplash); return;
      default: break;
    }
  } else if (kButtonB == buttonPressed) {
    switch (fsm) {
      case kTitlesFSM_ChooseLevel: FSMDo(kTitlesFSM_ChooseLevelToChoosePlayer); return;
      case kTitlesFSM_ChooseHole: FSMDo(kTitlesFSM_ChooseHoleToChooseLevel); return;
      default: break;
    }
  }
}

void inputHandleGame(const enum FSM_t fsm, const uint32_t buttonPressed) {
  if (kButtonB == buttonPressed && (fsm == kGameFSM_AimMode || fsm == kGameFSM_TutorialScrollUp)) {
    FSMDo(kGameFSM_AimModeScrollToTop);
  } else if ((kButtonA == buttonPressed || kButtonDown == buttonPressed) && fsm == kGameFSM_ScoresAnimation) {
    FSMDo(kGameFSM_ScoresToSplash);
  } else if ((kButtonB == buttonPressed || kButtonUp == buttonPressed) && fsm == kGameFSM_ScoresAnimation) {
    FSMDo(kGameFSM_ScoresToChooseHole);
  }
}

void inputDoHandle(const enum FSM_t fsm,const enum GameMode_t gm) {
  PDButtons current, pushed, released = 0;
  pd->system->getButtonState(&m_current, &pushed, &released);

  if (released & kButtonUp) inputHandlerClick(fsm, gm, kButtonUp);
  if (released & kButtonRight) inputHandlerClick(fsm, gm, kButtonRight);
  if (released & kButtonDown) inputHandlerClick(fsm, gm, kButtonDown);
  if (released & kButtonLeft) inputHandlerClick(fsm, gm, kButtonLeft);
  if (released & kButtonB) inputHandlerClick(fsm, gm, kButtonB);
  if (released & kButtonA) inputHandlerClick(fsm, gm, kButtonA);

  m_crankChanged = pd->system->getCrankChange();
  m_crankAngle = pd->system->getCrankAngle();
}