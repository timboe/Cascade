#include "game.h"
#include "fsm.h"
#include "sound.h"

enum FSM_t m_FSM = (enum FSM_t)0;

/// ///

enum FSM_t FSMGet(void) { return m_FSM; }

enum FSM_t FSMUpdate(void) { 
  FSMDo(m_FSM);
  return m_FSM;
}

void FSMDo(const enum FSM_t transitionTo) {
  bool newState = (m_FSM != transitionTo);
  if (newState) { 
    m_FSM = transitionTo;
    #ifdef DEV
    pd->system->logToConsole("State change %i", (int)transitionTo);
    #endif
  }

  switch (m_FSM) {
    case kFSM_INITIAL: return;
    case kTitlesFSM_DisplayTitles: return FSMDisplayTitles(newState);
    case kTitlesFSM_DisplayTitlesWFadeIn: return FSMDisplayTitlesWFadeIn(newState);
    case kTitlesFSM_TitlesToChoosePlayer: return FSMTitlesToChoosePlayer(newState);
    case kTitlesFSM_ChoosePlayer: return FSMChoosePlayer(newState);
    case kTitlesFSM_ChoosePlayerToChooseLevel: return FSMChoosePlayerToChooseLevel(newState);
    case kTitlesFSM_ChooseLevel: return FSMChooseLevel(newState);
    case kTitlesFSM_ChooseLevelToChooseHole: return FSMChooseLevelToChooseHole(newState);
    case kTitlesFSM_ChooseLevelToChoosePlayer: return FSMChooseLevelToChoosePlayer(newState);
    case kTitlesFSM_ChooseHole: return FSMChooseHole(newState);
    case kTitlesFSM_ChooseHoleWFadeIn: return FSMChooseHoleWFadeIn(newState);
    case kTitlesFSM_ChooseHoleToLevelTitle: return FSMChooseHoleToLevelTitle(newState);
    case kTitlesFSM_ChooseHoleToChooseLevel: return FSMChooseHoleToChooseLevel(newState);
    case kTitlesFSM_ToTitleCreditsTitle: return FSMToTitlesCreditsTitle(newState);
    case kFSM_SPLIT_TitlesGame: pd->system->error("FSM error kFSM_SPLIT_TitlesGame"); return;
    case kGameFSM_DisplayLevelTitle: return FSMDisplayLevelTitle(newState);
    case kGameFSM_DisplayLevelTitleWFadeIn: return FSMDisplayLevelTitleWFadeIn(newState);
    case kGameFSM_LevelTitleToStart: return FSMLevelTitleToStart(newState);
    case kGameFSM_TutorialScrollDown: return FSMTutorialScrollDown(newState);
    case kGameFSM_TutorialScrollUp: return FSMTutorialScrollUp(newState);
    case kGameFSM_TutorialFireMarble: return FSMTutorialFireMarble(newState);
    case kGameFSM_TutorialGetSpecial: return FSMTutorialGetSpecial(newState);
    case kGameFSM_TutorialGetRequired: return FSMTutorialGetRequired(newState);
    case kGameFSM_GameFadeOutQuit: return FSMGameFadeOutQuit(newState);
    case kGameFSM_GameFadeOutReset: return FSMGameFadeOutReset(newState);
    case kGameFSM_AimMode: return FSMAimMode(newState);
    case kGameFSM_AimModeScrollToTop: return FSMAimModeScrollToTop(newState);
    case kGameFSM_BallInPlay: return FSMBallInPlay(newState);
    case kGameFSM_PlayCredits: return FSMPlayCredits(newState);
    case kGameFSM_BallStuck: return FSMBallStuck(newState);
    case kGameFSM_CloseUp: return FSMCloseUp(newState);
    case kGameFSM_WinningToast: return FSMWinningToast(newState);
    case kGameFSM_BallGutter: return FSMBallGutter(newState);
    case kGameFSM_GutterToTurret: return FSMGutterToTurret(newState);
    case kGameFSM_TurretLower: return FSMTurretLower(newState);
    case kGameFSM_GutterToScores: return FSMGutterToScores(newState);
    case kGameFSM_ScoresAnimation: return FSMScoresAnimation(newState);
    case kGameFSM_DisplayScores: return FSMDisplayScores(newState);
    case kGameFSM_ScoresToTryAgain: return FSMScoresToTryAgain(newState);
    case kGameFSM_ScoresToSplash: return FSMScoresToSplash(newState);
    case kGameFSM_ToGameCreditsTitle: return FSMToGameCreditsTitle(newState);
    case kNFSMModes: pd->system->error("FSM error kNFSMModes"); return;
  }
}

enum GameMode_t FSMGetGameMode(void) { 
  return (m_FSM < kFSM_SPLIT_TitlesGame ? kTitles : kGameWindow);
}

bool FSMGetBallInPlay(void) {
  return (m_FSM >= kGameFSM_BallInPlay && m_FSM <= kGameFSM_BallGutter);
}

bool FSMGetIsAimMode(void) {
  return (m_FSM == kGameFSM_AimMode || m_FSM == kGameFSM_TutorialFireMarble || m_FSM == kGameFSM_TutorialGetSpecial || m_FSM == kGameFSM_TutorialGetRequired);
}

float FSMDoCommonScrollTo(const int16_t origin, const int16_t destination, const float progress, const enum EasingFunction_t e) {
  const float y = origin + (destination - origin)*getEasing(e, progress);
  gameSetYOffset(y, true);
  gameSetYNotClamped();
  return y;
}
