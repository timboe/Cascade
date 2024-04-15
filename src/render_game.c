#include "render.h"
#include "physics.h"
#include "bitmap.h"
#include "board.h"
#include "peg.h"
#include "sshot.h"
#include "io.h"

uint16_t m_ballPootRadius = 0.0f;

uint16_t m_ballFallN = 0;
uint16_t m_ballFallX = 0;
float m_ballFallY[32] = {0};

cpVect m_ballTrace[PREDICTION_TRACE_LEN * 2]; //x2 for the special aim ability
uint8_t m_ballTraces = 0;

int16_t m_ballSplashPos[MAX_BALLS];
uint16_t m_ballSplashTimer[MAX_BALLS];

cpVect m_starPos[MAX_STARS];
cpVect m_starVel[MAX_STARS];
uint8_t m_starAng[MAX_STARS];
int8_t m_starType[MAX_STARS] = {-1};

cpVect m_blastPos = cpvzero;
uint8_t m_blastFrame;

cpVect m_specialPos = cpvzero;
uint8_t m_specialOffset = 0;
enum PegSpecial_t m_specialType;

/// ///

void renderDoAddSpecial(cpBody* body, const enum PegSpecial_t special) {
  m_specialPos = cpBodyGetPosition(body);
  m_specialOffset = 0;
  m_specialType = special;
}

void renderDoAddBlast(cpBody* body) {
  m_blastPos = cpBodyGetPosition(body);
  m_blastFrame = 0;
}

void renderDoAddStar(const uint8_t ball) {
  for (int s = 0; s < MAX_STARS; ++s) {
    if (m_starType[s] != -1) { continue; }
    cpBody* cpBall = physicsGetBall(ball);
    const float ang =  (rand() % 180) * M_PIf;
    m_starType[s] = rand() % 2;
    m_starPos[s] = cpBodyGetPosition(cpBall);
    m_starVel[s] = cpv(STAR_STRENGTH * cosf(ang), -STAR_STRENGTH * sinf(ang));
    // m_starVel[s] = cpvadd( cpBodyGetVelocity(cpBall), m_starVel[s] );
    m_starVel[s] = cpvmult( m_starVel[s], TIMESTEP );
    m_starAng[s] = rand() % 128;
    return;
  }
}

void renderDoResetStars(void) {
  for (int s = 0; s < MAX_STARS; ++s) { m_starType[s] = -1; }
}

void renderDoTriggerSplash(const uint8_t ball, const int16_t x) {
  if (!m_ballSplashTimer[ball]) {
    m_ballSplashPos[ball] = x - POND_SPLASH_WIDTH/2;
    m_ballSplashTimer[ball] = 1;
  }
}

void renderDoResetTriggerSplash(void) {
  for (int i = 0; i < MAX_BALLS; ++i) { m_ballSplashTimer[i] = 0; }
}

void renderSetBallPootCircle(const uint16_t radius) { m_ballPootRadius = radius; }

void renderSetBallFallN(const uint16_t n) { m_ballFallN = n; }
void renderSetBallFallX(const uint16_t x) { m_ballFallX = x; }
void renderSetBallFallY(const uint16_t ball, const float y) { m_ballFallY[ball] = y; }

void renderSetBallTrace(const uint16_t i, const uint16_t x, const uint16_t y) {
  m_ballTrace[i].x = x;
  m_ballTrace[i].y = y;
  m_ballTraces = i;
}

void renderDoResetBallTrace(void) {
  for (int i = 0; i < PREDICTION_TRACE_LEN * 2; ++i) {
    m_ballTrace[i].x = HALF_DEVICE_PIX_X;
    m_ballTrace[i].y = gameGetMinimumY() + TURRET_RADIUS;
  }
}

void renderGameBall(const int32_t fc) {
  // Start out by rendering any end of level effects
  const enum FSM_t fsm = FSMGet();
  if (fsm >= kGameFSM_WinningToast && fsm <= kGameFSM_GutterToScores) {
    for (int s = 0; s < MAX_STARS; ++s) {
      if (m_starType[s] == -1) { continue; }
      m_starPos[s] = cpvadd( m_starPos[s], m_starVel[s] );
      m_starVel[s].y += 0.01f; 
      m_starAng[s] += 3;
      pd->graphics->setDrawMode(kDrawModeNXOR);
      pd->graphics->drawBitmap(bitmapGetStar(m_starType[s], m_starAng[s]), m_starPos[s].x - STAR_WIDTH/2, m_starPos[s].y - STAR_WIDTH/2, kBitmapUnflipped);
      pd->graphics->setDrawMode(kDrawModeCopy);
      if (m_starAng[s] > 250) { m_starType[s] = -1; }
    }
  }

  const int32_t gutterY = IOGetCurrentHoleHeight();
  const float pfn = gameGetParalaxFactorNear(true);
  const int32_t parallaxPond = pfn - gameGetParalaxFactorNearForY(true, gutterY - DEVICE_PIX_Y);

  if (!FSMGetBallInPlay()) {
    // render at dummy location
    pd->graphics->setDrawMode(kDrawModeInverted);
    pd->graphics->drawBitmap(bitmapGetMarble(), DEVICE_PIX_X/2 - BALL_RADIUS, gameGetMinimumY() + TURRET_RADIUS - BALL_RADIUS, kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
  } else {
    for (int i = 0; i < MAX_BALLS; ++i) {
      if (i == 1 && !physicsGetSecondBallInPlay()) { continue; }
      const cpBody* ball = physicsGetBall(i);
      const cpVect pos = cpBodyGetPosition(ball);
      if (pos.y > gutterY) { continue; }
      int16_t* trailX = physicsGetMotionTrailX(i);
      int16_t* trailY = physicsGetMotionTrailY(i);
      uint8_t size = BALL_RADIUS;
      for (int32_t i = fc; i > fc - MOTION_TRAIL_LEN; --i) {
        pd->graphics->fillEllipse(trailX[i%MOTION_TRAIL_LEN] - size, trailY[i%MOTION_TRAIL_LEN] - size, 2*size, 2*size, 0.0f, 360.0f, kColorWhite);
        size -= (BALL_RADIUS / MOTION_TRAIL_LEN);
      }
      pd->graphics->setDrawMode(kDrawModeInverted);
      pd->graphics->drawBitmap(bitmapGetMarble(), pos.x - BALL_RADIUS, pos.y - BALL_RADIUS, kBitmapUnflipped);
      pd->graphics->setDrawMode(kDrawModeCopy);
    }
  }

  // Splash
  for (int i = 0; i < MAX_BALLS; ++i) {
    if (m_ballSplashTimer[i]) {
      uint8_t frame = m_ballSplashTimer[i] / 4;
      if (frame == 9)  frame = 7; // Repeat
      if (frame == 10) frame = 6; // Repeat
      if (frame >= 9) continue;
      pd->graphics->setDrawMode(kDrawModeNXOR);
      pd->graphics->drawBitmap(bitmapGetWaterSplash(frame), m_ballSplashPos[i], gutterY + parallaxPond, kBitmapUnflipped);
      pd->graphics->setDrawMode(kDrawModeCopy);
      ++m_ballSplashTimer[i];
    }
  }
}

void renderGamePoot(const enum FSM_t fsm) {
  if (FSMGetIsAimMode() && m_ballPootRadius) {
    pd->graphics->setDrawMode(kDrawModeNXOR);
    pd->graphics->drawBitmap(bitmapGetMarbleFirePoot(m_ballPootRadius), DEVICE_PIX_X/2 - TURRET_RADIUS, gameGetMinimumY(), kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
  }
}

void renderGameTurret(void) {
  const int16_t minY = gameGetMinimumY();
  const int16_t so = gameGetYOffset();
  if (so - minY >= 2*TURRET_RADIUS) {
    return;
  }
  pd->graphics->drawLine(0, minY+1, DEVICE_PIX_X, minY+1, 2, kColorWhite);
  pd->graphics->drawBitmap(bitmapGetTurretBody(), DEVICE_PIX_X/2 - TURRET_RADIUS, minY,  kBitmapUnflipped);
  pd->graphics->drawBitmap(bitmapGetTurretBarrel(), DEVICE_PIX_X/2 - TURRET_RADIUS, minY, kBitmapUnflipped);
}

void renderGameTrajectory(void) {
  if (!FSMGetIsAimMode()) {
    return;
  }
  for (int i = 2; i < m_ballTraces; ++i) {
    pd->graphics->drawLine(m_ballTrace[i].x, m_ballTrace[i].y, m_ballTrace[i-1].x, m_ballTrace[i-1].y, 4, kColorWhite);
    pd->graphics->drawLine(m_ballTrace[i].x, m_ballTrace[i].y, m_ballTrace[i-1].x, m_ballTrace[i-1].y, 2, kColorBlack);
  }
}

void renderGameBoard(const int32_t fc) {
#ifdef TAKE_SCREENSHOTS
  if (screenShotGetInProgress()) {
    for (int i = 0; i < boardGetNPegs(); ++i) {
      const struct Peg_t* p = boardGetPeg(i);
      if (p->motion == kPegMotionEllipse) {
        pd->graphics->drawEllipse(p->pathX[0], p->pathY[0], p->a, p->a, 4, 0.0f, 360.0f, kColorBlack);
      } else if (p->motion == kPegMotionPath) {
        for (int j = 1; j < p->pathSteps; ++j) {
          pd->graphics->drawLine(p->pathX[j], p->pathY[j], p->pathX[j-1], p->pathY[j-1], 4, kColorBlack);
        }
      }
    }
  }
#endif
  for (int i = 0; i < boardGetNPegs(); ++i) {
    const struct Peg_t* p = boardGetPeg(i);
    if (p->state == kPegStateRemoved) {
      continue;
    } else if (p->state == kPegStateHit) {
      pd->graphics->setDrawMode(kDrawModeInverted);
    }
    pd->graphics->drawBitmap(p->bitmap, p->xBitmap, p->yBitmap, kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
  }

  if (m_specialPos.x) { // Draw special obtained toast
    m_specialOffset++;
    if (m_specialOffset == TICK_FREQUENCY) {
      m_specialPos = cpvzero;
    } else {
      pd->graphics->setDrawMode(kDrawModeNXOR);
      pd->graphics->drawBitmap(bitmapGetSpecial(m_specialType),
        m_specialPos.x - SPECIAL_TEXT_WIDTH/2,
        m_specialPos.y - TITLETEXT_HEIGHT - m_specialOffset, kBitmapUnflipped);
      pd->graphics->setDrawMode(kDrawModeCopy);
    }
  }

  if (m_blastPos.x) { // Draw blast graphic
    m_blastFrame++;
    if (m_blastFrame / 4 == 9) {
      m_blastPos = cpvzero;
    } else {
      pd->graphics->setDrawMode(kDrawModeCopy); // kDrawModeNXOR ?
      pd->graphics->drawBitmap(bitmapGetBlast(m_blastFrame / 4), m_blastPos.x - BLAST_RADIUS, m_blastPos.y - BLAST_RADIUS, kBitmapUnflipped);
      // pd->graphics->setDrawMode(kDrawModeCopy);
    }
  }

}

void renderGameBackground(void) {
#ifdef TAKE_SCREENSHOTS
  if (screenShotGetInProgress()) { return; }
#endif

  const int32_t parallax = gameGetParalaxFactorFar(false); // Note: float -> int here. Hard=false
  const int32_t so = ((int32_t) gameGetYOffset()) - parallax;
  const uint32_t start = MAX(0, so / WF_DIVISION_PIX_Y);
  // pd->system->logToConsole("so is %i, rendering from %i to %i", so, start, start+5);
  uint8_t wf = 0;
  static float wfOffC = 0;
  int16_t wfOff = 59 - ((int)wfOffC % 60); 
  //pd->system->logToConsole("off %i", wfOff);
  wfOffC += WF_VELOCITY * physicsGetTimestepMultiplier();
  // NOTE: Need to draw one extra background due to animation
  for (uint32_t i = start; i < start+6; ++i) {
    if (i >= (WFSHEET_SIZE_Y - 2)) break;
    if (WF_DIVISION_PIX_Y * i > IOGetCurrentHoleHeight()) { break; }
    pd->graphics->setDrawMode(kDrawModeInverted); // TODO fix in the gfx
    pd->graphics->drawBitmap(bitmapGetWfBg(wf), WF_BG_OFFSET[wf], (WF_DIVISION_PIX_Y * i) - wfOff + parallax, kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
  }
  for (uint32_t i = start; i < start+5; ++i) {
    LCDBitmap* bm = bitmapGetWfFg(wf, 0, i);
    if (WF_DIVISION_PIX_Y * i > IOGetCurrentHoleHeight()) { break; }
    if (bm) pd->graphics->drawBitmap(bm, 0, (WF_DIVISION_PIX_Y * i) + parallax, kBitmapUnflipped);
  }

  const float minY = gameGetMinimumY(); 
  if (gameGetYOffset() - minY < 0) {
    pd->graphics->fillRect(0, minY - TURRET_RADIUS - 60, DEVICE_PIX_X, 60, kColorBlack); // mask in case of over-scroll
    pd->graphics->drawBitmap(bitmapGetGameInfoTopper(), 0, minY - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
  }

  if (gameGetYOffset() <= -TURRET_RADIUS) { // Note no parallax here
    pd->graphics->drawBitmap(bitmapGetLevelTitle(), 0, -DEVICE_PIX_Y - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
  }
}

void renderGameGutter(void) {
#ifdef TAKE_SCREENSHOTS
  if (screenShotGetInProgress()) { return; }
#endif

  const int32_t gutterY = IOGetCurrentHoleHeight();
  const float pfn = gameGetParalaxFactorNear(true);
  const int32_t parallaxPond = pfn - gameGetParalaxFactorNearForY(true, gutterY - DEVICE_PIX_Y); // Note: float -> int here. Hard = true
  const int32_t so = gameGetYOffset();

  if (so > gutterY + parallaxPond - DEVICE_PIX_Y) {
    pd->graphics->drawBitmap(bitmapGetWfPond(), 0, gutterY + parallaxPond, kBitmapUnflipped);
    pd->graphics->drawRect(0, gutterY, DEVICE_PIX_X, DEVICE_PIX_Y, kColorWhite);
    pd->graphics->drawRect(1, gutterY + 1, DEVICE_PIX_X-2, DEVICE_PIX_Y-2, kColorBlack);
  }

  //Note no parallax here
  if (so > DEVICE_PIX_Y * 4) {
    pd->graphics->drawBitmap(BitmapGetScoreHistogram(), 0, DEVICE_PIX_Y * 5, kBitmapUnflipped);
    for (int i = 0; i < m_ballFallN; ++i) {
      pd->graphics->drawBitmap(bitmapGetMarble(),
        BUF + BALL_RADIUS/2 + m_ballFallX*3*BALL_RADIUS,
        (DEVICE_PIX_Y * 5) + m_ballFallY[i],
        kBitmapUnflipped);
    }
  }

  if (so > DEVICE_PIX_Y * 5) {
    pd->graphics->drawBitmap(bitmapGetLevelTitle(), 0, DEVICE_PIX_Y * 6, kBitmapUnflipped);
  }
}

void renderGameTutorial(const int32_t fc, const enum FSM_t fsm) {
  const int16_t yOff = gameGetYOffset();
    const uint16_t fast = TICK_FREQUENCY / 5;
    const uint16_t slow = TICK_FREQUENCY / 2;
  if (fsm == kGameFSM_TutorialScrollDown) {

    if (pd->system->isCrankDocked()) {
      pd->graphics->drawBitmap(bitmapGetTutorialDPad((fc / slow) % 2), 20, 40 + yOff, kBitmapFlippedY);
    } else {
      pd->graphics->drawBitmap(bitmapGetTutorialCrankRotate(fc / fast), 20, 20 + yOff, kBitmapUnflipped);
    }

  } else if (fsm == kGameFSM_TutorialScrollUp) {

    if (pd->system->isCrankDocked()) {
      pd->graphics->drawBitmap(bitmapGetTutorialDPad((fc / slow) % 2), 20, 40 + yOff, kBitmapUnflipped);
    } else {
      pd->graphics->drawBitmap(bitmapGetTutorialCrankRotate(fc / fast), 20, 20 + yOff, kBitmapFlippedX);
    }
    pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 2 ? 1 : 3) , HALF_DEVICE_PIX_X + 20, 60 + yOff, kBitmapUnflipped);

  } else if (fsm == kGameFSM_TutorialFireMarble) {

    if (pd->system->isCrankDocked()) {
      const uint8_t f = (fc / slow) % 4;
      const LCDBitmapFlip flip = (f == 2 ? kBitmapFlippedX : kBitmapUnflipped);
      pd->graphics->drawBitmap(bitmapGetTutorialDPad(f % 2 ? 0 : 2), 20, 40 + yOff, flip);
    } else {
      uint8_t f = (fc / fast) % 8;
      if (f > 4) f = 4 - (f - 4);
      pd->graphics->drawBitmap(bitmapGetTutorialCrankAngle(f), 20, 65 + yOff, kBitmapUnflipped);
    }
    pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 2 ? 0 : 2) , HALF_DEVICE_PIX_X + 20, 60 + yOff, kBitmapUnflipped);

  } else if (fsm == kGameFSM_TutorialGetSpecial) {

    if (pd->system->isCrankDocked()) {
      const uint8_t f = (fc / slow) % 4;
      if (f < 2) pd->graphics->drawBitmap(bitmapGetTutorialDPad((fc / slow) % 2 ? 2 : 0), HALF_DEVICE_PIX_X + 20, 40 + yOff, kBitmapUnflipped);
      else       pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 2 ? 2 : 0) , HALF_DEVICE_PIX_X + 20, 60 + yOff, kBitmapUnflipped);
    } else {
      pd->graphics->drawBitmap(bitmapGetTutorialCrankAngle(0), HALF_DEVICE_PIX_X + 20, 5 + yOff, kBitmapUnflipped);
      pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 2 ? 0 : 2) , HALF_DEVICE_PIX_X + 20, 5 + 110 + yOff, kBitmapUnflipped);
    }
    if ((fc / slow) % 2) { pd->graphics->drawBitmap(bitmapGetTutorialArrows(0), 0, yOff, kBitmapUnflipped); }

  } else if (fsm == kGameFSM_TutorialGetRequired) {

    if (pd->system->isCrankDocked()) {
      const uint8_t f = (fc / slow) % 4;
      if (f < 2) pd->graphics->drawBitmap(bitmapGetTutorialDPad((fc / slow) % 2 ? 2 : 0), 20, 40 + yOff, kBitmapFlippedX);
      else       pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 2 ? 2 : 0), 20, 60 + yOff, kBitmapUnflipped);
    } else {
      pd->graphics->drawBitmap(bitmapGetTutorialCrankAngle(4), 20, 5 + yOff, kBitmapUnflipped);
      pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 2 ? 0 : 2), 20, 5 + 110 + yOff, kBitmapUnflipped);
    }
    if ((fc / slow) % 2) { pd->graphics->drawBitmap(bitmapGetTutorialArrows(1), HALF_DEVICE_PIX_X, yOff, kBitmapUnflipped); }

  }

}