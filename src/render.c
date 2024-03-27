#include <math.h>
#include "render.h"
#include "bitmap.h"
#include "io.h"
#include "input.h"
#include "ui.h"
#include "physics.h"
#include "board.h"

float m_trauma = 0.0f, m_decay = 0.0f;

void renderTitles(int32_t _fc);

void renderGameWindow(int32_t _fc);

/// ///

void addTrauma(float _amount) {
  m_trauma += _amount;
  m_trauma *= -1;
  m_decay = _amount;
}

void render(int32_t _fc) {

  if (true && m_decay > 0.0f) {
    m_decay -= TRAUMA_DECAY;
    m_trauma += (m_trauma > 0 ? -m_decay : m_decay);
    pd->display->setOffset(0, m_trauma * TRAUMA_AMPLIFICATION);
  } else {
    pd->display->setOffset(0, 0);
  }

  const float offX = 0.0f;
  const float offY = -getScrollOffset();

  pd->graphics->setDrawMode(kDrawModeCopy);
  pd->graphics->setDrawOffset(offX, offY);

  pd->graphics->clear(kColorWhite);

  switch (getGameMode()) {
    case kTitles: renderTitles(_fc); break;
    case kGameWindow: renderGameWindow(_fc); break;
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


void renderBall(void) {
  cpBody* ball = getBall();
  const cpVect center = cpBodyGetPosition(ball);
  const float angle = (cpBodyGetAngle(ball) / (M_PIf * 2.0f)) * 180.0f;
  const float x = center.x - BALL_RADIUS;
  const float y = center.y - BALL_RADIUS;
  pd->graphics->drawBitmap(getBitmapBall(angle), x, y, kBitmapUnflipped);
  if (y < PHYSWALL_PIX_Y) setScrollOffset(y - HALF_DEVICE_PIX_Y); // TODO - smooth this and move this call
}

void renderTurret(void) {
  if (getScrollOffset() >= 32) {
    return;
  }
  pd->graphics->drawBitmap(getBitmapTurretBody(), DEVICE_PIX_X/2 - 32, 0, kBitmapUnflipped);
  pd->graphics->drawBitmap(getBitmapTurretBarrel(), DEVICE_PIX_X/2 - 32, 0, kBitmapUnflipped);
}

void renderPath(void) {
  if (ballInPlay()) {
    return;
  }

  cpBody* ball = getBall();  
  setDoMotionPath(true);
  launchBall();  
  for(int i=0; i<64; i++){
    updateSpace();
    if(i%8==0){
      const cpVect center = cpBodyGetPosition(ball);
      pd->graphics->fillEllipse(center.x-3, center.y-3, 6, 6, 0.0f, 360.0f, kColorWhite);
      pd->graphics->fillEllipse(center.x-2, center.y-2, 4, 4, 0.0f, 360.0f, kColorBlack);
    }
  }
  resetBall();
  setDoMotionPath(false);
}

void renderBackground(void) {
  const int32_t so = ((int32_t) getScrollOffset()) - UI_OFFSET_TOP;
  const uint32_t start = MAX(0, so / WF_DIVISION_PIX_Y);
  // pd->system->logToConsole("so is %i, rendering from %i to %i", so, start, start+5);
  uint8_t wf = 0;
  static uint8_t wfOffC = 0;
  int8_t wfOff = 15 - (wfOffC % 16);
  //pd->system->logToConsole("off %i", wfOff);
  ++wfOffC;
  // NOTE: Need to draw one extra background due to animation
  for (uint32_t i = start; i < start+6; ++i) {
    if (i >= WFSHEET_SIZE_Y) break;
    pd->graphics->drawBitmap(getBitmapWfBg(wf, 0,i), WF_BG_OFFSET[wf], UI_OFFSET_TOP + (WF_DIVISION_PIX_Y * i) - wfOff, kBitmapUnflipped);
  }
  for (uint32_t i = start; i < start+5; ++i) {
    if (i >= WFSHEET_SIZE_Y) break;
    pd->graphics->drawBitmap(getBitmapWfFg(wf, 0,i), 0,                UI_OFFSET_TOP + (WF_DIVISION_PIX_Y * i), kBitmapUnflipped);
  }
}

void renderGameWindow(int32_t _fc) {

  // DRAW BACKGROUND
  renderBackground();

  // DRAW TURRET & TOP DECORATION
  renderTurret();

  // DRAW BALL
  renderBall();

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

  // DRAW PATH
  renderPath();


}