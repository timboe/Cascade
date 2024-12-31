#include "render.h"
#include "bitmap.h"
#include "io.h"

float m_numeralOffset = 0.0;

/// ///

void renderSetNumeralOffset(float no) { m_numeralOffset = no; }

void renderTitlesHeader(const int32_t fc) {
  bitmapSetRoobert10();
  const uint16_t vX = DEVICE_PIX_X-32, vY = 4, nameX = 8, nameY = 4;
  pd->graphics->setDrawMode(kDrawModeFillWhite);
  for (int8_t xM = -2; xM < 4; xM += 2) {
    for (int8_t yM = -2; yM < 4; yM += 2) {
      pd->graphics->drawText(VERSION, 8, kUTF8Encoding, vX + xM, vY + yM);
      pd->graphics->drawText("Tim Martin, 2025", 32, kUTF8Encoding, nameX + xM, nameY + yM);
    }
  }
  pd->graphics->setDrawMode(kDrawModeFillBlack);
  pd->graphics->drawText(VERSION, 8, kUTF8Encoding, vX, vY);
  pd->graphics->drawText("Tim Martin, 2024", 32, kUTF8Encoding, nameX, nameY);
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

void renderTitlesPlayerSelect(const bool locked, const int32_t fc) {
  const float parallax = gameGetParalaxFactorNear(true) - gameGetParalaxFactorNearForY(true, DEVICE_PIX_Y); // Hard = true

  bitmapDoUpdateScoreCard();
  pd->graphics->drawBitmap(bitmapGetTitleScoreCard(), HALF_DEVICE_PIX_X - 5*BUF/2, DEVICE_PIX_Y + 3*BUF/4, kBitmapUnflipped);

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

  if (locked && (fc / TICK_FREQUENCY) % 2) {
    pd->graphics->drawBitmap(bitmapGetFwBkwIcon(1), HALF_DEVICE_PIX_X - FW_BKW_WIDTH/2 - 4, DEVICE_PIX_Y*2 - FW_BKW_HEIGHT - FW_BKW_BUF, kBitmapUnflipped);
  }
}

void renderTitlesLevelSelect(const bool locked, const int32_t fc) {
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

  if (locked && (fc / TICK_FREQUENCY) % 2) {
    pd->graphics->drawBitmap(bitmapGetFwBkwIcon(0), 104/2 - FW_BKW_WIDTH/2, DEVICE_PIX_Y*2 + FW_BKW_BUF, kBitmapUnflipped);
    pd->graphics->drawBitmap(bitmapGetFwBkwIcon(1), 104/2 - FW_BKW_WIDTH/2, DEVICE_PIX_Y*3 - FW_BKW_BUF - FW_BKW_HEIGHT, kBitmapUnflipped);
  }
}

void renderTitlesHoleSelect(const bool locked, const int32_t fc) {
  const float parallax = gameGetParalaxFactorNear(true) - gameGetParalaxFactorNearForY(true, DEVICE_PIX_Y*3); // Hard = true
  static float offset = 0;
  LCDBitmap* bm = bitmapGetLevelPreview(IOGetCurrentLevel(), IOGetCurrentHole(), offset);
  offset += WF_VELOCITY;
  if (bm) { pd->graphics->drawBitmap(bm, DEVICE_PIX_X - 200, DEVICE_PIX_Y*3 + parallax, kBitmapUnflipped); }

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

  if (locked && (fc / TICK_FREQUENCY) % 2) {
    pd->graphics->drawBitmap(bitmapGetFwBkwIcon(0), HALF_DEVICE_PIX_X - FW_BKW_WIDTH/2 - 8, DEVICE_PIX_Y*3 + FW_BKW_BUF, kBitmapUnflipped);
    pd->graphics->drawBitmap(bitmapGetFwBkwIcon(1), HALF_DEVICE_PIX_X - FW_BKW_WIDTH/2 - 8, DEVICE_PIX_Y*4 - FW_BKW_BUF - FW_BKW_HEIGHT, kBitmapUnflipped);
  }
}

void renderTitlesWfPond(const int32_t fc) {
  const float parallax = gameGetParalaxFactorNear(true) - gameGetParalaxFactorNearForY(true, DEVICE_PIX_Y*3 - 16); // Hard = false
  pd->graphics->drawBitmap(bitmapGetWfPond(0, fc), 0, DEVICE_PIX_Y*4 + parallax, kBitmapUnflipped);
  for (int i = 0; i < POND_WATER_TILES; ++i) {
    const int16_t offset = (i * POND_WATER_HEIGHT);
    pd->graphics->drawBitmap(bitmapGetWfPond(i, fc), 0, DEVICE_PIX_Y*4 + parallax + offset, kBitmapUnflipped);
  }
}

void renderTitlesTransitionLevelSplash(void) {
  const float parallax = gameGetParalaxFactorNear(true) - gameGetParalaxFactorNearForY(true, DEVICE_PIX_Y*5); // Hard = true
  pd->graphics->drawBitmap(bitmapGetLevelTitle(), 0, DEVICE_PIX_Y*5 + parallax, kBitmapUnflipped);
}