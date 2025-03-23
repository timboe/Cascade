#include "render.h"
#include "physics.h"
#include "bitmap.h"
#include "board.h"
#include "peg.h"
#include "sshot.h"
#include "io.h"
#include "sound.h"

uint16_t m_ballPootRadius = 0.0f;

uint16_t m_ballFallN = 0;
uint16_t m_ballFallX = 0;
float m_ballFallY[32] = {0};

cpVect m_ballTrace[PREDICTION_TRACE_LEN * 2]; //x2 for the special aim ability
uint8_t m_ballTraces = 0;

int16_t m_ballSplashPos[MAX_BALLS];
uint16_t m_ballSplashTimer[MAX_BALLS];

uint8_t m_endBlasts = 0;
int8_t m_endBlastID[MAX_END_BLASTS] = {-1};
cpVect m_endBlast[MAX_END_BLASTS];
uint8_t m_endBlastFrame[MAX_END_BLASTS] = {0};

cpVect m_blastPos = cpvzero;
uint8_t m_blastFrame;

cpVect m_specialPos = cpvzero;
uint8_t m_specialOffset = 0;
enum PegSpecial_t m_specialType;

void renderEndBlasts(const int32_t fc);

/// ///

void renderDoAddEndBlast(const cpVect location) {
  m_endBlast[m_endBlasts] = location;
  m_endBlastID[m_endBlasts] = rand() % N_END_BLASTS;
  m_endBlastFrame[m_endBlasts] = 0;
  if (++m_endBlasts == MAX_END_BLASTS) { m_endBlasts = 0; }
}

void renderDoResetEndBlast(void) {
  for (int i = 0; i < MAX_END_BLASTS; ++i) { m_endBlastID[i] = -1; }
}

cpVect renderGetLastEndBlast(void) {
  if (m_endBlasts == 0) return m_endBlast[MAX_END_BLASTS-1];
  return m_endBlast[m_endBlasts-1];
}

void renderDoAddSpecial(cpBody* body, const enum PegSpecial_t special) {
  m_specialPos = cpBodyGetPosition(body);
  m_specialOffset = 0;
  m_specialType = special;
}

void renderDoAddSpecialBlast(cpBody* body) {
  m_blastPos = cpBodyGetPosition(body);
  m_blastFrame = 0;
}

void renderDoTriggerSplash(const uint8_t ball, const int16_t x) {
  if (!m_ballSplashTimer[ball]) {
    m_ballSplashPos[ball] = x - POND_SPLASH_WIDTH/2;
    m_ballSplashTimer[ball] = 1;
    soundDoSfx(kSplashSfx1);
  }
}

void renderDoResetTriggerSplash(void) {
  for (int i = 0; i < MAX_BALLS; ++i) { m_ballSplashTimer[i] = 0; }
}

void renderSetMarblePootCircle(const uint16_t radius) { m_ballPootRadius = radius; }

void renderSetMarbleFallN(const uint16_t n) { m_ballFallN = n; }
void renderSetMarbleFallX(const uint16_t x) { m_ballFallX = x; }
void renderSetMarbleFallY(const uint16_t ball, const float y) { m_ballFallY[ball] = y; }

void renderSetMarbleTrace(const cpVect v, const uint16_t i) {
  m_ballTrace[i] = v;
  m_ballTraces = i;
}

void renderDoResetMarbleTrace(void) {
  for (int i = 0; i < PREDICTION_TRACE_LEN * 2; ++i) {
    m_ballTrace[i].x = HALF_DEVICE_PIX_X;
    m_ballTrace[i].y = gameGetMinimumY() + TURRET_RADIUS;
  }
}

void renderEndBlasts(const int32_t fc) {
  for (int s = 0; s < MAX_END_BLASTS; ++s) {
    if (m_endBlastID[s] == -1) { continue; }
    pd->graphics->setDrawMode(kDrawModeXOR);
    pd->graphics->drawBitmap(bitmapGetEndBlast(m_endBlastID[s], m_endBlastFrame[s]), m_endBlast[s].x - END_BLAST_HWIDTH, m_endBlast[s].y - END_BLAST_HWIDTH, kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
    if (fc % 2 == 0) {
      if (++m_endBlastFrame[s] == END_BLAST_FRAMES) {
        m_endBlastID[s] = -1;
      }
    }
  }
}

void renderGameMarble(const int32_t fc, const enum FSM_t fsm) {
#ifdef TAKE_SCREENSHOTS
  return;
#endif

  // Start out by rendering any end of level effects

  // Draw end blasts
  if (fsm >= kGameFSM_WinningToastA && fsm <= kGameFSM_GutterToScores) {
    renderEndBlasts(fc);
  }

  const int32_t gutterY = IOGetCurrentHoleHeight();
  const float yOff = gameGetYOffset();
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
      const cpVect pos = physicsGetBallPosition(i);
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

      // Over-top
      if (pos.y < yOff && !IOIsCredits()) {
        pd->graphics->drawBitmap(bitmapGetFwBkwIcon(2), pos.x - FW_BKW_WIDTH/2, yOff, kBitmapUnflipped);
      }
    }
  }

  // Splash
  for (int i = 0; i < MAX_BALLS; ++i) {
    if (m_ballSplashTimer[i]) {
      uint8_t frame = m_ballSplashTimer[i] / 4;
      if (frame >= POND_SPLASH_ANIM_FRAMES) continue;
      pd->graphics->setDrawMode(kDrawModeXOR);
      pd->graphics->drawBitmap(bitmapGetWaterSplash(frame), m_ballSplashPos[i], gutterY - POND_SPLASH_HEIGHT, kBitmapUnflipped);
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
  pd->graphics->drawBitmap(bitmapGetTurretBody(boardGetCurrentSpecial()), DEVICE_PIX_X/2 - TURRET_RADIUS, minY,  kBitmapUnflipped);
#ifndef TAKE_SCREENSHOTS
  pd->graphics->drawBitmap(bitmapGetTurretBarrel(), DEVICE_PIX_X/2 - TURRET_RADIUS, minY, kBitmapUnflipped);
#endif
}

void renderGameTrajectory(void) {
  if (!FSMGetIsAimMode()) {
    return;
  }
  pd->graphics->setLineCapStyle(kLineCapStyleSquare);
  for (int i = 2; i < m_ballTraces; ++i) {
    pd->graphics->drawLine(round(m_ballTrace[i].x), round(m_ballTrace[i].y), round(m_ballTrace[i-1].x), round(m_ballTrace[i-1].y), 8, kColorWhite);
  }
  for (int i = 2; i < m_ballTraces; ++i) {
    pd->graphics->drawLine(round(m_ballTrace[i].x), round(m_ballTrace[i].y), round(m_ballTrace[i-1].x), round(m_ballTrace[i-1].y), 3, kColorBlack);
  }
}

void renderGameSpecials(const int32_t fc) {
  if (m_specialPos.x) { // Draw special obtained toast
    m_specialOffset++;
    if (m_specialOffset == TICK_FREQUENCY) {
      m_specialPos = cpvzero;
    } else {
      int16_t x = m_specialPos.x - SPECIAL_TEXT_WIDTH/2;
      if (x < 16) { x = 16; }
      else if (x + SPECIAL_TEXT_WIDTH > DEVICE_PIX_X - 16) { x = DEVICE_PIX_X - SPECIAL_TEXT_WIDTH - 16; }
      pd->graphics->setDrawMode(kDrawModeCopy); // kDrawModeNXOR
      pd->graphics->drawBitmap(bitmapGetSpecial(m_specialType),
        x, m_specialPos.y - TITLETEXT_HEIGHT - m_specialOffset, kBitmapUnflipped);
      pd->graphics->setDrawMode(kDrawModeCopy);
    }
  }

  if (m_blastPos.x) { // Draw explosion graphic
    m_blastFrame++;
    const uint8_t animFrame = m_blastFrame / 4;
    if (animFrame == 1 && !pd->system->getReduceFlashing()) { // Flash
      pd->graphics->setDrawMode(kDrawModeInverted);
      pd->graphics->fillRect(0, gameGetYOffset(), DEVICE_PIX_X, DEVICE_PIX_Y, kColorWhite);
    }
    if (animFrame == 4) {
      soundSetDoingExplosion(false);
    }
    if (animFrame == SPECIAL_BLAST_FRAMES) { // Explosion blast is over
      m_blastPos = cpvzero;
    } else {
      pd->graphics->setDrawMode(kDrawModeCopy);
      pd->graphics->drawBitmap(bitmapGetSpecialBlast(animFrame), m_blastPos.x - SPECIAL_BLAST_RADIUS, m_blastPos.y - SPECIAL_BLAST_RADIUS, kBitmapUnflipped);
    }
  }
}


void renderGameBoard(const int32_t fc) {
#ifdef TAKE_SCREENSHOTS
  if (screenShotGetInProgress()) {
    for (int i = 0; i < boardGetNPegs(); ++i) {
      const struct Peg_t* p = boardGetPeg(i);
      if (p->motion == kPegMotionEllipse && p->speed) {
        pd->graphics->drawEllipse(p->pathX[0] - p->a, p->pathY[0] - p->b, p->a*2, p->b*2, 4, 0.0f, 360.0f, kColorBlack);
      } else if (p->motion == kPegMotionPath && p->speed) {
        for (int j = 1; j < p->pathSteps; ++j) {
          pd->graphics->drawLine(p->pathX[j], p->pathY[j], p->pathX[j-1], p->pathY[j-1], 4, kColorBlack);
        }
      }
    }
  }
#endif

  for (int i = 0; i < boardGetNPegs(); ++i) {
    const struct Peg_t* p = boardGetPeg(i);
    if (p->state == kPegStateRemoved || p->y < 0 || p->y > IOGetCurrentHoleHeight()) {
      continue;
    } else if (p->state == kPegStateHit) {
      pd->graphics->setDrawMode(kDrawModeInverted);
    }
    pd->graphics->drawBitmap(p->bitmap, p->xBitmap, p->yBitmap, kBitmapUnflipped);
    pd->graphics->setDrawMode(kDrawModeCopy);
  }

}

void renderGamePops(const int32_t fc) {
  pd->graphics->setDrawMode(kDrawModeXOR);
  for (int i = 0; i < boardGetNPegs(); ++i) {
    struct Peg_t* p = boardGetPeg(i);
    if (p->popFrame >= 0 && p->popFrame < POP_ANIM_FRAMES) {
      LCDBitmap* bm = bitmapGetPegPop(p);
      pd->graphics->drawBitmap(bm, p->x - POP_ANIM_HWIDTH, p->y - POP_ANIM_HWIDTH, kBitmapUnflipped);
      if (fc % 2 == 0) { p->popFrame++; }
    }
  }
  pd->graphics->setDrawMode(kDrawModeCopy);
}

void renderGameFountains(const int32_t fc) {
  pd->graphics->setDrawMode(kDrawModeXOR);
  for (int i = 0; i < 3; ++i) {
    const int16_t x = (i*128) + ((i+1) * 4);
    pd->graphics->drawBitmap(bitmapGetFountain(i, fc / 2), x, IOGetCurrentHoleHeight() - 128, kBitmapUnflipped);
  }
  pd->graphics->setDrawMode(kDrawModeCopy);
}


void renderGameTopper(void) {
  const float minY = gameGetMinimumY(); 
  const float yOff = gameGetYOffset();
  if (yOff - minY < 0 && FSMGet() != kGameFSM_ScoresToTryAgain) {
    pd->graphics->fillRect(0, minY - TURRET_RADIUS - 60, DEVICE_PIX_X, 60, kColorBlack); // mask in case of over-scroll
    pd->graphics->drawBitmap(bitmapGetGameInfoTopper(), 0, minY - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
  }
  if (yOff <= -TURRET_RADIUS) { // Note no parallax here
    pd->graphics->drawBitmap(bitmapGetLevelTitle(), 0, -DEVICE_PIX_Y - TURRET_RADIUS, kBitmapUnflipped); //Note no parallax here
  }
}

void renderGameGutter(const int32_t fc) {
#ifdef TAKE_SCREENSHOTS
  if (screenShotGetInProgress()) { return; }
#endif

  const int32_t gutterY = IOGetCurrentHoleHeight();
  const float pfn = gameGetParalaxFactorNear(true);
  const int32_t parallaxPond = pfn - gameGetParalaxFactorNearForY(true, gutterY - DEVICE_PIX_Y); // Note: float -> int here. Hard = true
  const int32_t so = gameGetYOffset();

  for (int i = 0; i < POND_WATER_TILES; ++i) {
    const int16_t offset = (i * POND_WATER_HEIGHT);
    if (so > gutterY + parallaxPond + offset - DEVICE_PIX_Y) {
      pd->graphics->drawBitmap(bitmapGetWfPond(i, fc), 0, gutterY + parallaxPond + offset, kBitmapUnflipped);
    }
  }
  // DEBUG
  // pd->graphics->drawRect(0, gutterY, DEVICE_PIX_X, (POND_WATER_HEIGHT*POND_WATER_TILES), kColorWhite);
  // pd->graphics->drawRect(1, gutterY + 1, DEVICE_PIX_X-2, (POND_WATER_HEIGHT*POND_WATER_TILES)-2, kColorBlack);
}

void renderGameScores(const int32_t fc, const enum FSM_t fsm) {
  const int32_t so = gameGetYOffset();

  //Note no parallax here
  if (so > DEVICE_PIX_Y * 4) {
    pd->graphics->drawBitmap(bitmapGetScoreHistogram(), 0, DEVICE_PIX_Y * 5, kBitmapUnflipped);
    for (int i = 0; i < m_ballFallN; ++i) {
      pd->graphics->drawBitmap(bitmapGetMarble(),
        BUF + BALL_RADIUS/2 + m_ballFallX*3*BALL_RADIUS,
        (DEVICE_PIX_Y * 5) + m_ballFallY[i],
        kBitmapUnflipped);
    }

    if (fsm >= kGameFSM_DisplayScores) {
      #define C_OFF (96 + 0)
      // add 16 if adding arrows too
      pd->graphics->drawBitmap(bitmapGetChevron((fc / (TICK_FREQUENCY/2)) % 3), DEVICE_PIX_X - 2*BUF - C_OFF, DEVICE_PIX_Y*5 + BUF, kBitmapFlippedY);
      pd->graphics->drawBitmap(bitmapGetChevron((fc / (TICK_FREQUENCY/2)) % 3), DEVICE_PIX_X - 2*BUF - C_OFF, DEVICE_PIX_Y*6 - C_OFF - BUF, kBitmapUnflipped);
    }

    if (fsm >= kGameFSM_DisplayScores) {
      static uint16_t bufY, bufX, bufDisp = 0;
      if (fsm == kGameFSM_DisplayScores) { // Invalidated once moved to next level, buffer here
        const uint16_t par = IOGetCurrentHolePar();
        const uint16_t score = IOGetCurrentHoleScore();
        bufX = BUF + BALL_RADIUS/2 + m_ballFallX*3*BALL_RADIUS - 2;
        bufY = BUF + BUF/2 + (12 - par - 2)*2*BALL_RADIUS + (DEVICE_PIX_Y * 5);
        bufDisp = (score && score <= par);
      }
      if (bufDisp) {
        pd->graphics->drawBitmap(bitmapGetTick(), bufX, bufY, kBitmapUnflipped);
      }
    }

    if (fsm == kGameFSM_ScoresAnimationB) {
      const int16_t timer = FSMGetHoleScoreTimer();
      const float progress = timer / (float)TIME_SCORE_ANIMATION_B;
      LCDBitmap* par = bitmapGetPar( FSMGetHoleScoreID() );
      LCDBitmap* mask = bitmapGetParMask( progress );

      int16_t yOff = 0;
      if (progress > 0.8f) {
        const float endProg = (progress - 0.8f) * 5.0f;
        const float endProgSmooth = getEasing(kEaseInBack, endProg);
        yOff = DEVICE_PIX_Y * endProgSmooth;
      }

      pd->graphics->setDrawMode(kDrawModeCopy);
      pd->graphics->drawBitmap(par, 0, (DEVICE_PIX_Y * 5) - yOff , kBitmapUnflipped);
      pd->graphics->setBitmapMask(mask, par);
      pd->graphics->drawBitmap(mask, 0, (DEVICE_PIX_Y * 5) - yOff, kBitmapUnflipped);
      pd->graphics->setDrawMode(kDrawModeCopy);
      pd->graphics->setBitmapMask(mask, NULL);

      renderEndBlasts(fc);
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
      pd->graphics->drawBitmap(bitmapGetTutorialCrankRotate(fc / fast), 0, yOff + 20, kBitmapUnflipped);
    }

  } else if (fsm == kGameFSM_TutorialScrollUp) {

    if (pd->system->isCrankDocked()) {
      pd->graphics->drawBitmap(bitmapGetTutorialDPad((fc / slow) % 2), 20, 40 + yOff, kBitmapUnflipped);
    } else {
      pd->graphics->drawBitmap(bitmapGetTutorialCrankRotate(fc / fast), 0, yOff, kBitmapFlippedX);
    }
    pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 2 ? 0 : 2) , HALF_DEVICE_PIX_X + 20, 60 + yOff, kBitmapUnflipped);

  } else if (fsm == kGameFSM_TutorialFireMarble) {

    if (pd->system->isCrankDocked()) {
      const uint8_t f = (fc / slow) % 4;
      const LCDBitmapFlip flip = (f == 2 ? kBitmapFlippedX : kBitmapUnflipped);
      pd->graphics->drawBitmap(bitmapGetTutorialDPad(f % 2 ? 0 : 2), 20, 40 + yOff, flip);
    } else {
      uint8_t f = (fc / fast) % 8;
      if (f > 4) f = 4 - (f - 4);
      pd->graphics->drawBitmap(bitmapGetTutorialCrankAngle(f), 0, 45 + yOff, kBitmapUnflipped);
    }
    pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 4 == 0 ? 1 : 3) , HALF_DEVICE_PIX_X + 20, 60 + yOff, kBitmapUnflipped);

  } else if (fsm == kGameFSM_TutorialGetSpecial) {

    if (pd->system->isCrankDocked()) {
      const uint8_t f = (fc / slow) % 4;
      if (f < 2) pd->graphics->drawBitmap(bitmapGetTutorialDPad((fc / slow) % 2 ? 2 : 0), HALF_DEVICE_PIX_X + 20, 40 + yOff, kBitmapUnflipped);
      else       pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 2 ? 3 : 1) , HALF_DEVICE_PIX_X + 20, 60 + yOff, kBitmapUnflipped);
    } else {
      pd->graphics->drawBitmap(bitmapGetTutorialCrankAngle(0), HALF_DEVICE_PIX_X + 20, yOff, kBitmapUnflipped);
      pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 4 == 0 ? 1 : 3) , HALF_DEVICE_PIX_X + 20 - 50, 5 + 110 + yOff, kBitmapUnflipped);
    }
    if ((fc / slow) % 2) { pd->graphics->drawBitmap(bitmapGetTutorialArrows(0), 0, yOff, kBitmapUnflipped); }

  } else if (fsm == kGameFSM_TutorialGetRequired) {

    if (pd->system->isCrankDocked()) {
      const uint8_t f = (fc / slow) % 4;
      if (f < 2) pd->graphics->drawBitmap(bitmapGetTutorialDPad((fc / slow) % 2 ? 2 : 0), 20, 40 + yOff, kBitmapFlippedX);
      else       pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 2 ? 3 : 1), 20, 60 + yOff, kBitmapUnflipped);
    } else {
      pd->graphics->drawBitmap(bitmapGetTutorialCrankAngle(4), 0, yOff, kBitmapUnflipped);
      pd->graphics->drawBitmap(bitmapGetTutorialButton((fc / slow) % 4 == 0 ? 1 : 3), 20 + 50, 5 + 110 + yOff, kBitmapUnflipped);
    }
    if ((fc / slow) % 2) { pd->graphics->drawBitmap(bitmapGetTutorialArrows(1), HALF_DEVICE_PIX_X, yOff, kBitmapUnflipped); }

  } else if (fsm == kGameFSM_DisplayScores) {

    if (pd->system->isCrankDocked()) {
      pd->graphics->drawBitmap(bitmapGetTutorialDPad((fc / slow) % 2), 40, 40 + yOff, kBitmapFlippedY);
    } else {
      pd->graphics->drawBitmap(bitmapGetTutorialCrankRotate(fc / fast), 20, yOff, kBitmapUnflipped);
    }

  }

}