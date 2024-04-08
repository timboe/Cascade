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
  enum PegShape_t shapeOverride[MAX_PEGS_ON_PATH];
  uint8_t sizeOveride[MAX_PEGS_ON_PATH];
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
  enum PegShape_t shapeOverride[MAX_PEGS_ON_PATH];
  uint8_t sizeOveride[MAX_PEGS_ON_PATH];

  int16_t pathX[MAX_LINEAR_PATH_SEGMENTS];
  int16_t pathY[MAX_LINEAR_PATH_SEGMENTS];
};

void initBoard(void);

void randomiseBoard(void);

void updateBoard(void);

void renderBoard(void);

void clearBoard(void);

void popBoard(float y);

bool popRandom(void);

struct Peg_t* getPeg(uint16_t i);

int16_t requiredPegsInPlay(void);

void requiredPegHit(void);

struct Peg_t* boardAddStatic(const struct StaticLoader_t* staticLoader);

void boardAddLinear(const struct LinearLoader_t* linearLoader);

void boardAddWheel(const struct EllipticLoader_t* ellipticLoader);
