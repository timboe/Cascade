#include <math.h>
#include "render.h"
#include "bitmap.h"
#include "io.h"
#include "input.h"
#include "ui.h"
#include "physics.h"
#include "board.h"

float m_trauma = 0.0f, m_decay = 0.0f;
float m_cTraumaAngle = 0.0f, m_sTraumaAngle;

uint16_t m_freeze = 0;

void renderTitles(int32_t _fc);

void renderGameWindow(int32_t _fc);

uint16_t m_ballTraceX[PREDICTION_TRAIL_LEN];
uint16_t m_ballTraceY[PREDICTION_TRAIL_LEN];
uint8_t m_ballTraces = 0;

/// ///

void setBallTrace(const uint16_t i, const uint16_t x, const uint16_t y) {
  m_ballTraceX[i] = x;
  m_ballTraceY[i] = y;
  m_ballTraces = i;
}

void addFreeze(uint16_t amount) {
  m_freeze += amount;
}

bool getSubFreeze(void) {
  if (m_freeze) {
    return m_freeze--;
  }
  return false;
}

void addTrauma(float amount) {
  m_trauma += amount;
  m_trauma *= -1;
  m_decay = amount;
  const float traumaAngle = M_2PIf / (rand() % 255);
  m_cTraumaAngle = cosf(traumaAngle) * TRAUMA_AMPLIFICATION;
  m_sTraumaAngle = sinf(traumaAngle) * TRAUMA_AMPLIFICATION;
}

void render(int32_t fc) {

  if (true && m_decay > 0.0f) {
    m_decay -= TRAUMA_DECAY;
    m_trauma += (m_trauma > 0 ? -m_decay : m_decay);
    pd->display->setOffset(m_trauma * m_cTraumaAngle, m_trauma * m_sTraumaAngle);
  } else {
    pd->display->setOffset(0, 0);
  }

  const float offX = 0.0f;
  const float offY = -getScrollOffset();

  pd->graphics->setDrawMode(kDrawModeCopy);
  pd->graphics->setDrawOffset(offX, offY);

  pd->graphics->clear(kColorWhite);

  switch (getGameMode()) {
    case kTitles: renderTitles(fc); break;
    case kGameWindow: renderGameWindow(fc); break;
    default: break;
  }

  // Draw FPS indicator (dbg only)
  #ifdef DEV
  if (ALWAYS_FPS) {
    pd->system->drawFPS(0, 0);
  }
  #endif
}

void renderTitles(int32_t _fc) {
  pd->graphics->drawBitmap(getSpriteSplash(), 0, 0, kBitmapUnflipped);
  const int i = 1;
  pd->graphics->drawBitmap(getTitleNewGameBitmap(i),
    DEVICE_PIX_X/2 - 4*TILE_PIX,
    DEVICE_PIX_Y - 2*TILE_PIX,
    kBitmapUnflipped);
  if (_fc % TICK_FREQUENCY < TICK_FREQUENCY/2) pd->graphics->drawBitmap(getTitleSelectedBitmap(),
    DEVICE_PIX_X/2  - 4*TILE_PIX,
    DEVICE_PIX_Y - TILE_PIX*2,
    kBitmapUnflipped);  
}

void renderBall(int32_t fc) {
  if (!ballInPlay()) {
    // render at dummy location
    pd->graphics->setDrawMode(kDrawModeInverted);
    pd->graphics->drawBitmap(getBitmapBall(), DEVICE_PIX_X/2 - BALL_RADIUS, getMinimumY() + TURRET_RADIUS - BALL_RADIUS, kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
    return;
  }
  cpBody* ball = getBall();
  int16_t* trailX = motionTrailX();
  int16_t* trailY = motionTrailY();
  uint8_t size = BALL_RADIUS;
  for (int32_t i = fc; i > fc - MOTION_TRAIL_LEN; --i) {
    pd->graphics->fillEllipse(trailX[i%MOTION_TRAIL_LEN] - size, trailY[i%MOTION_TRAIL_LEN] - size, 2*size, 2*size, 0.0f, 360.0f, kColorWhite);
    size -= (BALL_RADIUS / MOTION_TRAIL_LEN);
  }
  const cpVect pos = cpBodyGetPosition(ball);
  pd->graphics->setDrawMode(kDrawModeInverted);
  pd->graphics->drawBitmap(getBitmapBall(), pos.x - BALL_RADIUS, pos.y - BALL_RADIUS, kBitmapUnflipped);
  pd->graphics->setDrawMode(kDrawModeCopy);
}

void renderTurret(void) {
  const int16_t minY = getMinimumY();
  if (getScrollOffset() - minY >= 2*TURRET_RADIUS) {
    return;
  }
  //pd->graphics->drawBitmap(getBitmapHeader(), 0, 0, kBitmapUnflipped);
  pd->graphics->drawBitmap(getBitmapTurretBody(), DEVICE_PIX_X/2 - TURRET_RADIUS, minY,  kBitmapUnflipped);
  pd->graphics->drawBitmap(getBitmapTurretBarrel(), DEVICE_PIX_X/2 - TURRET_RADIUS, minY, kBitmapUnflipped);
}

void renderTrajectory(void) {
  if (ballInPlay()) {
    return;
  }
  for (int i = 2; i < m_ballTraces; ++i) {
    pd->system->logToConsole("%i is %i %i to %i %i", i, m_ballTraceX[i], m_ballTraceY[i], m_ballTraceX[i-i], m_ballTraceY[i-1]);
    pd->graphics->drawLine(m_ballTraceX[i], m_ballTraceY[i], m_ballTraceX[i-1], m_ballTraceY[i-1], 4, kColorWhite);
    pd->graphics->drawLine(m_ballTraceX[i], m_ballTraceY[i], m_ballTraceX[i-1], m_ballTraceY[i-1], 2, kColorBlack);
  }
}

void renderBackground(void) {
  const int32_t parallax = getParalaxFactorFar(); // Note: float -> int here
  const int32_t so = ((int32_t) getScrollOffset()) - UI_OFFSET_TOP - parallax;
  const uint32_t start = MAX(0, so / WF_DIVISION_PIX_Y);
  // pd->system->logToConsole("so is %i, rendering from %i to %i", so, start, start+5);
  uint8_t wf = 0;
  static uint8_t wfOffC = 0;
  int8_t wfOff = 59 - (wfOffC % 60); 
  //pd->system->logToConsole("off %i", wfOff);
  wfOffC += 2;
  // NOTE: Need to draw one extra background due to animation
  for (uint32_t i = start; i < start+6; ++i) {
    if (i >= WFSHEET_SIZE_Y) break;
    pd->graphics->drawBitmap(getBitmapWfBg(wf), WF_BG_OFFSET[wf], UI_OFFSET_TOP + (WF_DIVISION_PIX_Y * i) - wfOff + parallax, kBitmapUnflipped);
  }
  for (uint32_t i = start; i < start+5; ++i) {
    if (i >= WFSHEET_SIZE_Y) break;
    pd->graphics->drawBitmap(getBitmapWfFg(wf, 0,i), 0, UI_OFFSET_TOP + (WF_DIVISION_PIX_Y * i) + parallax, kBitmapUnflipped);
  }
  if (start == 0) {
    pd->graphics->drawBitmap(getInfoTopperBitmap(), 0, -32 + parallax, kBitmapUnflipped);
  }
}

void renderGameWindow(int32_t fc) {

  // DRAW BACKGROUND
  renderBackground();

  // DRAW TURRET & TOP DECORATION
  renderTurret();

  // DRAW BALL
  renderBall(fc);

  // DRAW OBS
  renderBoard();

  // for (uint32_t i = 0; i < N_OBST; ++i) {
  //   cpBody* obst = getBox(i);
  //   const cpVect center = cpBodyGetPosition(obst);
  //   const float y = center.y - BOX_MAX;
  //   const int off = y - getScrollOffset();
  //   if (off < 0 || off > DEVICE_PIX_Y) continue;
  //   const float x = center.x - BOX_MAX;
  //   pd->graphics->drawBitmap(getBitmapBox(cpBodyGetAngle(obst)), x, y, kBitmapUnflipped);
  // }

  // pd->graphics->fillEllipse(WFALL_PIX_X/2 - 3, 128 + UI_OFFSET_TOP - 3, 6, 6, 0.0f, 360.0f, kColorBlack);
  // pd->graphics->fillEllipse(WFALL_PIX_X/2 - 2, 128 + UI_OFFSET_TOP - 2, 4, 4, 0.0f, 360.0f, kColorWhite);

  // DRAW TRAJECTORY
  renderTrajectory();

  renderBallEndSweep();


}