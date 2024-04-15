#include "render.h"
#include "bitmap.h"
#include "io.h"

float m_numeralOffset = 0.0;

void commonRenderBackgroundWaterfallWithAnim(const bool locked, const uint8_t offY, uint16_t* timer);

/// ///

void renderSetNumeralOffset(float no) { m_numeralOffset = no; }

void commonRenderBackgroundWaterfallWithAnim(const bool locked, const uint8_t offY, uint16_t* timer) {
  // Draw "previous" waterfall (will become current once gameDoResetPreviousWaterfall is called)
  const uint16_t prevWf = gameGetPreviousWaterfall();
  for (int i = offY; i < (offY+4); ++i) {
    pd->graphics->drawBitmap(bitmapGetWfFg(prevWf, 0, i), 0, WF_DIVISION_PIX_Y * i, kBitmapUnflipped);
  }

  // Animate in new waterfall
  const uint16_t currentWf = IOGetWaterfallForeground(IOGetCurrentLevel(), 0);
  if (currentWf != prevWf) {
    if (locked) {
      pd->graphics->setStencilImage(bitmapGetStencilWipe(*timer), 0);
      for (int i = offY; i < (offY+4); ++i) {
        pd->graphics->drawBitmap(bitmapGetWfFg(currentWf, 0, i), 0, WF_DIVISION_PIX_Y * i, kBitmapUnflipped);
      }
      pd->graphics->setStencilImage(NULL, 0);
      if (++(*timer) == STENCIL_WIPE_N) {
        gameDoResetPreviousWaterfall();
        *timer = 0;
      }
    } else {
      for (int i = offY; i < (offY+4); ++i) {
        pd->graphics->drawBitmap(bitmapGetWfFg(currentWf, 0, i), 0, WF_DIVISION_PIX_Y * i, kBitmapUnflipped);
      }
    }
  }
}

void renderTitlesHeader(const int32_t fc) {
  for (int i = 0; i < 4; ++i) {
    pd->graphics->drawBitmap(bitmapGetWfFg(0, 0, i), 0, WF_DIVISION_PIX_Y * i, kBitmapUnflipped);
  }
  bitmapSetRoobert10();
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  pd->graphics->drawText(VERSION, 8, kUTF8Encoding, 8, DEVICE_PIX_Y-16);
  pd->graphics->drawText("Tim Martin, 2024", 32, kUTF8Encoding, 8, 4);
  pd->graphics->setDrawMode(kDrawModeCopy);
  pd->graphics->drawBitmap(bitmapGetTitleHeaderImage(), 0, 0, kBitmapUnflipped);

  if (IOGetIsPreloading()) {
    const float progress = IOGetPreloadingProgress();
    const uint16_t x2 = (DEVICE_PIX_X/4) + (DEVICE_PIX_X/2)*progress; 
    pd->graphics->setLineCapStyle(kLineCapStyleRound);
    pd->graphics->drawLine(DEVICE_PIX_X/4, (4*DEVICE_PIX_Y)/5, (3*DEVICE_PIX_X)/4, (4*DEVICE_PIX_Y)/5, TITLETEXT_HEIGHT, kColorBlack);
    pd->graphics->drawLine(DEVICE_PIX_X/4, (4*DEVICE_PIX_Y)/5, x2, (4*DEVICE_PIX_Y)/5, TITLETEXT_HEIGHT/2, kColorWhite);
  } else {
    if (pd->system->isCrankDocked() && (fc / (TICK_FREQUENCY / 2)) % 2) { 
      pd->graphics->drawBitmap(bitmapGetUseTheCrank(), DEVICE_PIX_X - 88, DEVICE_PIX_Y - 51 - 16, kBitmapUnflipped);
    }
  }
}

void renderTitlesPlayerSelect(const bool locked) {
  static uint16_t newWaterfallTimer = 0;
  if (!locked) newWaterfallTimer = 0;
  commonRenderBackgroundWaterfallWithAnim(locked, 4, &newWaterfallTimer);
  const float parallax = gameGetParalaxFactorNear(true) - gameGetParalaxFactorNearForY(true, DEVICE_PIX_Y); // Hard = true

  uint8_t digit[3];
  digit[1] = IOGetCurrentPlayer();
  digit[0] = digit[1] == 0 ? MAX_PLAYERS-1 : digit[1]-1;
  digit[2] = (digit[1] + 1) % MAX_PLAYERS;
  for (int i = 0; i < 3; ++i) { digit[i]++; }
  const float offY = (NUMERAL_PIX_Y / 2) * m_numeralOffset;

  pd->graphics->drawBitmap(bitmapGetTitlePlayer(), NUMERAL_BUF - TITLETEXT_HEIGHT, DEVICE_PIX_Y + NUMERAL_BUF + parallax, kBitmapUnflipped);
  if (!locked) {
    pd->graphics->drawBitmap(bitmapGetNumeral(digit[1]),
      NUMERAL_BUF, DEVICE_PIX_Y + NUMERAL_BUF + parallax, kBitmapUnflipped);
  } else {
    pd->graphics->setScreenClipRect(NUMERAL_BUF, NUMERAL_BUF, NUMERAL_PIX_X, NUMERAL_PIX_Y);
    pd->graphics->drawBitmap(bitmapGetNumeral(digit[0]),
      NUMERAL_BUF, DEVICE_PIX_Y + NUMERAL_BUF - NUMERAL_PIX_Y + offY, kBitmapUnflipped);
    pd->graphics->drawBitmap(bitmapGetNumeral(digit[1]),
      NUMERAL_BUF, DEVICE_PIX_Y + NUMERAL_BUF + offY, kBitmapUnflipped);
    pd->graphics->drawBitmap(bitmapGetNumeral(digit[2]),
      NUMERAL_BUF, DEVICE_PIX_Y + NUMERAL_BUF + NUMERAL_PIX_Y + offY, kBitmapUnflipped);
    pd->graphics->clearClipRect();
  }
}

void renderTitlesLevelSelect(const bool locked) {
  static uint16_t newWaterfallTimer = 0;
  if (!locked) newWaterfallTimer = 0;
  commonRenderBackgroundWaterfallWithAnim(locked, 8, &newWaterfallTimer);
  const float parallax = gameGetParalaxFactorNear(true) - gameGetParalaxFactorNearForY(true, DEVICE_PIX_Y*2); // Hard = true

  uint8_t digit0[3];
  uint8_t digit1[3];
  // +1 is because we display levels 0-98 as 1-99
  digit0[0] = (IOGetPreviousLevel() + 1) / 10;
  digit1[0] = (IOGetPreviousLevel() + 1) % 10;
  digit0[1] = (IOGetCurrentLevel() + 1) / 10;
  digit1[1] = (IOGetCurrentLevel() + 1) % 10;
  digit0[2] = (IOGetNextLevel() + 1) / 10;
  digit1[2] = (IOGetNextLevel() + 1) % 10;
  float offY = (NUMERAL_PIX_Y / 2) * m_numeralOffset;

  pd->graphics->drawBitmap(bitmapGetTitleLevel(), DEVICE_PIX_X - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + parallax, kBitmapUnflipped);
  pd->graphics->drawBitmap(bitmapGetTitleLevelStats(), DEVICE_PIX_X - (2*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + NUMERAL_PIX_Y + parallax, kBitmapUnflipped);
  if (!locked) {
    pd->graphics->drawBitmap(bitmapGetNumeral(digit0[1]),
      DEVICE_PIX_X - (2*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + parallax, kBitmapUnflipped);
    pd->graphics->drawBitmap(bitmapGetNumeral(digit1[1]),
      DEVICE_PIX_X - (1*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + parallax, kBitmapUnflipped);
  } else {
    pd->graphics->setScreenClipRect(DEVICE_PIX_X - (NUMERAL_PIX_X*2) - NUMERAL_BUF, NUMERAL_BUF, NUMERAL_PIX_X*2, NUMERAL_PIX_Y);
    pd->graphics->drawBitmap(bitmapGetNumeral(digit1[0]),
      DEVICE_PIX_X - (1*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF - NUMERAL_PIX_Y + offY, kBitmapUnflipped);
    pd->graphics->drawBitmap(bitmapGetNumeral(digit1[1]),
      DEVICE_PIX_X - (1*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + offY, kBitmapUnflipped);
    pd->graphics->drawBitmap(bitmapGetNumeral(digit1[2]),
      DEVICE_PIX_X - (1*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + NUMERAL_PIX_Y + offY, kBitmapUnflipped);
    if (m_numeralOffset < 0 && digit0[1] == digit0[2]) { offY = 0.0f; }
    if (m_numeralOffset > 0 && digit0[0] == digit0[1]) { offY = 0.0f; }
    pd->graphics->drawBitmap(bitmapGetNumeral(digit0[0]),
      DEVICE_PIX_X - (2*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF - NUMERAL_PIX_Y + offY, kBitmapUnflipped);
    pd->graphics->drawBitmap(bitmapGetNumeral(digit0[1]),
      DEVICE_PIX_X - (2*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + offY, kBitmapUnflipped);
    pd->graphics->drawBitmap(bitmapGetNumeral(digit0[2]),
      DEVICE_PIX_X - (2*NUMERAL_PIX_X) - NUMERAL_BUF, (DEVICE_PIX_Y*2) + NUMERAL_BUF + NUMERAL_PIX_Y + offY, kBitmapUnflipped);
    pd->graphics->clearClipRect();
  }
}

void renderTitlesHoleSelect(const bool locked) {
 const uint16_t currentWf = IOGetWaterfallForeground(IOGetCurrentLevel(), 0);
  for (int i = 12; i < 16; ++i) {
    pd->graphics->drawBitmap(bitmapGetWfFg(currentWf, 0, i), 0, WF_DIVISION_PIX_Y * i, kBitmapUnflipped);
  }
  const float parallax = gameGetParalaxFactorNear(true) - gameGetParalaxFactorNearForY(true, DEVICE_PIX_Y*3); // Hard = true

  static uint16_t offset = 0;
  LCDBitmap* bm = bitmapGetLevelPreview(IOGetCurrentLevel(), IOGetCurrentHole(), offset);
  offset += 2;
  if (bm) { pd->graphics->drawBitmap(bm, DEVICE_PIX_X - 200, DEVICE_PIX_Y*3 + parallax, kBitmapUnflipped); }

  pd->graphics->drawBitmap(bitmapGetDither(), 0, (WF_DIVISION_PIX_Y * 15), kBitmapUnflipped);
  pd->graphics->drawBitmap(bitmapGetTitleHoleName(), HALF_DEVICE_PIX_X, (DEVICE_PIX_Y * 3) + NUMERAL_BUF - TITLETEXT_HEIGHT + parallax, kBitmapUnflipped);
  pd->graphics->drawBitmap(bitmapGetTitleHoleAuthor(), HALF_DEVICE_PIX_X, (DEVICE_PIX_Y * 4) - NUMERAL_BUF + parallax, kBitmapUnflipped);

  uint8_t digit[3];
  LCDBitmap* digitBm[3];
  // +1 is because we display levels 0-98 as 1-99
  digit[0] = (IOGetPreviousHole() + 1);
  digit[1] = (IOGetCurrentHole() + 1);
  digit[2] = (IOGetNextHole() + 1);
  digitBm[0] = bitmapGetNumeral(digit[0]);
  digitBm[1] = bitmapGetNumeral(digit[1]);
  digitBm[2] = bitmapGetNumeral(digit[2]);
  if (IOGetCurrentLevel() == 0) {
    for (int i = 0; i < 3; ++i) {
      if (digit[i] == 1) { // Remmber we have already +1 for display
        digitBm[i] = bitmapGetTitleHoleTutorial();
      }
    }
  } 
  const float offY = (NUMERAL_PIX_Y / 2) * m_numeralOffset;

  pd->graphics->drawBitmap(bitmapGetTitleHole(), NUMERAL_BUF - TITLETEXT_HEIGHT, (DEVICE_PIX_Y*3) + NUMERAL_BUF + parallax, kBitmapUnflipped);
  pd->graphics->drawBitmap(bitmapGetTitleHoleStatsA(), NUMERAL_BUF, (DEVICE_PIX_Y*3) + NUMERAL_BUF - TITLETEXT_HEIGHT + parallax, kBitmapUnflipped);
  // Note this one is extra wide
  pd->graphics->drawBitmap(bitmapGetTitleHoleStatsB(), 0, (DEVICE_PIX_Y*3) + NUMERAL_BUF + NUMERAL_PIX_Y + parallax, kBitmapUnflipped);
  if (!locked) {
    pd->graphics->drawBitmap(digitBm[1],
      NUMERAL_BUF, (DEVICE_PIX_Y*3) + NUMERAL_BUF + parallax, kBitmapUnflipped);
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

void renderTitlesWfPond(void) {
  const float parallax = gameGetParalaxFactorNear(false) - gameGetParalaxFactorNearForY(false, DEVICE_PIX_Y*4); // Hard = false
  pd->graphics->drawBitmap(bitmapGetWfPond(), 0, DEVICE_PIX_Y*4 + parallax, kBitmapUnflipped);
}

void renderTitlesTransitionLevelSplash(void) {
  const float parallax = gameGetParalaxFactorNear(true) - gameGetParalaxFactorNearForY(true, DEVICE_PIX_Y*5); // Hard = true
  pd->graphics->drawBitmap(bitmapGetLevelTitle(), 0, DEVICE_PIX_Y*5 + parallax, kBitmapUnflipped);
}