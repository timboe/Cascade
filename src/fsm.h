#pragma once
#include "game.h"
#include "easing.h"

enum GameMode_t {
  kTitles, 
  kGameWindow,
  kNGameModes
};

enum FSM_t {
  kFSM_INITIAL,
  kTitlesFSM_DisplayTitles,
  kTitlesFSM_DisplayTitlesWFadeIn,
  kTitlesFSM_TitlesToChoosePlayer,
  kTitlesFSM_ChoosePlayer,
  kTitlesFSM_ChoosePlayerToChooseLevel,
  kTitlesFSM_ChooseLevel,
  kTitlesFSM_ChooseLevelToChooseHole,
  kTitlesFSM_ChooseLevelToChoosePlayer,
  kTitlesFSM_ChooseHole,
  kTitlesFSM_ChooseHoleWFadeIn,
  kTitlesFSM_ChooseHoleToLevelTitle,
  kTitlesFSM_ChooseHoleToChooseLevel,
  kTitlesFSM_ToTitleCreditsTitle,
  kFSM_SPLIT_TitlesGame,
  kGameFSM_DisplayLevelTitle,
  kGameFSM_DisplayLevelTitleWFadeIn,
  kGameFSM_LevelTitleToStart,
  kGameFSM_TutorialScrollDown,
  kGameFSM_TutorialScrollUp,
  kGameFSM_TutorialFireMarble,
  kGameFSM_TutorialGetSpecial,
  kGameFSM_TutorialGetRequired,
  kGameFSM_GameFadeOutQuit,
  kGameFSM_GameFadeOutReset,
  kGameFSM_AimMode,
  kGameFSM_AimModeScrollToTop,
  kGameFSM_BallInPlay, // START OF BALL-IN-PLAY
  kGameFSM_PlayCredits, // Place after BallInPlay 
  kGameFSM_BallStuck,
  kGameFSM_CloseUp,
  kGameFSM_WinningToastA,
  kGameFSM_WinningToastB,
  kGameFSM_BallGutter, // END OF BALL-IN-PLAY
  kGameFSM_GutterToTurret,
  kGameFSM_TurretLower,
  kGameFSM_GutterToScores,
  kGameFSM_ScoresAnimation,
  kGameFSM_DisplayScores,
  kGameFSM_ScoresToTryAgain,
  kGameFSM_ScoresToSplash,
  kGameFSM_ToGameCreditsTitle,
  kNFSMModes
};

void FSMDo(const enum FSM_t transitionTo);

enum FSM_t FSMUpdate(void);

enum FSM_t FSMGet(void);

enum GameMode_t FSMGetGameMode(void);

bool FSMGetBallInPlay(void);

void FSMDoResetBallStuckCounter(void);

float FSMDoCommonScrollTo(const int16_t origin, const int16_t destination, const float progress, const enum EasingFunction_t e);

bool FSMGetIsAimMode(void);

void FSMDisplayTitles(const bool newState);
void FSMDisplayTitlesWFadeIn(const bool newState);
void FSMTitlesToChoosePlayer(const bool newState);
void FSMChoosePlayer(const bool newState);
void FSMChoosePlayerToChooseLevel(const bool newState);
void FSMChooseLevel(const bool newState);
void FSMChooseLevelToChooseHole(const bool newState);
void FSMChooseLevelToChoosePlayer(const bool newState);
void FSMChooseHole(const bool newState);
void FSMChooseHoleWFadeIn(const bool newState);
void FSMChooseHoleToLevelTitle(const bool newState);
void FSMChooseHoleToChooseLevel(const bool newState);
void FSMToTitlesCreditsTitle(const bool newState);

void FSMDisplayLevelTitle(const bool newState);
void FSMDisplayLevelTitleWFadeIn(const bool newState);
void FSMLevelTitleToStart(const bool newState);
void FSMTutorialScrollDown(const bool newState);
void FSMTutorialScrollUp(const bool newState);
void FSMTutorialFireMarble(const bool newState);
void FSMTutorialGetSpecial(const bool newState);
void FSMTutorialGetRequired(const bool newState);
void FSMGameFadeOutQuit(const bool newState);
void FSMGameFadeOutReset(const bool newState);
void FSMAimMode(const bool newState);
void FSMAimModeScrollToTop(const bool newState);
void FSMPlayCredits(const bool newState);
void FSMBallInPlay(const bool newState);
void FSMBallStuck(const bool newState);
void FSMCloseUp(const bool newState);
void FSMWinningToastA(const bool newState);
void FSMWinningToastB(const bool newState);
void FSMBallGutter(const bool newState);
void FSMGutterToTurret(const bool newState);
void FSMTurretLower(const bool newState);
void FSMGutterToScores(const bool newState);
void FSMScoresAnimation(const bool newState);
void FSMDisplayScores(const bool newState);
void FSMScoresToTryAgain(const bool newState);
void FSMScoresToSplash(const bool newState);
void FSMToGameCreditsTitle(const bool newState);
