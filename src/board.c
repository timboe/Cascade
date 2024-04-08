#include "board.h"
#include "easing.h"

struct Peg_t m_pegs[MAX_PEGS];

uint16_t m_nPegs = 0;

uint16_t m_requiredPegsInPlay = 0;

///

int16_t requiredPegsInPlay(void) {
  return m_requiredPegsInPlay;
}

void requiredPegHit(void) { --m_requiredPegsInPlay; }

void initBoard(void) {
  memset(&m_pegs, 0, sizeof(struct Peg_t) * MAX_PEGS);
  for (int i = 0; i < MAX_PEGS; ++i) {
    m_pegs[i].m_id = i;
  }
}

struct Peg_t* pegFromPool(void) {
  return &m_pegs[m_nPegs++];
}

struct Peg_t* getPeg(uint16_t i) {
  return &m_pegs[i];
}

void boardAddWheel(const struct EllipticLoader_t* ellipticLoader) {
  for (int i = 0; i < ellipticLoader->nPegs; ++i) {
    struct Peg_t* p = pegFromPool();
    initPeg(p, ellipticLoader->shape, ellipticLoader->x, ellipticLoader->y, ellipticLoader->angle, ellipticLoader->size);
    setPegMotionSpeed(p, ellipticLoader->speed);
    const float angleOffset = (ellipticLoader->maxAngle / ellipticLoader->nPegs) * i;
    setPegMotionOffset(p, angleOffset);
    setPegMotionEasing(p, ellipticLoader->easing);
    setPegMotionEllipse(p, ellipticLoader->a, ellipticLoader->b, ellipticLoader->useArc);
  }
}

void boardAddLinear(const struct LinearLoader_t* linearLoader) {
  // for (int i = 0; i < linearLoader->nPegs; ++i) {
    
  //   struct Peg_t* p = pegFromPool();
  //   initPeg(p, s, x[0], y[0], angle, size);
  //   setPegMotionSpeed(p, linearLoader->speed);
  //   const float angleOffset = (linearLoader->angleMax / linearLoader->nPegs) * i;
  //   setPegMotionOffset(p, angleOffset);
  //   setPegMotionEasing(p, linearLoader->easing);
  //   for (int j = 1; j < MAX_LINEAR_PATH_SEGMENTS-2; ++j) {
  //     if (!x[j] && !y[j]) {
  //       break;
  //     }
  //     addPegMotionPath(p, x[j], y[j]);
  //   }
  //   pegMotionPathFinalise(p);
  // }
}


struct Peg_t* boardAddStatic(const struct StaticLoader_t* staticLoader) {
  struct Peg_t* p = pegFromPool();
  initPeg(p, staticLoader->shape, staticLoader->x, staticLoader->y, staticLoader->angle, staticLoader->size);
  setPegMotionStatic(p);
  if (staticLoader->type == kPegTypeRequired) {
    ++m_requiredPegsInPlay;
    setPegType(p, kPegTypeRequired);
  }
  return p;
}

void randomiseBoard(void) {
  clearBoard();

  // const int maxStatic = 16;//rand() % 2 ? 16 : 64+32;

  // for (int i = 0; i < maxStatic; ++i) {
  //   const int16_t x = rand() % WFALL_PIX_X;
  //   const int16_t y = (rand() % WFALL_PIX_Y) + TURRET_RADIUS;
  //   const float angle = (M_2PIf / 256.0f) * (rand() % 256);
  //   const enum PegShape_t shape = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
  //   const uint8_t size = rand() % MAX_PEG_SIZE;
  //   const enum PegType_t type = (i < 4 ? kPegTypeRequired : kPegTypeNormal);
  //   boardAddStatic(shape, typr, x, y, angle, size);
  // }

  // if (maxStatic == 16) return;

  // #define PEGS_PER_WHEEL 8
  // #define WHEELS 4
  // for (int wheelStep = 0; wheelStep < WHEELS; ++wheelStep) {
  //   const int16_t x = rand() % WFALL_PIX_X;
  //   const int16_t y = (rand() % WFALL_PIX_Y) + TURRET_RADIUS;
  //   const float a = 64 + rand() % 32;
  //   const float b = 64 + rand() % 32;
  //   const enum PegShape_t s = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
  //   const float speed = 1.0f;//0.1f * ((rand() % 10) + 1); 
  //   struct Peg_t* pegs[32] = {0};
  //   boardAddWheel(pegs, PEGS_PER_WHEEL, M_2PIf, shape, x, y, angle, a, b, speed);
  // }

  // #define PEGS_PER_PATH 8
  // #define PATHS 4
  // for (int pathStep = 0; pathStep < PATHS; ++pathStep) {
  //   int16_t x[MAX_LINEAR_PATH_SEGMENTS] = {0};
  //   int16_t y[MAX_LINEAR_PATH_SEGMENTS] = {0};
  //   x[0] = rand() % WFALL_PIX_X;
  //   y[0] = (rand() % WFALL_PIX_Y) + TURRET_RADIUS;
  //   const uint8_t pathLegs = 1 + rand() % (MAX_LINEAR_PATH_SEGMENTS/2);
  //   for (int leg = 1; leg <= pathLegs; ++leg) {
  //     x[leg] = x[leg-1] - 64 + rand() % 128;
  //     y[leg] = y[leg-1] - 64 + rand() % 128;
  //   }
  //   const enum PegShape_t s = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
  //   const enum EasingFunction_t e = (enum EasingFunction_t) rand() % NEasingFunctions;
  //   const float speed = 0.1f * ((rand() % 10) + 1); 
  //   boardAddPath(PEGS_PER_PATH, M_2PIf, s, e, x, y, speed);    
  // }

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
  m_requiredPegsInPlay = 0;
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

bool popRandom(void) {
  for (int i = 0; i < m_nPegs; ++i) {
    const bool popped = checkPopPeg(&m_pegs[i], 0);
    if (popped) {
      return true;
    }
  }
  return false;
}