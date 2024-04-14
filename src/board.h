#pragma once
#include "game.h"
#include "peg.h"

struct StaticLoader_t {
  enum PegShape_t shape;
  int16_t x;
  int16_t y;
  float angle; // in radians
  uint8_t size;
  enum PegType_t type;
};

struct EllipticLoader_t {
  enum PegShape_t shape;
  uint16_t nPegs;
  int16_t x;
  int16_t y;
  int16_t a;
  int16_t b;
  float angle; // in radians
  float maxAngle; // in radians
  float speed;
  uint8_t size;
  enum EasingFunction_t easing;
  bool useArc;

  enum PegType_t types[MAX_PEGS_ON_PATH];
  uint8_t shapeOverride[MAX_PEGS_ON_PATH];
  uint8_t sizeOverride[MAX_PEGS_ON_PATH];
};

struct LinearLoader_t {
  enum PegShape_t shape;
  uint16_t nPegs;
  uint16_t nLines;
  int16_t x;
  int16_t y;
  float angle; // in radians
  float maxAngle; // in radians
  float speed;
  uint8_t size;
  enum EasingFunction_t easing;
  bool useArc;

  enum PegType_t types[MAX_PEGS_ON_PATH];
  uint8_t shapeOverride[MAX_PEGS_ON_PATH];
  uint8_t sizeOverride[MAX_PEGS_ON_PATH];

  int16_t pathX[MAX_LINEAR_PATH_SEGMENTS];
  int16_t pathY[MAX_LINEAR_PATH_SEGMENTS];
};

void boardDoInit(void);

void boardDoRandomise(void);

void boardDoUpdate(void);

void boardDoClear(void);

void boardDoBurstPegs(const float yLevel);
bool boardDoBurstRandomPeg(void);

uint16_t boardGetNPegs(void);
struct Peg_t* boardGetPeg(const uint16_t i);

int16_t boardGetRequiredPegsInPlay(void);

void boardDoRequiredPegHit(void);

void boardDoSpecialBlast(void);

enum PegSpecial_t boardGetCurrentSpecial(void);
void boardDoClearSpecial(void);
enum PegSpecial_t boardDoAddSpecial(const bool activate);

struct Peg_t* boardDoAddStatic(const struct StaticLoader_t* staticLoader);
void boardDoAddLinear(const struct LinearLoader_t* linearLoader);
void boardDoAddWheel(const struct EllipticLoader_t* ellipticLoader);
