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

uint16_t m_freeze = 0;

uint8_t m_renderScale = 1;

void renderTitles(const int32_t fc);
void renderGame(const int32_t fc, const enum FSM_t fsm);

void renderCommonBackground(void);

/// ///

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
  const float traumaAngle = M_2PIf / (rand() % 255);
  m_cTraumaAngle = cosf(traumaAngle) * TRAUMA_AMPLIFICATION;
  m_sTraumaAngle = sinf(traumaAngle) * TRAUMA_AMPLIFICATION;
}

void renderDo(const int32_t fc, const enum FSM_t fsm, const enum GameMode_t gm) {
  if (!pd->system->getReduceFlashing() && m_decay > 0.0f) {
    m_decay -= TRAUMA_DECAY;
    m_trauma += (m_trauma > 0 ? -m_decay : m_decay);
    pd->display->setOffset(m_trauma * m_cTraumaAngle, m_trauma * m_sTraumaAngle);
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

  renderCommonBackground();

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
  if (so > DEVICE_PIX_Y*2 && so <= DEVICE_PIX_Y*4) { renderTitlesHoleSelect(so == 3*DEVICE_PIX_Y); }

  // HOLE SELECT
  if (so > DEVICE_PIX_Y*3 && so <= DEVICE_PIX_Y*5) { renderTitlesWfPond(); }

  // TRANSITION LEVEL SPLASH
  if (so > DEVICE_PIX_Y*4) { renderTitlesTransitionLevelSplash(); }
}



void renderGame(const int32_t fc, const enum FSM_t fsm) {
  // DRAW TURRET & TOP DECORATION
  renderGameTurret();

  // DRAW TRAJECTORY
  renderGameTrajectory();

  // DRAW GUTTER
  renderGameGutter(fc);

  // DRAW PEGS
  renderGameBoard(fc);

  // DRAW BALL
  renderGameMarble(fc);

  if (IOGetIsTutorial()) { renderGameTutorial(fc, fsm); }

  // DRAW POOT CIRCLE
  renderGamePoot(fsm);

#ifdef DEV
  // Debug gutter line
  pd->graphics->drawLine(0, IOGetCurrentHoleHeight(), DEVICE_PIX_X, IOGetCurrentHoleHeight(), 4, kColorBlack);
  pd->graphics->drawLine(0, IOGetCurrentHoleHeight(), DEVICE_PIX_X, IOGetCurrentHoleHeight(), 2, kColorWhite);
#endif
}

void renderBackgroundDo(int16_t bg, int16_t fg, uint16_t startID, int16_t wfBgOff, int16_t parallax) {
  for (int i = startID; i < (startID+6); ++i) {  // Background
    if (i >= WFSHEET_SIZE_Y + 1) break;
    pd->graphics->drawBitmap(bitmapGetWfBg(bg), WF_BG_OFFSET[bg], (WF_DIVISION_PIX_Y * i) - wfBgOff + parallax, kBitmapUnflipped);
  }
  for (int i = startID; i < (startID+5); ++i) { // Foreground
    if (i >= WFSHEET_SIZE_Y) break;
    pd->graphics->drawBitmap(bitmapGetWfFg(fg, i), 0, (WF_DIVISION_PIX_Y * i) + parallax, kBitmapUnflipped);
  }
}

void renderCommonBackground(void) {

#ifdef TAKE_SCREENSHOTS
  if (screenShotGetInProgress()) { return; }
#endif

  const int32_t yOffset = gameGetYOffset();
  const bool locked = (yOffset % DEVICE_PIX_Y == 0);
  const int32_t parallax = gameGetParalaxFactorFar(false); // Note: float -> int here. Hard=false
  const int32_t startOffset = yOffset - parallax;
  const uint32_t startID = MAX(0, startOffset / WF_DIVISION_PIX_Y);

  // pd->system->logToConsole("startID %i, yOff is %i, paralax is %i, startOffset is %i", startID, yOffset, parallax, startOffset);

  static float wfBgOffC = 0;
  if (!IOGetIsPreloading()) {
    wfBgOffC += WF_VELOCITY * physicsGetTimestepMultiplier();
  }
  const int16_t wfBgOff = WF_DIVISION_PIX_Y - ((int)wfBgOffC % WF_DIVISION_PIX_Y); 

  // Draw "previous" waterfall (will become current once gameDoResetPreviousWaterfall is called)
  const uint8_t prevWfBg = gameGetPreviousWaterfallBg();
  const uint8_t prevWfFg = gameGetPreviousWaterfallFg();
  renderBackgroundDo(prevWfBg, prevWfFg, startID, wfBgOff, parallax);

  // Animate in new waterfall
  const uint8_t currentWfBg = IOGetCurrentHoleWaterfallBackground(); 
  const uint8_t currentWfFg = IOGetCurrentHoleWaterfallForeground();

  if (currentWfFg != prevWfFg || currentWfBg != prevWfBg) {

    if (locked) {
      static uint16_t timer = 0;
      pd->graphics->setStencilImage(bitmapGetStencilWipe(timer), 0);
      renderBackgroundDo(currentWfBg, currentWfFg, startID, wfBgOff, parallax);
      pd->graphics->setStencilImage(NULL, 0);
      if (++timer == STENCIL_WIPE_N) {
        gameDoResetPreviousWaterfall();
        timer = 0;
      }
    } else {
      renderBackgroundDo(currentWfBg, currentWfFg, startID, wfBgOff, parallax);
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

  if (FSMGetGameMode() == kGameWindow) {
    const float minY = gameGetMinimumY(); 
    if (gameGetYOffset() - minY < 0) {
      pd->graphics->fillRect(0, minY - TURRET_RADIUS - 60, DEVICE_PIX_X, 60, kColorBlack); // mask in case of over-scroll
      pd->graphics->drawBitmap(bitmapGetGameInfoTopper(), 0, minY - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
    }
    if (gameGetYOffset() <= -TURRET_RADIUS) { // Note no parallax here
      pd->graphics->drawBitmap(bitmapGetLevelTitle(), 0, -DEVICE_PIX_Y - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
    }
  }

}