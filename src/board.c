#include "board.h"
#include "easing.h"
#include "util.h"
#include "io.h"

struct Peg_t m_pegs[MAX_PEGS];

uint16_t m_nPegs = 0;

uint16_t m_pegsHit;

uint16_t m_requiredPegsInPlay = 0;

float m_lastBurstLevel = 0.0f;

enum PegSpecial_t m_special = kPegSpecialNotSpecial;
enum PegSpecial_t m_specialToActivate = kPegSpecialNotSpecial;

uint8_t m_specialCounter = 0;

/// ///

uint16_t boardGetPegsHit(void) { return m_pegsHit;  }

void boardResetPegsHit(void) { m_pegsHit = 0; }

void boardDoPegHit(void) { ++m_pegsHit; }

float boardGetLastBurstLevel(void) { return m_lastBurstLevel; }

uint16_t boardGetRequiredPegsInPlay(void) { return m_requiredPegsInPlay; }

uint16_t boardGetRequiredPegsHit(void) { return boardGetRequiredPegsTotal() - m_requiredPegsInPlay; }

void boardDoRequiredPegHit(void) { --m_requiredPegsInPlay; }

uint16_t boardGetRequiredPegsTotal(void) { // This is not often needed and is not cached
  uint16_t count = 0;
  for (uint16_t i = 0; i < m_nPegs; ++i) {
    if (boardGetPeg(i)->type == kPegTypeRequired) { ++count; }
  }
  return count;
}

uint16_t boardGetSpecialPegsInPlay(void) { // This is not often needed and is not cached
  uint16_t count = 0;
  for (uint16_t i = 0; i < m_nPegs; ++i) {
    if (boardGetPeg(i)->type == kPegTypeSpecial && boardGetPeg(i)->state == kPegStateActive) { ++count; }
  }
  return count;
}

void boardDoInit(void) {
  memset(&m_pegs, 0, sizeof(struct Peg_t) * MAX_PEGS);
  for (int i = 0; i < MAX_PEGS; ++i) {
    m_pegs[i].id = i;
  }
  // m_special = kPegSpecialBlast; // TESTING
}

struct Peg_t* boardNewPegFromPool(void) { 
  if (m_nPegs == MAX_PEGS) { 
    pd->system->error("Run out of pegs!");
    return NULL;
  }
  return &m_pegs[m_nPegs++];
}

struct Peg_t* boardGetPeg(const uint16_t i) { return &m_pegs[i]; }

uint16_t boardGetNPegs(void) { return m_nPegs; }

void boardDoSpecialBlast(void) {
  const cpVect pos = physicsGetBallPosition(0);
  for (int i = 0; i < m_nPegs; ++i) {
    struct Peg_t* p = boardGetPeg(i);
    if (p->state == kPegStateActive) {
      const cpVect pegPos = cpv(p->x, p->y);
      if (cpvdist(pos, pegPos) < SPECIAL_BLAST_RADIUS) {
        pegDoHit(p);
      }
    }
  }
}

void boardDoAddWheel(const struct EllipticLoader_t* ellipticLoader) {
  for (int i = 0; i < ellipticLoader->nPegs; ++i) {
    // pd->system->logToConsole("+++ boardDoAddWheel %i of %i", i, ellipticLoader->nPegs); 
    if (ellipticLoader->types[i] == kPegTypeMissing) { continue; }
    struct Peg_t* p = boardNewPegFromPool();
    if (!p) continue;
    enum PegShape_t shape = ellipticLoader->shape;
    // -1 in the overrides is because 0 here represents Inherit, and all othher enums are incremented by 1
    if (ellipticLoader->shapeOverride[i]) { shape = (enum PegShape_t) ellipticLoader->shapeOverride[i] - 1; }
    uint8_t size = ellipticLoader->size;
    if (ellipticLoader->sizeOverride[i]) { size = ellipticLoader->sizeOverride[i] - 1; }
    pegDoInit(p, shape, ellipticLoader->x, ellipticLoader->y, ellipticLoader->angle, size);
    pegSetMotionSpeed(p, ellipticLoader->speed);
    const float angleOffset = (ellipticLoader->maxAngle / ellipticLoader->nPegs) * i;
    pegSetMotionOffset(p, angleOffset + (ellipticLoader->maxAngle));
    pegSetMotionEasing(p, ellipticLoader->easing);
    pegSetMotionDoArcAngle(p, ellipticLoader->useArc);
    pegSetMotionEllipse(p, ellipticLoader->a, ellipticLoader->b);
    if (ellipticLoader->types[i] == kPegTypeRequired) { ++m_requiredPegsInPlay; }
    pegSetType(p, ellipticLoader->types[i]);
  }
}

void boardDoAddLinear(const struct LinearLoader_t* linearLoader) {
  for (int i = 0; i < linearLoader->nPegs; ++i) {
    // pd->system->logToConsole("+++ boardDoAddLinear %i of %i", i, linearLoader->nPegs); 
    if (linearLoader->types[i] == kPegTypeMissing) { continue; }
    struct Peg_t* p = boardNewPegFromPool();
    if (!p) continue;
    enum PegShape_t shape = linearLoader->shape;
    if (linearLoader->shapeOverride[i]) { shape = (enum PegShape_t) linearLoader->shapeOverride[i] - 1; }
    uint8_t size = linearLoader->size;
    if (linearLoader->sizeOverride[i]) { size = linearLoader->sizeOverride[i] - 1; }
    pegDoInit(p, shape, linearLoader->x, linearLoader->y, linearLoader->angle, size);
    const float angleOffset = (linearLoader->maxAngle / linearLoader->nPegs) * i;
    pegSetMotionSpeed(p, linearLoader->speed);
    pegSetMotionOffset(p, angleOffset + linearLoader->maxAngle);
    pegSetMotionEasing(p, linearLoader->easing);
    pegSetMotionDoArcAngle(p, linearLoader->useArc);
    if (linearLoader->types[i] == kPegTypeRequired) { ++m_requiredPegsInPlay; }
    pegSetType(p, linearLoader->types[i]);
    for (int j = 0; j < linearLoader->nLines; ++j) {
      // pd->system->logToConsole("  +++ mp %i of %i: %i %i", j, linearLoader->nLines, linearLoader->pathX[j], linearLoader->pathY[j]);
      pegAddMotionPath(p, linearLoader->pathX[j], linearLoader->pathY[j]);
    }
    pegDoMotionPathFinalise(p);
  }
}

struct Peg_t* boardDoAddStatic(const struct StaticLoader_t* staticLoader) {
  struct Peg_t* p = boardNewPegFromPool();
  if (!p) { return NULL; }
  pegDoInit(p, staticLoader->shape, staticLoader->x, staticLoader->y, staticLoader->angle, staticLoader->size);
  pegSetMotionStatic(p);
  if (staticLoader->type == kPegTypeRequired) { ++m_requiredPegsInPlay; }
  pegSetType(p, staticLoader->type);
  return p;
}

void boardDoTestLevel(void) {
  boardDoClear();

  const int maxStatic = 16;//rand() % 2 ? 16 : 64+32;

  for (int i = 0; i < maxStatic; ++i) {
    struct StaticLoader_t staticLoader;
    staticLoader.x = rand() % DEVICE_PIX_X;
    staticLoader.y = (rand() % IOGetCurrentHoleHeight()) - TURRET_RADIUS;
    staticLoader.angle = (M_2PIf / 256.0f) * (rand() % 256);
    // staticLoader.angle = 0;
    // staticLoader.angle = (M_2PIf / 30) * (rand() % 256);
    staticLoader.shape = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
    staticLoader.size = rand() % MAX_PEG_SIZE;
    // staticLoader.size = 4;
    staticLoader.type = (i < 0 ? kPegTypeRequired : kPegTypeNormal);
    staticLoader.type = (i >= 4 && i < 6 ? kPegTypeSpecial : staticLoader.type);
    boardDoAddStatic(&staticLoader);
  }

  struct StaticLoader_t staticLoader = {0};
  staticLoader.x = DEVICE_PIX_X/2;
  staticLoader.y = DEVICE_PIX_Y;
  staticLoader.type = kPegTypeRequired;
  boardDoAddStatic(&staticLoader);  

  struct StaticLoader_t staticLoader2 = {0};
  staticLoader2.x = DEVICE_PIX_X/4;
  staticLoader2.y = DEVICE_PIX_Y/2;
  staticLoader2.type = kPegTypeSpecial;
  boardDoAddStatic(&staticLoader2);  

  // return;
  // if (maxStatic == 16) return;

  #define PEGS_PER_WHEEL 8
  #define WHEELS 2
  for (int wheelStep = 0; wheelStep < WHEELS; ++wheelStep) {
    struct EllipticLoader_t ellipticLoader = {0};
    ellipticLoader.x = rand() % DEVICE_PIX_X;
    ellipticLoader.y = (rand() % IOGetCurrentHoleHeight()) - TURRET_RADIUS;
    ellipticLoader.a = 64 + rand() % 32;
    ellipticLoader.b = 64 + rand() % 32;
    ellipticLoader.shape = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
    ellipticLoader.speed = 0.1f * ((rand() % 10) + 1); 
    ellipticLoader.size = rand() % MAX_PEG_SIZE;
    ellipticLoader.nPegs = PEGS_PER_WHEEL;
    ellipticLoader.useArc = rand() % 2;
    ellipticLoader.easing = kEaseLinear;
    ellipticLoader.angle = degToRad(rand() % 360);
    ellipticLoader.maxAngle = degToRad((rand() % 180) + 180);
    for (int i = 0; i < PEGS_PER_WHEEL; ++i) {
      ellipticLoader.types[i] = kPegTypeNormal;
      ellipticLoader.shapeOverride[i] = 0;
      ellipticLoader.sizeOverride[i] = 0;
    }
    boardDoAddWheel(&ellipticLoader);
  }
 
  // #define PEGS_PER_PATH 8
  // #define PATHS 1
  // for (int pathStep = 0; pathStep < PATHS; ++pathStep) {
  //   struct LinearLoader_t linearLoader;
  //   linearLoader.x = rand() % DEVICE_PIX_X;
  //   linearLoader.y = (rand() % IOGetCurrentHoleHeight()) - TURRET_RADIUS;
  //   linearLoader.nPegs = PEGS_PER_PATH;
  //   for (int i = 0; i < PEGS_PER_PATH; ++i) { 
  //     linearLoader.types[i] = kPegTypeNormal;
  //     linearLoader.shapeOverride[i] = 0;
  //     linearLoader.sizeOverride[i] = 0;
  //   }
  //   linearLoader.nLines = 8;
  //   linearLoader.pathX[0] = linearLoader.x - 32 + rand() % 64;
  //   linearLoader.pathY[0] = linearLoader.y - 32 + rand() % 64;
  //   for (int leg = 1; leg < linearLoader.nLines; ++leg) {
  //     linearLoader.pathX[leg] = linearLoader.pathX[leg-1] - 32 + rand() % 64;
  //     linearLoader.pathY[leg] = linearLoader.pathY[leg-1] - 32 + rand() % 64;
  //   }
  //   linearLoader.shape = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
  //   linearLoader.easing = (enum EasingFunction_t) rand() % kNEasingFunctions;
  //   linearLoader.speed = 0.1f * ((rand() % 10) + 1); 
  //   linearLoader.size = rand() % MAX_PEG_SIZE;
  //   boardDoAddLinear(&linearLoader);    
  // }

}

void boardDoUpdate(void) {
  const float closeUpMod = (FSMGet() == kGameFSM_CloseUp ? 0.25f : 1.0f);;
  const float ts = physicsGetTimestepMultiplier() * TIMESTEP * closeUpMod;
  for (int i = 0; i < m_nPegs; ++i) {
    pegDoUpdate(&m_pegs[i], ts);
  }
}

void boardDoClear(void) {
  for (int i = 0; i < m_nPegs; ++i) {
    pegDoClear(&m_pegs[i]);
  }
  m_nPegs = 0;
  m_requiredPegsInPlay = 0;
  m_specialCounter = 0;
  m_special = kPegSpecialNotSpecial;
  m_specialToActivate = kPegSpecialNotSpecial;
}

void boardDoBurstPegs(const float yLevel) {
  m_lastBurstLevel = yLevel;
  for (int i = 0; i < m_nPegs; ++i) {
    pegDoCheckBurst(&m_pegs[i], yLevel);
  }
}

bool boardDoBurstRandomPeg(void) {
  for (int i = 0; i < m_nPegs; ++i) {
    const bool popped = pegDoCheckBurst(&m_pegs[i], 0);
    if (popped) {
      return true;
    }
  }
  return false;
}

enum PegSpecial_t boardGetCurrentSpecial(void) {
  return m_special;
}

void boardDoClearSpecial(void) {
  m_special = kPegSpecialNotSpecial;
}

enum PegSpecial_t boardDoAddSpecial(const bool activate) {

  if (activate) {

    if (m_specialCounter) { --m_specialCounter; } 
    else                  { m_specialToActivate = kPegSpecialNotSpecial; }
    m_special = m_specialToActivate;

  } else {

    if (!m_specialCounter) { 
      m_specialToActivate = IOGetCurrentHoleSpecial();
      if (m_specialToActivate == kPegSpecialNotSpecial) { // Hole can request to have a random special
        m_specialToActivate = (rand() % (kNPegSpecial - 1)) + 1;
      }
    }
    ++m_specialCounter;

  }

  return m_specialToActivate;
}