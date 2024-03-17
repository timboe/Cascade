#include <math.h>
#include "render.h"
#include "bitmap.h"
#include "io.h"
#include "input.h"
#include "ui.h"
#include "physics.h"

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

  // Draw FPS indicator (dbg only)
  #ifdef DEV
  if (ALWAYS_FPS) {
    pd->system->drawFPS(0, 0);
  }
  #endif

  switch (getGameMode()) {
    case kTitles: return renderTitles(_fc);
    case kGameWindow: return renderGameWindow(_fc);
    default: return;
  }

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


#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

void renderGameWindow(int32_t _fc) {

  const int32_t so = ((int32_t) getScrollOffset()) - UI_OFFSET_TOP;
  const uint32_t start = MAX(0, so / WF_DIVISION_PIX_Y);

  // pd->system->logToConsole("so is %i, rendering from %i to %i", so, start, start+5);

  uint8_t wf = 0;
  static uint8_t wfOffC = 0;
  int8_t wfOff = 15 - (wfOffC % 16);
    pd->system->logToConsole("off %i", wfOff);
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


  pd->graphics->drawBitmap(getBitmapTurretBody(), DEVICE_PIX_X/2 - 32, 0, kBitmapUnflipped);
  pd->graphics->drawBitmap(getBitmapTurretBarrel(), DEVICE_PIX_X/2 - 32, 0, kBitmapUnflipped);

  {
    cpBody* ball = getBall();
    const cpVect center = cpBodyGetPosition(ball);
    const float angle = (cpBodyGetAngle(ball) / (M_PIf * 2.0f)) * 180.0f;
    const float x = center.x - BALL_RADIUS;
    const float y = center.y - BALL_RADIUS;
    pd->graphics->drawBitmap(getBitmapBall(angle), x, y, kBitmapUnflipped);
    if (y < PHYSWALL_PIX_Y) setScrollOffset(y - HALF_DEVICE_PIX_Y);
  }

  for (uint32_t i = 0; i < N_OBST; ++i) {
    cpBody* obst = getObst(i);
    const cpVect center = cpBodyGetPosition(obst);
    const float y = center.y - BALL_RADIUS;

    int off = y - getScrollOffset();
    if (off < 0 || off > DEVICE_PIX_Y) continue;

    const float angle = (cpBodyGetAngle(obst) / (M_PIf * 2.0f)) * 180.0f;
    const float x = center.x - BALL_RADIUS;

    pd->graphics->drawBitmap(getBitmapBall(angle), x, y, kBitmapUnflipped);
  }



}