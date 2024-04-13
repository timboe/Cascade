#include "game.h"
#include "fsm.h"

enum kFSM m_FSM = (enum kFSM)0;

///

enum kFSM getFSM(void) { return m_FSM; }

enum kFSM updateFSM(void) { 
  doFSM(m_FSM);
  return m_FSM;
}

void doFSM(enum kFSM transitionTo) {
  bool newState = (m_FSM != transitionTo);
  if (newState) { 
    m_FSM = transitionTo;
    pd->system->logToConsole("State change %i", (int)transitionTo);
  }

  switch (m_FSM) {
    case kFSM_INITIAL: return;
    case kTitlesFSM_DisplayTitles: return FSMDisplayTitles(newState);
    case kTitlesFSM_TitlesToChoosePlayer: return FSMTitlesToChoosePlayer(newState);
    case kTitlesFSM_ChoosePlayer: return FSMChoosePlayer(newState);
    case kTitlesFSM_ChoosePlayerToChooseLevel: return FSMChoosePlayerToChooseLevel(newState);
    case kTitlesFSM_ChooseLevel: return FSMChooseLevel(newState);
    case kTitlesFSM_ChooseLevelToChooseHole: return FSMChooseLevelToChooseHole(newState);
    case kTitlesFSM_ChooseLevelToChoosePlayer: return FSMChooseLevelToChoosePlayer(newState);
    case kTitlesFSM_ChooseHole: return FSMChooseHole(newState);
    case kTitlesFSM_ChooseHoleToSplash: return FSMChooseHoleToSplash(newState);
    case kTitlesFSM_ChooseHoleToChooseLevel: return FSMChooseHoleToChooseLevel(newState);
    case kFSM_SPLIT_TitlesGame: pd->system->error("FSM error kFSM_SPLIT_TitlesGame"); return;
    case kGameFSM_DisplaySplash: return FSMDisplaySplash(newState);
    case kGameFSM_SplashToStart: return FSMSplashToStart(newState);
    case kGameFSM_AimModeScrollToTop: return FSMAimModeScrollToTop(newState);
    case kGameFSM_AimMode: return FSMAimMode(newState);
    case kGameFSM_BallInPlay: return FSMBallInPlay(newState);
    case kGameFSM_BallStuck: return FSMBallStuck(newState);
    case kGameFSM_CloseUp: return FSMCloseUp(newState);
    case kGameFSM_WinningToast: return FSMWinningToast(newState);
    case kGameFSM_BallGutter: return FSMBallGutter(newState);
    case kGameFSM_GutterToTurret: return FSMGutterToTurret(newState);
    case kGameFSM_GutterToScores: return FSMGutterToScores(newState);
    case kGameFSM_ScoresAnimation: return FSMScoresAnimation(newState);
    case kGameFSM_ScoresToTitle: return FSMScoresToTitle(newState);
    case kGameFSM_ScoresToSplash: return FSMScoresToSplash(newState);
    case kNFSMModes: pd->system->error("FSM error kNFSMModes"); return;
  }
}

enum kGameMode getGameMode(void) { 
  return (m_FSM < kFSM_SPLIT_TitlesGame ? kTitles : kGameWindow);
}

bool ballInPlay(void) {
  return (m_FSM >= kGameFSM_BallInPlay && m_FSM <= kGameFSM_BallGutter);
}


void commonScrollTo(const int16_t destination, const float progress, const enum EasingFunction_t e) {
  const float so = getScrollOffset();
  setScrollOffset(so + (destination - so)*getEasing(e, progress), true);
}