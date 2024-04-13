#include "board.h"
#include "easing.h"

struct Peg_t m_pegs[MAX_PEGS];

uint16_t m_nPegs = 0;

uint16_t m_requiredPegsInPlay = 0;

enum PegSpecial_t m_special = kPegSpecialNotSpecial;


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

  // TEMP
  int i = rand() % 3;
  if (i == 0) m_special = kPegSpecialAim;
  else if (i == 1) m_special = kPegSpecialSecondTry;
  else m_special = kPegSpecialBounce;

  m_special = kPegSpecialPenetrate;

}

struct Peg_t* pegFromPool(void) { return &m_pegs[m_nPegs++]; }

struct Peg_t* getPeg(const uint16_t i) { return &m_pegs[i]; }

uint16_t getNPegs(void) { return m_nPegs; }

void specialBurst(void) {
  const cpVect pos = cpBodyGetPosition(getBall(0));
  for (int i = 0; i < m_nPegs; ++i) {
    struct Peg_t* p = getPeg(i);
    if (p->m_state == kPegStateActive) {
      const cpVect pegPos = cpv(p->m_x, p->m_y);
      if (cpvdist(pos, pegPos) < SPECIAL_BURST_RADIUS) {
        hitPeg(p);
      }
    }
  }
}


void boardAddWheel(const struct EllipticLoader_t* ellipticLoader) {
  for (int i = 0; i < ellipticLoader->nPegs; ++i) {
    struct Peg_t* p = pegFromPool();
    // -1 in the overrides is because 0 here represents Inherit, and all othher enums are incremented by 1
    enum PegShape_t shape = ellipticLoader->shape;
    if (ellipticLoader->shapeOverride[i]) { shape = (enum PegShape_t) ellipticLoader->shapeOverride[i] - 1; }
    uint8_t size = ellipticLoader->size;
    if (ellipticLoader->sizeOverride[i]) { size = ellipticLoader->sizeOverride[i] - 1; }
    initPeg(p, shape, ellipticLoader->x, ellipticLoader->y, ellipticLoader->angle, size);
    setPegMotionSpeed(p, ellipticLoader->speed);
    const float angleOffset = (ellipticLoader->maxAngle / ellipticLoader->nPegs) * i;
    setPegMotionOffset(p, angleOffset);
    setPegMotionEasing(p, ellipticLoader->easing);
    setPegMotionDoArcAngle(p, ellipticLoader->useArc);
    setPegMotionEllipse(p, ellipticLoader->a, ellipticLoader->b);
    if (ellipticLoader->types[i] == kPegTypeRequired) {
      ++m_requiredPegsInPlay;
      setPegType(p, kPegTypeRequired);
    }
  }
}

void boardAddLinear(const struct LinearLoader_t* linearLoader) {
  for (int i = 0; i < linearLoader->nPegs; ++i) {
    struct Peg_t* p = pegFromPool();
    enum PegShape_t shape = linearLoader->shape;
    if (linearLoader->shapeOverride[i]) { shape = (enum PegShape_t) linearLoader->shapeOverride[i] - 1; }
    uint8_t size = linearLoader->size;
    if (linearLoader->sizeOverride[i]) { size = linearLoader->sizeOverride[i] - 1; }
    initPeg(p, shape, linearLoader->x, linearLoader->y, linearLoader->angle, size);
    const float angleOffset = (linearLoader->maxAngle / linearLoader->nPegs) * i;
    setPegMotionSpeed(p, linearLoader->speed);
    setPegMotionOffset(p, angleOffset);
    setPegMotionEasing(p, linearLoader->easing);
    setPegMotionDoArcAngle(p, linearLoader->useArc);
    if (linearLoader->types[i] == kPegTypeRequired) {
      ++m_requiredPegsInPlay;
      setPegType(p, kPegTypeRequired);
    }
    for (int j = 0; j < linearLoader->nLines; ++j) {
       addPegMotionPath(p, linearLoader->pathX[j], linearLoader->pathY[j]);
    }
    pegMotionPathFinalise(p);
  }
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

  const int maxStatic = 64;//rand() % 2 ? 16 : 64+32;

  for (int i = 0; i < maxStatic; ++i) {
    struct StaticLoader_t staticLoader;
    staticLoader.x = rand() % WFALL_PIX_X;
    staticLoader.y = (rand() % WFALL_PIX_Y) + TURRET_RADIUS;
    staticLoader.angle = (M_2PIf / 256.0f) * (rand() % 256);
    staticLoader.shape = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
    staticLoader.size = rand() % MAX_PEG_SIZE;
    staticLoader.type = (i < 4 ? kPegTypeRequired : kPegTypeNormal);
    boardAddStatic(&staticLoader);
  }

  return;

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

enum PegSpecial_t getCurrentSpecial(void) {
  return m_special;
}

void clearSpecial(void) {
  m_special = kPegSpecialNotSpecial;
}