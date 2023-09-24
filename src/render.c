#include <math.h>
#include "render.h"
#include "sprite.h"
#include "io.h"
#include "input.h"
#include "ui.h"

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



void renderGameWindow(int32_t _fc) {

  const int32_t so = getScrollOffset();
  const uint32_t start = so / WF_DIVISION_PIX_Y;

  pd->system->logToConsole("so is %i, rendering from %i", so, start);

  for (uint32_t i = start; i < start+5; ++i) {
    if (i >= WFSHEET_SIZE_Y) break;
    pd->graphics->drawBitmap(getBitmapWf(0,i),
      0,
      WF_DIVISION_PIX_Y * i,
      kBitmapUnflipped);
  }

  pd->graphics->drawBitmap(getBitmapTurretBarrel(), DEVICE_PIX_X/2 - 32, -32, kBitmapUnflipped);
  pd->graphics->drawBitmap(getBitmapTurretBody(), DEVICE_PIX_X/2 - 32, -32, kBitmapUnflipped);



}