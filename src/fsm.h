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
  kTitlesFSM_TitlesToChoosePlayer,
  kTitlesFSM_ChoosePlayer,
  kTitlesFSM_ChoosePlayerToChooseLevel,
  kTitlesFSM_ChooseLevel,
  kTitlesFSM_ChooseLevelToChooseHole,
  kTitlesFSM_ChooseLevelToChoosePlayer,
  kTitlesFSM_ChooseHole,
  kTitlesFSM_ChooseHoleToSplash,
  kTitlesFSM_ChooseHoleToChooseLevel,
  kFSM_SPLIT_TitlesGame,
  kGameFSM_DisplaySplash,
  kGameFSM_SplashToStart,
  kGameFSM_TutorialScrollDown,
  kGameFSM_TutorialScrollUp,
  kGameFSM_TutorialFireMarble,
  kGameFSM_TutorialGetSpecial,
  kGameFSM_TutorialGetRequired,
  kGameFSM_AimMode,
  kGameFSM_AimModeScrollToTop,
  kGameFSM_BallInPlay,
  kGameFSM_BallStuck,
  kGameFSM_CloseUp,
  kGameFSM_WinningToast,
  kGameFSM_BallGutter,
  kGameFSM_GutterToTurret,
  kGameFSM_TurretLower,
  kGameFSM_GutterToScores,
  kGameFSM_ScoresAnimation,
  kGameFSM_DisplayScores,
  kGameFSM_ScoresToChooseHole,
  kGameFSM_ScoresToSplash,
  kNFSMModes
};

void FSMDo(const enum FSM_t transitionTo);

enum FSM_t FSMUpdate(void);

enum FSM_t FSMGet(void);

enum GameMode_t FSMGetGameMode(void);

bool FSMGetBallInPlay(void);

void FSMDoResetBallStuckCounter(void);

void FSMDoCommonScrollTo(const int16_t origin, const int16_t destination, const float progress, const enum EasingFunction_t e);

bool FSMGetIsAimMode(void);

void FSMDisplayTitles(const bool newState);
void FSMTitlesToChoosePlayer(const bool newState);
void FSMChoosePlayer(const bool newState);
void FSMChoosePlayerToChooseLevel(const bool newState);
void FSMChooseLevel(const bool newState);
void FSMChooseLevelToChooseHole(const bool newState);
void FSMChooseLevelToChoosePlayer(const bool newState);
void FSMChooseHole(const bool newState);
void FSMChooseHoleToSplash(const bool newState);
void FSMChooseHoleToChooseLevel(const bool newState);

void FSMDisplaySplash(const bool newState);
void FSMSplashToStart(const bool newState);
void FSMTutorialScrollDown(const bool newState);
void FSMTutorialScrollUp(const bool newState);
void FSMTutorialFireMarble(const bool newState);
void FSMTutorialGetSpecial(const bool newState);
void FSMTutorialGetRequired(const bool newState);
void FSMAimMode(const bool newState);
void FSMAimModeScrollToTop(const bool newState);
void FSMBallInPlay(const bool newState);
void FSMBallStuck(const bool newState);
void FSMCloseUp(const bool newState);
void FSMWinningToast(const bool newState);
void FSMBallGutter(const bool newState);
void FSMGutterToTurret(const bool newState);
void FSMTurretLower(const bool newState);
void FSMGutterToScores(const bool newState);
void FSMScoresAnimation(const bool newState);
void FSMDisplayScores(const bool newState);
void FSMScoresToChooseHole(const bool newState);
void FSMScoresToSplash(const bool newState);
