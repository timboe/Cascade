#include "board.h"
#include "easing.h"

struct Peg_t m_pegs[MAX_PEGS];

uint16_t m_nPegs = 0;

///

void initBoard(void) {
  memset(&m_pegs, 0, sizeof(struct Peg_t) * MAX_PEGS);
  for (int i = 0; i < MAX_PEGS; ++i) {
    m_pegs[i].m_id = i;
  }
}

struct Peg_t* pegFromPool(void) {
  return &m_pegs[m_nPegs++];
}

void boardAddWheel(const uint8_t n, const float angleMax, const enum PegShape_t s, const uint16_t x, const uint16_t y, const uint16_t a, const uint16_t b, const float speed) {
  for (int i = 0; i < n; ++i) {
    const float angle = (angleMax / n) * i;
    struct Peg_t* p = pegFromPool();
    initPeg(p, s, x, y, angle);
    setPegMotionSpeed(p, speed);
    setPegMotionOffset(p, angle);
    setPegMotionEllipse(p, a, b);
  }
}

void boardAddPath(const uint8_t n, const float angleMax, const enum PegShape_t s, const enum EasingFunction_t e, int16_t x[], int16_t y[], const float speed) {
  for (int i = 0; i < n; ++i) {
    const float angle = (angleMax / n) * i;
    struct Peg_t* p = pegFromPool();
    initPeg(p, s, x[0], y[0], angle);
    setPegMotionSpeed(p, speed);
    setPegMotionOffset(p, angle);
    setPegMotionEasing(p, e);
    for (int j = 1; j < MAX_PEG_PATHS; ++j) {
      if (!x[j] && !y[j]) {
        break;
      }
      addPegMotionPath(p, x[j], y[j]);
    }
    pegMotionPathFinalise(p);
  }
}

void randomiseBoard(void) {
  clearBoard();

  for (int i = 0; i < 16; ++i) {
    const uint16_t x = rand() % WFALL_PIX_X;
    const uint16_t y = (rand() % WFALL_PIX_Y) + (2*UI_OFFSET_TOP);
    const float a = (M_2PIf / 256.0f) * (rand() % 256);
    const enum PegShape_t s = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
    struct Peg_t* p = pegFromPool();
    initPeg(p, s, x, y, a);
  }

  #define PEGS_PER_WHEEL 8
  #define WHEELS 4
  for (int wheelStep = 0; wheelStep < WHEELS; ++wheelStep) {
    const uint16_t x = rand() % WFALL_PIX_X;
    const uint16_t y = (rand() % WFALL_PIX_Y) + (2*UI_OFFSET_TOP);
    const float a = 64 + rand() % 32;
    const float b = 64 + rand() % 32;
    const enum PegShape_t s = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
    const float speed = 0.1f * ((rand() % 10) + 1); 
    boardAddWheel(PEGS_PER_WHEEL, M_2PIf, s, x, y, a, b, speed);
  }

  #define PEGS_PER_PATH 8
  #define PATHS 4
  for (int pathStep = 0; pathStep < PATHS; ++pathStep) {
    int16_t x[MAX_PEG_PATHS] = {0};
    int16_t y[MAX_PEG_PATHS] = {0};
    x[0] = rand() % WFALL_PIX_X;
    y[0] = (rand() % WFALL_PIX_Y) + (2*UI_OFFSET_TOP);
    const uint8_t pathLegs = 1 + rand() % (MAX_PEG_PATHS-2);
    for (int leg = 1; leg <= pathLegs; ++leg) {
      x[leg] = x[leg-1] - 64 + rand() % 128;
      y[leg] = y[leg-1] - 64 + rand() % 128;
    }
    const enum PegShape_t s = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
    const enum EasingFunction_t e = (enum EasingFunction_t) rand() % NEasingFunctions;
    const float speed = 0.1f * ((rand() % 10) + 1); 
    boardAddPath(PEGS_PER_PATH, M_2PIf, s, e, x, y, speed);    
  }

}

void updateBoard(void) {
  for (int i = 0; i < m_nPegs; ++i) {
    updatePeg(&m_pegs[i]);
  }
}

void clearBoard(void) {
  for (int i = 0; i < m_nPegs; ++i) {
    clearPeg(&m_pegs[i]);
  }
  m_nPegs = 0;
}

void renderBoard(void) {
  for (int i = 0; i < m_nPegs; ++i) {
    pd->graphics->drawBitmap(m_pegs[i].m_bitmap, m_pegs[i].m_xBitmap, m_pegs[i].m_yBitmap, kBitmapUnflipped);
    // DEBUG
    if (m_pegs[i].m_motion == kPegMotionEllipse) {
      pd->graphics->fillEllipse(m_pegs[i].m_pathX[0]-3, m_pegs[i].m_pathY[0]-3, 6, 6, 0.0f, 360.0f, kColorWhite);
      pd->graphics->fillEllipse(m_pegs[i].m_pathX[0]-2, m_pegs[i].m_pathY[0]-2, 4, 4, 0.0f, 360.0f, kColorBlack);
    } else if (m_pegs[i].m_motion == kPegMotionPath) {
      for (int j = 1; j < m_pegs[i].m_pathSteps; ++j) {
        pd->graphics->drawLine(m_pegs[i].m_pathX[j], m_pegs[i].m_pathY[j], m_pegs[i].m_pathX[j-1], m_pegs[i].m_pathY[j-1], 2, kColorWhite);
      }
    }
  }
}