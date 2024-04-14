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

void renderTitles(void);
void renderGame(const int32_t fc, const enum FSM_t fsm);

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
  const float offY = -gameGetYOffset();

  pd->graphics->setDrawMode(kDrawModeCopy);
  pd->graphics->setDrawOffset(offX, offY);
  pd->display->setScale(m_renderScale);

  pd->graphics->clear(kColorWhite);
  pd->graphics->setBackgroundColor(kColorBlack);

  switch (gm) {
    case kTitles: renderTitles(); break;
    case kGameWindow: renderGame(fc, fsm); break;
    case kNGameModes: break;
  }

  // Draw FPS indicator (dbg only)
  #ifdef DEV
  if (ALWAYS_FPS && !screenShotGetInProgress()) {
    pd->system->drawFPS(0, 0);
  }
  #endif
}

void renderTitles(void) {
  const int32_t so = gameGetYOffset();

  // INTRO SPLASH
  if (so < DEVICE_PIX_Y) { renderTitlesSplash(); }

  // PLAYER SELECT
  if (so > 0 && so <= DEVICE_PIX_Y*2 ) { renderTitlesPlayerSelect(so == DEVICE_PIX_Y); }

  // LEVEL SELECT
  if (so > DEVICE_PIX_Y && so <= DEVICE_PIX_Y*3) { renderTitlesLevelSelect(so == 2*DEVICE_PIX_Y); }

  // HOLE SELECT
  if (so > DEVICE_PIX_Y*2 && so <= DEVICE_PIX_Y*4) { renderTitlesHoleSelect(so == 3*DEVICE_PIX_Y); }

  // TRANSITION LEVEL SPLASH
  if (so > DEVICE_PIX_Y*3) { renderTitlesTransitionLevelSplash(); }
}



void renderGame(int32_t fc, enum FSM_t fsm) {
  // DRAW BACKGROUND
  renderGameBackground();

  // DRAW TURRET & TOP DECORATION
  renderGameTurret();

  // DRAW BALL
  renderGameBall(fc);

  // DRAW POOT CIRCLE
  renderGamePoot(fsm);

  // DRAW PEGS
  renderGameBoard(fc);

  // DRAW TRAJECTORY
  renderGameTrajectory();

  // DRAW GUTTER
  renderGameGutter();

  // Debug gutter line
  pd->graphics->drawLine(0, WF_PIX_Y, DEVICE_PIX_X, WF_PIX_Y, 4, kColorBlack);
  pd->graphics->drawLine(0, WF_PIX_Y, DEVICE_PIX_X, WF_PIX_Y, 2, kColorWhite);
}