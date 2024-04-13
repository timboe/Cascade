#pragma once
#include "game.h"
#include "easing.h"

enum kGameMode {
  kTitles, 
  kGameWindow,
  kNGameModes
};

enum kFSM {
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
  kGameFSM_AimMode,
  kGameFSM_AimModeScrollToTop,
  kGameFSM_BallInPlay,
  kGameFSM_BallStuck,
  kGameFSM_CloseUp,
  kGameFSM_WinningToast,
  kGameFSM_BallGutter,
  kGameFSM_GutterToTurret,
  kGameFSM_GutterToScores,
  kGameFSM_ScoresAnimation,
  kGameFSM_ScoresToTitle,
  kGameFSM_ScoresToSplash,
  kNFSMModes
};

void doFSM(enum kFSM transitionTo);

enum kFSM updateFSM(void);

enum kFSM getFSM(void);

enum kGameMode getGameMode(void);

bool ballInPlay(void);

void resetBallStuckCounter(void);

void commonScrollTo(const int16_t destination, const float progress, const enum EasingFunction_t e);

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
void FSMAimModeScrollToTop(const bool newState);
void FSMAimMode(const bool newState);
void FSMBallInPlay(const bool newState);
void FSMBallStuck(const bool newState);
void FSMCloseUp(const bool newState);
void FSMWinningToast(const bool newState);
void FSMBallGutter(const bool newState);
void FSMGutterToTurret(const bool newState);
void FSMGutterToScores(const bool newState);
void FSMScoresAnimation(const bool newState);
void FSMScoresToTitle(const bool newState);
void FSMScoresToSplash(const bool newState);