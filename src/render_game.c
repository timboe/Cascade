#include "render.h"
#include "physics.h"
#include "bitmap.h"
#include "board.h"
#include "peg.h"
#include "sshot.h"

uint16_t m_ballPootRadius = 0.0f;

uint16_t m_ballFallN = 0;
uint16_t m_ballFallX = 0;
float m_ballFallY[32] = {0};

uint16_t m_ballTraceX[PREDICTION_TRACE_LEN * 2];
uint16_t m_ballTraceY[PREDICTION_TRACE_LEN * 2];
uint8_t m_ballTraces = 0;

/// ///

void setBallPootCircle(const uint16_t radius) { m_ballPootRadius = radius; }

void setBallFallN(const uint16_t n) { m_ballFallN = n; }
void setBallFallX(const uint16_t x) { m_ballFallX = x; }
void setBallFallY(const uint16_t ball, const float y) { m_ballFallY[ball] = y; }

void setBallTrace(const uint16_t i, const uint16_t x, const uint16_t y) {
  m_ballTraceX[i] = x;
  m_ballTraceY[i] = y;
  m_ballTraces = i;
}

void resetBallTrace(void) {
  for (int i = 0; i < PREDICTION_TRACE_LEN * 2; ++i) {
    m_ballTraceX[i] = HALF_DEVICE_PIX_X;
    m_ballTraceY[i] = gameGetMinimumY() + TURRET_RADIUS;
  }
}

void renderGameBall(const int32_t fc) {
  if (!FSMGetBallInPlay()) {
    // render at dummy location
    pd->graphics->setDrawMode(kDrawModeInverted);
    pd->graphics->drawBitmap(getBitmapBall(), DEVICE_PIX_X/2 - BALL_RADIUS, gameGetMinimumY() + TURRET_RADIUS - BALL_RADIUS, kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
    return;
  }
  for (int i = 0; i < 2; ++i) {
    if (i == 1 && !getSecondBallInPlay()) { continue; }
    cpBody* ball = getBall(i);
    int16_t* trailX = motionTrailX(i);
    int16_t* trailY = motionTrailY(i);
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
}

void renderGamePoot(const enum FSM_t fsm) {
  if (fsm == kGameFSM_AimMode && m_ballPootRadius) {
    pd->graphics->setDrawMode(kDrawModeNXOR);
    pd->graphics->drawBitmap(getBitmapAnimPoot(m_ballPootRadius), DEVICE_PIX_X/2 - TURRET_RADIUS, gameGetMinimumY(), kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
  }
}

void renderGameTurret(void) {
  const int16_t minY = gameGetMinimumY();
  const int16_t so = gameGetScrollOffset();
  if (so - minY >= 2*TURRET_RADIUS) {
    return;
  }
  pd->graphics->drawLine(0, minY+1, DEVICE_PIX_X, minY+1, 2, kColorWhite);
  // pd->graphics->drawBitmap(getBitmapHeader(), 0, 0, kBitmapUnflipped);
  pd->graphics->drawBitmap(getBitmapTurretBody(), DEVICE_PIX_X/2 - TURRET_RADIUS, minY,  kBitmapUnflipped);
  pd->graphics->drawBitmap(getBitmapTurretBarrel(), DEVICE_PIX_X/2 - TURRET_RADIUS, minY, kBitmapUnflipped);


}

void renderGameTrajectory(void) {
  if (FSMGet() != kGameFSM_AimMode) {
    return;
  }
  for (int i = 2; i < m_ballTraces; ++i) {
    // pd->system->logToConsole("%i is %i %i to %i %i", i, m_ballTraceX[i], m_ballTraceY[i], m_ballTraceX[i-i], m_ballTraceY[i-1]);
    pd->graphics->drawLine(m_ballTraceX[i], m_ballTraceY[i], m_ballTraceX[i-1], m_ballTraceY[i-1], 4, kColorWhite);
    pd->graphics->drawLine(m_ballTraceX[i], m_ballTraceY[i], m_ballTraceX[i-1], m_ballTraceY[i-1], 2, kColorBlack);
  }
}

void renderGameBoard(void) {
  for (int i = 0; i < boardGetNPegs(); ++i) {
    const struct Peg_t* p = boardGetPeg(i);
    if (p->state == kPegStateRemoved) {
      continue;
    } else if (p->state == kPegStateHit) {
      pd->graphics->setDrawMode(kDrawModeInverted);
    }
    pd->graphics->drawBitmap(p->bitmap, p->xBitmap, p->yBitmap, kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
    // if (!FSMGetBallInPlay() && !getScreenShotInProgress()) {
    //   if (p->motion == kPegMotionEllipse) {
    //     pd->graphics->fillEllipse(p->pathX[0]-3, p->pathY[0]-3, 6, 6, 0.0f, 360.0f, kColorWhite);
    //     pd->graphics->fillEllipse(p->pathX[0]-2, p->pathY[0]-2, 4, 4, 0.0f, 360.0f, kColorBlack);
    //   } else if (p->motion == kPegMotionPath) {
    //     for (int j = 1; j < p->pathSteps; ++j) {
    //       pd->graphics->drawLine(p->pathX[j], p->pathY[j], p->pathX[j-1], p->pathY[j-1], 2, kColorWhite);
    //     }
    //   }
    // }
  }
}

void renderGameBackground(void) {
  if (getScreenShotInProgress()) { return; }

  const int32_t parallax = gameGetParalaxFactorFar(); // Note: float -> int here
  const int32_t so = ((int32_t) gameGetScrollOffset()) - UI_OFFSET_TOP - parallax;
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

  const float minY = gameGetMinimumY(); 
  if (gameGetScrollOffset() - minY < 0) {
    pd->graphics->fillRect(0, minY - TURRET_RADIUS - 60, DEVICE_PIX_X, 60, kColorBlack); // mask in case of over-scroll
    pd->graphics->drawBitmap(getInfoTopperBitmap(), 0, minY - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
  }

  if (gameGetScrollOffset() <= -TURRET_RADIUS) { // Note no parallax here
    pd->graphics->drawBitmap(getLevelSplashBitmap(), 0, -DEVICE_PIX_Y - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
  }
}

void renderGameGutter(void) {
  const int32_t gutterY = WFALL_PIX_Y - ((WFALL_PIX_Y-DEVICE_PIX_Y) * PARALAX_NEAR);
  const int32_t parallax = gameGetParalaxFactorNear(); // Note: float -> int here
  const int32_t so = gameGetScrollOffset();

  if (so > gutterY + parallax - DEVICE_PIX_Y && FSMGet() != kGameFSM_ScoresToTitle) {
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