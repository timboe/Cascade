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
    const uint8_t size = 0;
    const float angle = (angleMax / n) * i;
    struct Peg_t* p = pegFromPool();
    initPeg(p, s, x, y, angle, size);
    setPegMotionSpeed(p, speed);
    setPegMotionOffset(p, angle);
    setPegMotionEllipse(p, a, b);
  }
}

void boardAddPath(const uint8_t n, const float angleMax, const enum PegShape_t s, const enum EasingFunction_t e, int16_t x[], int16_t y[], const float speed) {
  for (int i = 0; i < n; ++i) {
    const uint8_t size = 0;
    const float angle = (angleMax / n) * i;
    struct Peg_t* p = pegFromPool();
    initPeg(p, s, x[0], y[0], angle, size);
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

  const int maxStatic = rand() % 2 ? 16 : 64;

  for (int i = 0; i < maxStatic; ++i) {
    const int16_t x = rand() % WFALL_PIX_X;
    const int16_t y = (rand() % WFALL_PIX_Y) + TURRET_RADIUS;
    const float angle = (M_2PIf / 256.0f) * (rand() % 256);
    const enum PegShape_t s = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
    const uint8_t size = rand() % MAX_PEG_SIZE;
    struct Peg_t* p = pegFromPool();
    initPeg(p, s, x, y, angle, size);
  }

  if (maxStatic == 64) return;

  #define PEGS_PER_WHEEL 8
  #define WHEELS 4
  for (int wheelStep = 0; wheelStep < WHEELS; ++wheelStep) {
    const int16_t x = rand() % WFALL_PIX_X;
    const int16_t y = (rand() % WFALL_PIX_Y) + TURRET_RADIUS;
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
    y[0] = (rand() % WFALL_PIX_Y) + TURRET_RADIUS;
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
    renderPeg(&m_pegs[i]);
  }
}

void popBoard(float y) {
  for (int i = 0; i < m_nPegs; ++i) {
    checkPopPeg(&m_pegs[i], y);
  }
}