#include <math.h>
#include "render.h"
#include "bitmap.h"
#include "io.h"
#include "input.h"
#include "fsm.h"
#include "board.h"
#include "sshot.h"

float m_trauma = 0.0f, m_decay = 0.0f;
float m_cTraumaAngle = 0.0f, m_sTraumaAngle;

cpVect m_backLines[N_BACKLINES];

uint16_t m_freeze = 0;

uint8_t m_renderScale = 1;

void renderTitles(const int32_t fc);
void renderGame(const int32_t fc, const enum FSM_t fsm);

void renderCommonBackground(const enum FSM_t fsm, const enum GameMode_t gm);

void renderBackgroundDo(const int16_t bg, const int16_t fg, const uint16_t startID, const int16_t wfBgOff, const int16_t parallax, const uint16_t maxY);

/// ///

void renderDoUpdateBacklines(void) {
  const uint16_t start = 16;
  const uint16_t stop = (DEVICE_PIX_Y * (WF_MAX_HEIGHT+1)) - BACKLINE_HEIGHT - 16;
  const float step = (stop - start) / (float)N_BACKLINES;
  for (int i = 0; i < N_BACKLINES; ++i) {
    do {
      m_backLines[i] = cpv( 16 * (( rand() % ((DEVICE_PIX_X/16)-1) )+1), start + (i * step));
    } while (i > 0 && m_backLines[i].x == m_backLines[i-1].x );
  }
}

void renderSetScale(const uint8_t scale) { m_renderScale = scale; }

void renderAddFreeze(const uint16_t amount) { 
  m_freeze += amount;
  if (m_freeze > TICK_FREQUENCY/5) { m_freeze = TICK_FREQUENCY/5; }
}

bool renderGetSubFreeze(void) {
  if (m_freeze) {
    return m_freeze--;
  }
  return false;
}

int16_t snap(int16_t s) {
  return s;
  //return (s / 4) * 4;
}

void renderAddTrauma(const float amount) {
  m_trauma += amount;
  m_trauma *= -1;
  m_decay = amount;
  // Trauma currently only in the Y direction
  const float traumaAngle = 0; // M_2PIf / (rand() % 255);
  m_cTraumaAngle = cosf(traumaAngle) * TRAUMA_AMPLIFICATION;
  m_sTraumaAngle = sinf(traumaAngle) * TRAUMA_AMPLIFICATION;
}

void renderDo(const int32_t fc, const enum FSM_t fsm, const enum GameMode_t gm) {
  if (!pd->system->getReduceFlashing() && m_decay > 0.0f) {
    m_decay -= TRAUMA_DECAY;
    m_trauma += (m_trauma > 0 ? -m_decay : m_decay);
    pd->display->setOffset(m_trauma * m_sTraumaAngle, m_trauma * m_cTraumaAngle);
  } else {
    pd->display->setOffset(0, 0);
  }

  const float offX = -gameGetXOffset();
  const float offY = -snap(gameGetYOffset());

  pd->graphics->setDrawMode(kDrawModeCopy);
  pd->graphics->setDrawOffset(offX, offY);
  pd->display->setScale(m_renderScale);

  pd->graphics->clear(kColorBlack);
  pd->graphics->setBackgroundColor(kColorBlack);

#ifdef TAKE_SCREENSHOTS
  pd->graphics->clear(kColorWhite);
  pd->graphics->setBackgroundColor(kColorWhite);
#endif // TAKE_SCREENSHOTS

  renderCommonBackground(fsm, gm);

  switch (gm) {
    case kTitles: renderTitles(fc); break;
    case kGameWindow: renderGame(fc, fsm); break;
    case kNGameModes: break;
  }

  // Draw FPS indicator (dbg only)
#ifdef DEV
  bool screenShotVeto = false;
#ifdef TAKE_SCREENSHOTS
  screenShotVeto = screenShotGetInProgress();
#endif // TAKE_SCREENSHOTS
  if (ALWAYS_FPS && !screenShotVeto) {
    pd->system->drawFPS(0, 0);
  }
#endif // DEV
}

void renderTitles(const int32_t fc) {
  const int32_t so = gameGetYOffset();

  // INTRO SPLASH
  if (so < DEVICE_PIX_Y) { renderTitlesHeader(fc); }

  // PLAYER SELECT
  if (so > 0 && so <= DEVICE_PIX_Y*2 ) { renderTitlesPlayerSelect(so == DEVICE_PIX_Y); }

  // LEVEL SELECT
  if (so > DEVICE_PIX_Y && so <= DEVICE_PIX_Y*3) { renderTitlesLevelSelect(so == 2*DEVICE_PIX_Y); }

  // HOLE SELECT
  if (so > DEVICE_PIX_Y*2 && so <= DEVICE_PIX_Y*5) { renderTitlesWfPond(fc); }

  // HOLE SELECT
  if (so > DEVICE_PIX_Y*2 && so <= DEVICE_PIX_Y*4) { renderTitlesHoleSelect(so == 3*DEVICE_PIX_Y); }

  // TRANSITION LEVEL SPLASH
  if (so > DEVICE_PIX_Y*4) { renderTitlesTransitionLevelSplash(); }
}

void renderGame(const int32_t fc, const enum FSM_t fsm) {

  // DRAW TOPPER
  renderGameTopper();

  // DRAW SCORES
  renderGameScores(fc);

  if (fsm == kGameFSM_ScoresToTryAgain) {
    return;
  }

  // DRAW TURRET & TOP DECORATION
  renderGameTurret();

  // DRAW TRAJECTORY
  renderGameTrajectory();

  // DRAW GUTTER
  renderGameGutter(fc);

  // DRAW PEGS
  renderGameBoard(fc);

  if (fsm == kGameFSM_BallStuck || fsm == kGameFSM_GutterToTurret || fsm == kGameFSM_WinningToast) {
    renderGamePops(fc);
  }

  if (  fsm == kGameFSM_WinningToast 
    || (fsm == kGameFSM_BallGutter && boardGetRequiredPegsInPlay() == 0)
    ||  fsm == kGameFSM_GutterToScores)
  {
    renderGameFountains(fc);
  }

  // DRAW BALL
  renderGameMarble(fc);

  if (IOGetIsTutorial()) { renderGameTutorial(fc, fsm); }

  // DRAW POOT CIRCLE
  renderGamePoot(fsm);

#ifdef DEV
  // Debug gutter line
  pd->graphics->drawLine(0, IOGetCurrentHoleHeight(), DEVICE_PIX_X, IOGetCurrentHoleHeight(), 4, kColorBlack);
  pd->graphics->drawLine(0, IOGetCurrentHoleHeight(), DEVICE_PIX_X, IOGetCurrentHoleHeight(), 2, kColorWhite);

  if (fsm == kGameFSM_GutterToTurret) {
    pd->graphics->drawLine(0, boardGetLastBurstLevel(), DEVICE_PIX_X, boardGetLastBurstLevel(), 4, kColorWhite);
    pd->graphics->drawLine(0, boardGetLastBurstLevel(), DEVICE_PIX_X, boardGetLastBurstLevel(), 2, kColorBlack);
  }
#endif
}

void renderBackgroundDo(const int16_t bg, const int16_t fg, const uint16_t startID, const int16_t wfBgOff, const int16_t parallax, const uint16_t maxY) {
  for (int i = startID; i < (startID+6); ++i) {  // Background
    const int16_t y = (WF_DIVISION_PIX_Y * i) - wfBgOff + parallax;
    if (i >= WFSHEET_SIZE_Y + 1 || y > maxY) break;
    pd->graphics->drawBitmap(bitmapGetWfBg(bg), 0/*WF_BG_OFFSET[bg]*/, y, kBitmapUnflipped);
  }
  for (int i = startID; i < (startID+5); ++i) { // Foreground
    const int16_t y = (WF_DIVISION_PIX_Y * i) + parallax;
    if (i >= WFSHEET_SIZE_Y || y > maxY) break;
    pd->graphics->drawBitmap(bitmapGetWfFg(fg, i), 0, y, kBitmapUnflipped);
  }
}

void renderCommonBackground(const enum FSM_t fsm, const enum GameMode_t gm) {

#ifdef TAKE_SCREENSHOTS
  if (screenShotGetInProgress()) { return; }
#endif

  const uint8_t prevWfBg = gameGetPreviousWaterfallBg();
  const uint8_t prevWfFg = gameGetPreviousWaterfallFg();

  const uint8_t currentWfBg = IOGetCurrentHoleWaterfallBackground(gm); 
  const uint8_t currentWfFg = IOGetCurrentHoleWaterfallForeground(gm);

  const int32_t yOffset = gameGetYOffset();
  const uint16_t chh = IOGetCurrentHoleHeight();
  const uint16_t maxY = (FSMGetGameMode() == kTitles ? (WF_MAX_HEIGHT * DEVICE_PIX_Y) : chh);
  const bool locked = (yOffset % DEVICE_PIX_Y == 0);
  const int32_t parallax = (currentWfFg >= 10 ? 0 : gameGetParalaxFactorFar(false)); // Note: float -> int here. Hard=false
  const int32_t startOffset = yOffset - parallax;
  const uint32_t startID = MAX(0, startOffset / WF_DIVISION_PIX_Y);

  // pd->system->logToConsole("startID %i, yOff is %i, paralax is %i, startOffset is %i", startID, yOffset, parallax, startOffset);

  pd->graphics->setLineCapStyle(kLineCapStyleRound);
  for (int i = 0; i < N_BACKLINES; ++i) {
    if (m_backLines[i].y - BACKLINE_HEIGHT < maxY && fsm != kGameFSM_ScoresToTryAgain) { continue; }
    if (m_backLines[i].y + BACKLINE_HEIGHT < yOffset) { continue; }
    pd->graphics->drawLine(m_backLines[i].x, m_backLines[i].y, m_backLines[i].x, m_backLines[i].y + BACKLINE_HEIGHT, BACKLINE_WIDTH, kColorWhite);
  }

  if (fsm == kGameFSM_ScoresToTryAgain) { return; }

  static float wfBgOffC = 0;
  if (!IOGetIsPreloading()) {
    wfBgOffC += WF_VELOCITY * physicsGetTimestepMultiplier();
  }
  const int16_t wfBgOff = WF_DIVISION_PIX_Y - ((int)wfBgOffC % WF_DIVISION_PIX_Y); 

  // Draw "previous" waterfall (will become current once gameDoResetPreviousWaterfall is called)
  renderBackgroundDo(prevWfBg, prevWfFg, startID, wfBgOff, parallax, maxY);

  // Animate in new waterfall
  if (currentWfFg != prevWfFg || currentWfBg != prevWfBg) {

    if (locked) {
      static uint16_t timer = 0;
      pd->graphics->setStencilImage(bitmapGetStencilWipe(timer), 0);
      renderBackgroundDo(currentWfBg, currentWfFg, startID, wfBgOff, parallax, maxY);
      pd->graphics->setStencilImage(NULL, 0);
      if (++timer == STENCIL_WIPE_N) {
        gameDoResetPreviousWaterfall();
        timer = 0;
      }
    } else {
      renderBackgroundDo(currentWfBg, currentWfFg, startID, wfBgOff, parallax, maxY);
    }
  }

  // // TEMP DEBUG
  // for (int i = 0; i < 4; ++i) {
  //   pd->graphics->drawRect(
  //   0 + (32*i), (DEVICE_PIX_Y*(i+0)) + parallax,
  //   DEVICE_PIX_X, DEVICE_PIX_Y, kColorWhite);
  // }
  // for (int i = 0; i < 4; ++i) {
  //   pd->graphics->drawRect(
  //   0           , (DEVICE_PIX_Y*(i+0)),
  //   DEVICE_PIX_X, DEVICE_PIX_Y, kColorBlack);
  // }

}