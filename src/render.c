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

uint16_t m_ballFallN = 0;
uint16_t m_ballFallX = 0;
float m_ballFallY[32] = {0};

float m_numeralOffset = 0.0;

uint16_t m_freeze = 0;

void renderTitles(int32_t fc, enum kFSM fsm);

void renderGameWindow(int32_t fc, enum kFSM fsm);

uint16_t m_ballTraceX[PREDICTION_TRACE_LEN];
uint16_t m_ballTraceY[PREDICTION_TRACE_LEN];
uint8_t m_ballTraces = 0;

/// ///

void setNumeralOffset(float no) { m_numeralOffset = no; }

void setBallFallN(uint16_t n) { m_ballFallN = n; }

void setBallFallX(uint16_t x) { m_ballFallX = x; }

void setBallFallY(uint16_t ball, float y) { m_ballFallY[ball] = y; }

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

void commonRenderBackgroundWaterfallWithAnim(bool locked, uint8_t offY, uint16_t* timer) {
  // Draw "previous" waterfall (will become current once resetPreviousWaterfall is called)
  const uint16_t prevWf = getPreviousWaterfall();
  for (int i = offY; i < (offY+4); ++i) {
    pd->graphics->drawBitmap(getBitmapWfFg(prevWf, 0, i), 0, WF_DIVISION_PIX_Y * i, kBitmapUnflipped);
  }

  // Animate in new waterfall
  const uint16_t currentWf = getWaterfallForeground(getCurrentLevel(), 0);
  if (currentWf != prevWf) {
    if (locked) {
      pd->graphics->setStencilImage(getStencilWipe(*timer), 0);
      for (int i = offY; i < (offY+4); ++i) {
        pd->graphics->drawBitmap(getBitmapWfFg(currentWf, 0, i), 0, WF_DIVISION_PIX_Y * i, kBitmapUnflipped);
      }
      pd->graphics->setStencilImage(NULL, 0);
      if (++(*timer) == STENCIL_WIPE_N) {
        resetPreviousWaterfall();
        *timer = 0;
      }
    } else {
      for (int i = offY; i < (offY+4); ++i) {
        pd->graphics->drawBitmap(getBitmapWfFg(currentWf, 0, i), 0, WF_DIVISION_PIX_Y * i, kBitmapUnflipped);
      }
    }
  }
}

void renderTitles(int32_t fc, enum kFSM fsm) {
  const int32_t so = getScrollOffset();

  // INTRO SPLASH
  if (so < DEVICE_PIX_Y) {

    for (int i = 0; i < 4; ++i) {
      pd->graphics->drawBitmap(getBitmapWfFg(0, 0, i), 0, WF_DIVISION_PIX_Y * i, kBitmapUnflipped);
    }
    setRoobert10();
    pd->graphics->setDrawMode(kDrawModeFillWhite);
    pd->graphics->drawText(VERSION, 8, kUTF8Encoding, 8, DEVICE_PIX_Y-16);
    pd->graphics->drawText("Tim Martin, 2024", 32, kUTF8Encoding, 8, 4);
    pd->graphics->setDrawMode(kDrawModeCopy);
    pd->graphics->drawBitmap(getSpriteSplash(), 0, 0, kBitmapUnflipped);
    if (pd->system->isCrankDocked()) { 
      pd->graphics->drawBitmap(getBitmapUseTheCrank(), DEVICE_PIX_X - 88, DEVICE_PIX_Y - 51 - 16, kBitmapUnflipped);
    }
  }

  // PLAYER
  if (so > 0 && so <= DEVICE_PIX_Y*2 ) {

    static uint16_t newWaterfallTimer = 0;
    const bool locked = (so == DEVICE_PIX_Y);
    if (!locked) newWaterfallTimer = 0;
    commonRenderBackgroundWaterfallWithAnim(locked, 4, &newWaterfallTimer);

    uint8_t digit[3];
    digit[1] = getCurrentPlayer();
    digit[0] = digit[1] == 0 ? MAX_PLAYERS-1 : digit[1]-1;
    digit[2] = (digit[1] + 1) % MAX_PLAYERS;
    for (int i = 0; i < 3; ++i) { digit[i]++; }
    const float offY = (NUMERAL_PIX_Y / 2) * m_numeralOffset;

    pd->graphics->drawBitmap(getBitmapPlayer(), NUMERAL_BUF - 32, DEVICE_PIX_Y + 40, kBitmapUnflipped);
    if (!locked) {
      pd->graphics->drawBitmap(getBitmapNumeral(digit[1]),
        NUMERAL_BUF, DEVICE_PIX_Y + NUMERAL_BUF, kBitmapUnflipped);
    } else {
      pd->graphics->setScreenClipRect(NUMERAL_BUF, NUMERAL_BUF, NUMERAL_PIX_X, NUMERAL_PIX_Y);
      pd->graphics->drawBitmap(getBitmapNumeral(digit[0]),
        NUMERAL_BUF, DEVICE_PIX_Y + NUMERAL_BUF - NUMERAL_PIX_Y + offY, kBitmapUnflipped);
      pd->graphics->drawBitmap(getBitmapNumeral(digit[1]),
        NUMERAL_BUF, DEVICE_PIX_Y + NUMERAL_BUF + offY, kBitmapUnflipped);
      pd->graphics->drawBitmap(getBitmapNumeral(digit[2]),
        NUMERAL_BUF, DEVICE_PIX_Y + NUMERAL_BUF + NUMERAL_PIX_Y + offY, kBitmapUnflipped);
      pd->graphics->clearClipRect();
    }

  }

  // LEVEL
  if (so > DEVICE_PIX_Y && so <= DEVICE_PIX_Y*3) {

    static uint16_t newWaterfallTimer = 0;
    const bool locked = (so == 2*DEVICE_PIX_Y);
    if (!locked) newWaterfallTimer = 0;
    commonRenderBackgroundWaterfallWithAnim(locked, 8, &newWaterfallTimer);

    uint8_t digit0[3];
    uint8_t digit1[3];
    // +1 is because we display levels 0-98 as 1-99
    digit0[0] = (getPreviousLevel() + 1) / 10;
    digit1[0] = (getPreviousLevel() + 1) % 10;
    digit0[1] = (getCurrentLevel() + 1) / 10;
    digit1[1] = (getCurrentLevel() + 1) % 10;
    digit0[2] = (getNextLevel() + 1) / 10;
    digit1[2] = (getNextLevel() + 1) % 10;
    float offY = (NUMERAL_PIX_Y / 2) * m_numeralOffset;

    pd->graphics->drawBitmap(getBitmapLevel(), DEVICE_PIX_X - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF, kBitmapUnflipped);
    pd->graphics->drawBitmap(getBitmapLevelStats(), DEVICE_PIX_X - (2*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + NUMERAL_PIX_Y, kBitmapUnflipped);
    if (!locked) {
      pd->graphics->drawBitmap(getBitmapNumeral(digit0[1]),
        DEVICE_PIX_X - (2*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF, kBitmapUnflipped);
      pd->graphics->drawBitmap(getBitmapNumeral(digit1[1]),
        DEVICE_PIX_X - (1*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF, kBitmapUnflipped);
    } else {
      pd->graphics->setScreenClipRect(DEVICE_PIX_X - (NUMERAL_PIX_X*2) - NUMERAL_BUF, NUMERAL_BUF, NUMERAL_PIX_X*2, NUMERAL_PIX_Y);
      pd->graphics->drawBitmap(getBitmapNumeral(digit1[0]),
        DEVICE_PIX_X - (1*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF - NUMERAL_PIX_Y + offY, kBitmapUnflipped);
      pd->graphics->drawBitmap(getBitmapNumeral(digit1[1]),
        DEVICE_PIX_X - (1*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + offY, kBitmapUnflipped);
      pd->graphics->drawBitmap(getBitmapNumeral(digit1[2]),
        DEVICE_PIX_X - (1*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + NUMERAL_PIX_Y + offY, kBitmapUnflipped);
      if (m_numeralOffset < 0 && digit0[1] == digit0[2]) { offY = 0.0f; }
      if (m_numeralOffset > 0 && digit0[0] == digit0[1]) { offY = 0.0f; }
      pd->graphics->drawBitmap(getBitmapNumeral(digit0[0]),
        DEVICE_PIX_X - (2*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF - NUMERAL_PIX_Y + offY, kBitmapUnflipped);
      pd->graphics->drawBitmap(getBitmapNumeral(digit0[1]),
        DEVICE_PIX_X - (2*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + offY, kBitmapUnflipped);
      pd->graphics->drawBitmap(getBitmapNumeral(digit0[2]),
        DEVICE_PIX_X - (2*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + NUMERAL_PIX_Y + offY, kBitmapUnflipped);
      pd->graphics->clearClipRect();
    }
  }

  // HOLE
  if (so > DEVICE_PIX_Y*2 && so <= DEVICE_PIX_Y*4) {
    const uint16_t currentWf = getWaterfallForeground(getCurrentLevel(), 0);
    for (int i = 12; i < 16; ++i) {
      pd->graphics->drawBitmap(getBitmapWfFg(currentWf, 0, i), 0, WF_DIVISION_PIX_Y * i, kBitmapUnflipped);
    }
    pd->graphics->drawBitmap(getBitmapDither(), 0, (WF_DIVISION_PIX_Y * 15), kBitmapUnflipped);

    uint8_t digit[3];
    LCDBitmap* digitBm[3];
    // +1 is because we display levels 0-98 as 1-99
    digit[0] = (getPreviousHole() + 1);
    digit[1] = (getCurrentHole() + 1);
    digit[2] = (getNextHole() + 1);
    digitBm[0] = getBitmapNumeral(digit[0]);
    digitBm[1] = getBitmapNumeral(digit[1]);
    digitBm[2] = getBitmapNumeral(digit[2]);
    if (getCurrentLevel() == 0) {
      for (int i = 0; i < 3; ++i) {
        if (digit[i] == 1) { // Remmber we have already +1 for display
          digitBm[i] = getBitmapHoleTutorial();
        }
      }
    } 
    const float offY = (NUMERAL_PIX_Y / 2) * m_numeralOffset;
    const bool locked = (so == 3*DEVICE_PIX_Y);

    pd->graphics->drawBitmap(getBitmapHole(), NUMERAL_BUF - 32, (DEVICE_PIX_Y*3) + NUMERAL_BUF, kBitmapUnflipped);
    pd->graphics->drawBitmap(getBitmapHoleStatsA(), NUMERAL_BUF, (DEVICE_PIX_Y*3) + NUMERAL_BUF - 32, kBitmapUnflipped);
    // Note this one is extra wide
    pd->graphics->drawBitmap(getBitmapHoleStatsB(), 0, (DEVICE_PIX_Y*3) + NUMERAL_BUF + NUMERAL_PIX_Y, kBitmapUnflipped);
    if (!locked) {
      pd->graphics->drawBitmap(digitBm[1],
        NUMERAL_BUF, (DEVICE_PIX_Y*3) + NUMERAL_BUF, kBitmapUnflipped);
    } else {
      pd->graphics->setScreenClipRect(NUMERAL_BUF, NUMERAL_BUF, NUMERAL_PIX_X, NUMERAL_PIX_Y);
      pd->graphics->drawBitmap(digitBm[0],
        NUMERAL_BUF, (DEVICE_PIX_Y*3) + NUMERAL_BUF - NUMERAL_PIX_Y + offY, kBitmapUnflipped);
      pd->graphics->drawBitmap(digitBm[1],
        NUMERAL_BUF, (DEVICE_PIX_Y*3) + NUMERAL_BUF + offY, kBitmapUnflipped);
      pd->graphics->drawBitmap(digitBm[2],
        NUMERAL_BUF, (DEVICE_PIX_Y*3) + NUMERAL_BUF + NUMERAL_PIX_Y + offY, kBitmapUnflipped);
      pd->graphics->clearClipRect();
    }
  }

  // LEVEL SPLASH
  if (so > DEVICE_PIX_Y*3) {
    pd->graphics->drawBitmap(getLevelSplashBitmap(), 0, DEVICE_PIX_Y*4, kBitmapUnflipped);
  }

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
  pd->graphics->drawLine(0, minY+1, DEVICE_PIX_X, minY+1, 2, kColorWhite);
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

  const float minY = getMinimumY(); 
  if (getScrollOffset() - minY < 0) {
    pd->graphics->fillRect(0, minY - TURRET_RADIUS - 60, DEVICE_PIX_X, 60, kColorBlack); // mask in case of over-scroll
    pd->graphics->drawBitmap(getInfoTopperBitmap(), 0, minY - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
  }

  if (getScrollOffset() <= -TURRET_RADIUS) { // Note no parallax here
    pd->graphics->drawBitmap(getLevelSplashBitmap(), 0, -DEVICE_PIX_Y - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
  }
}

void renderGutter(void) {
  const int32_t gutterY = WFALL_PIX_Y - ((WFALL_PIX_Y-DEVICE_PIX_Y) * PARALAX_NEAR);
  const int32_t parallax = getParalaxFactorNear(); // Note: float -> int here
  const int32_t so = getScrollOffset();

  if (so > gutterY + parallax - DEVICE_PIX_Y && getFSM() != kGameFSM_ScoresToTitle) {
    pd->graphics->drawBitmap(getBitmapWfPond(), 0, gutterY + parallax, kBitmapUnflipped);
    pd->graphics->drawRect(0, gutterY + parallax, DEVICE_PIX_X, DEVICE_PIX_Y, kColorWhite);
    pd->graphics->drawRect(1, gutterY + parallax + 1, DEVICE_PIX_X-2, DEVICE_PIX_Y-2, kColorBlack);
  }
  //Note no parallax here
  if (so > WFALL_PIX_Y) {
    pd->graphics->drawBitmap(getScoreHistogram(), 0, WFALL_PIX_Y + DEVICE_PIX_Y, kBitmapUnflipped);
    for (int i = 0; i < m_ballFallN; ++i) {
      pd->graphics->drawBitmap(getBitmapBall(),
        BUF + BALL_RADIUS/2 + m_ballFallX*3*BALL_RADIUS,
        WFALL_PIX_Y + DEVICE_PIX_Y + m_ballFallY[i],
        kBitmapUnflipped);
    }
  }
  if (so > WFALL_PIX_Y + DEVICE_PIX_Y) {
    pd->graphics->drawBitmap(getLevelSplashBitmap(), 0, WFALL_PIX_Y + (2*DEVICE_PIX_Y), kBitmapUnflipped);
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

  // DRAW GUTTER
  renderGutter();

  renderDebug();


}