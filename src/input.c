#include "input.h"
#include "bitmap.h"
#include "render.h"
#include "sound.h"
#include "fsm.h"
#include "io.h"
#include "physics.h"
#include "board.h"
#include "sshot.h"

PDButtons m_current, m_pushed, m_released;
float m_crankAngle = 0;
float m_crankChanged = 0;
uint8_t m_crankNoiseAngle = 0;

void inputHandleTitles(const enum FSM_t fsm, const uint32_t buttonPressed);
void inputHandleGame(const enum FSM_t fsm, const uint32_t buttonPressed);

void inputHandlerClick(const enum FSM_t fsm, const enum GameMode_t gm, uint32_t buttonPressed);

/// ///

void inputDoInit(void) {
  m_crankAngle = pd->system->getCrankAngle();
  m_crankNoiseAngle = (uint8_t)( m_crankAngle / CRANK_NOISE_ANGLE);
}

bool inputGetReleased(const PDButtons b) { return m_released & b; }
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
      case kTitlesFSM_ChooseHole: FSMDo(kTitlesFSM_ChooseHoleToLevelTitle); return;
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
  if (fsm == kGameFSM_DisplayScores) {
    if      (kButtonB == buttonPressed) { FSMDo(kGameFSM_ScoresToSplash);   }
    else if (kButtonA == buttonPressed) { FSMDo(kGameFSM_ScoresToTryAgain); }
  }
}

void inputDoHandle(const enum FSM_t fsm,const enum GameMode_t gm) {
  pd->system->getButtonState(&m_current, &m_pushed, &m_released);

  if (m_released & kButtonUp) inputHandlerClick(fsm, gm, kButtonUp);
  if (m_released & kButtonRight) inputHandlerClick(fsm, gm, kButtonRight);
  if (m_released & kButtonDown) inputHandlerClick(fsm, gm, kButtonDown);
  if (m_released & kButtonLeft) inputHandlerClick(fsm, gm, kButtonLeft);
  if (m_released & kButtonB) inputHandlerClick(fsm, gm, kButtonB);
  if (m_released & kButtonA) inputHandlerClick(fsm, gm, kButtonA);

  m_crankChanged = pd->system->getCrankChange();
  m_crankAngle = pd->system->getCrankAngle();

  if ( (uint8_t)( m_crankAngle / CRANK_NOISE_ANGLE) != m_crankNoiseAngle) { soundDoSfx(kCrankClickSfx); }
  m_crankNoiseAngle = (uint8_t)( m_crankAngle / CRANK_NOISE_ANGLE);
}