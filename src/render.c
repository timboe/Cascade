#include <math.h>
#include "render.h"
#include "sprite.h"
#include "io.h"
#include "input.h"
#include "ui.h"

float m_trauma = 0.0f, m_decay = 0.0f;

/// ///

void addTrauma(float _amount) {
  m_trauma += _amount;
  m_trauma *= -1;
  m_decay = _amount;
}

void render() {

  if (true && m_decay > 0.0f) {
    m_decay -= TRAUMA_DECAY;
    m_trauma += (m_trauma > 0 ? -m_decay : m_decay);
    pd->display->setOffset(0, m_trauma * TRAUMA_AMPLIFICATION);
  } else {
    pd->display->setOffset(0, 0);
  }

  const float offX = 0.0f;
  const float offY = 0.0f;

  pd->graphics->setDrawMode(kDrawModeCopy);
  pd->graphics->setDrawOffset(offX, offY);

  // Draw FPS indicator (dbg only)
  #ifdef DEV
  if (ALWAYS_FPS) {
    pd->system->drawFPS(0, 0);
  }
  #endif
}


