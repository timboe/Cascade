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

uint16_t m_ballPootRadius = 0.0f;

uint16_t m_freeze = 0;

void renderTitles(int32_t fc, enum kFSM fsm);

void renderGameWindow(int32_t fc, enum kFSM fsm);

uint16_t m_ballTraceX[PREDICTION_TRACE_LEN];
uint16_t m_ballTraceY[PREDICTION_TRACE_LEN];
uint8_t m_ballTraces = 0;

/// ///

void setBallTrace(const uint16_t i, const uint16_t x, const uint16_t y) {
  m_ballTraceX[i] = x;
  m_ballTraceY[i] = y;
  m_ballTraces = i;
}

void resetBallTrace(void) {
  for (int i = 0; i < PREDICTION_TRACE_LEN; ++i) {
    m_ballTraceX[i] = HALF_DEVICE_PIX_X;
    m_ballTraceY[i] = getMinimumY() + TURRET_RADIUS;
  }
}

void setBallPootCircle(uint16_t radius) { m_ballPootRadius = radius; }


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

void render(int32_t fc, enum kFSM fsm) {

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
  pd->graphics->setBackgroundColor(kColorBlack);

  switch (getGameMode()) {
    case kTitles: renderTitles(fc, fsm); break;
    case kGameWindow: renderGameWindow(fc, fsm); break;
    default: break;
  }

  // Draw FPS indicator (dbg only)
  #ifdef DEV
  if (ALWAYS_FPS) {
    pd->system->drawFPS(0, 0);
  }
  #endif
}

void renderTitles(int32_t fc, enum kFSM fsm) {
  if (getScoreHistogram()) pd->graphics->drawBitmap(getScoreHistogram(), 0, 0, kBitmapUnflipped);

  // TEMP

  static float py[13] = {0};
  static float vy[13] = {0};
  static uint8_t n = 0;  
  static uint8_t f = 0;  

  const int16_t histoHeight = 12*2*BALL_RADIUS;
  const int16_t histoWidth = 9*3*BALL_RADIUS;
  const int16_t buf = 16;
  const int16_t tick = 4;

  if (!n) {
    n = rand() % 12 + 1;
    for (int i = 0; i < n; ++i) {
      py[i] = -2*BALL_RADIUS*(i + 1);
    }
  }

  for (int i = 0; i < n; ++i) {
    vy[i] += 0.05f;
    py[i] += vy[i];
    if (py[i] > buf+(11 - i)*2*BALL_RADIUS) {
      py[i] = buf+(11 - i)*2*BALL_RADIUS;
    }
    pd->graphics->drawBitmap(getBitmapBall(), buf + BALL_RADIUS/2 + 6*3*BALL_RADIUS, py[i], kBitmapUnflipped);
    if (i*5 > f) break;
  }
  ++f;




  return;
  // pd->graphics->drawBitmap(getSpriteSplash(), 0, 0, kBitmapUnflipped);
  const int i = 1;
  pd->graphics->drawBitmap(getTitleNewGameBitmap(i),
    DEVICE_PIX_X/2 - 4*TILE_PIX,
    DEVICE_PIX_Y - 2*TILE_PIX,
    kBitmapUnflipped);
  if (fc % TICK_FREQUENCY < TICK_FREQUENCY/2) pd->graphics->drawBitmap(getTitleSelectedBitmap(),
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
  const int16_t so = getScrollOffset();
  if (so - minY >= 2*TURRET_RADIUS) {
    return;
  }
  // pd->graphics->drawBitmap(getBitmapHeader(), 0, 0, kBitmapUnflipped);
  pd->graphics->drawBitmap(getBitmapTurretBody(), DEVICE_PIX_X/2 - TURRET_RADIUS, minY,  kBitmapUnflipped);
  pd->graphics->drawBitmap(getBitmapTurretBarrel(), DEVICE_PIX_X/2 - TURRET_RADIUS, minY, kBitmapUnflipped);
}

void renderTrajectory(void) {
  if (getFSM() != kGameFSM_AimMode) {
    return;
  }
  for (int i = 2; i < m_ballTraces; ++i) {
    // pd->system->logToConsole("%i is %i %i to %i %i", i, m_ballTraceX[i], m_ballTraceY[i], m_ballTraceX[i-i], m_ballTraceY[i-1]);
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
    if (i >= (WFSHEET_SIZE_Y - 2)) break;
    pd->graphics->drawBitmap(getBitmapWfBg(wf), WF_BG_OFFSET[wf], UI_OFFSET_TOP + (WF_DIVISION_PIX_Y * i) - wfOff + parallax, kBitmapUnflipped);
  }
  for (uint32_t i = start; i < start+5; ++i) {
    LCDBitmap* bm = getBitmapWfFg(wf, 0, i);
    if (bm) pd->graphics->drawBitmap(bm, 0, UI_OFFSET_TOP + (WF_DIVISION_PIX_Y * i) + parallax, kBitmapUnflipped);
  }

  if (start == 0) {
    pd->graphics->drawBitmap(getInfoTopperBitmap(), 0, -TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
  }

  if (getScrollOffset() <= -TURRET_RADIUS) { // Note no parallax here
    pd->graphics->drawBitmap(getLevelSplashBitmap(), 0, -DEVICE_PIX_Y - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
  }
}

void renderForeground(void) {
  const int32_t parallax = getParalaxFactorNear(); // Note: float -> int here
  const int32_t so = getScrollOffset();
  if (so > WFALL_PIX_Y - DEVICE_PIX_Y) {
    pd->graphics->drawBitmap(getBitmapWfFront(), 0, WFALL_PIX_Y + (WFALL_PIX_Y/20) + parallax, kBitmapUnflipped);
  }
}

void renderGameWindow(int32_t fc, enum kFSM fsm) {

  // DRAW BACKGROUND
  renderBackground();

  // DRAW TURRET & TOP DECORATION
  renderTurret();

  // DRAW BALL
  renderBall(fc);

  // Ball poot
  if (fsm == kGameFSM_AimMode && m_ballPootRadius) {
    pd->graphics->setDrawMode(kDrawModeNXOR);
    pd->graphics->drawBitmap(getBitmapAnimPoot(m_ballPootRadius), DEVICE_PIX_X/2 - TURRET_RADIUS, getMinimumY(), kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
  }

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

  // DRAW FOREGROUND
  renderForeground();

  renderDebug();


}