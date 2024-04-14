#pragma once
#include "game.h"
#include "physics.h"
#include "easing.h"

enum PegShape_t {
  kPegShapeRect,
  kPegShapeBall,
  kPegShapeHex
};

enum PegMotion_t {
  kPegMotionStatic,
  kPegMotionEllipse,
  kPegMotionPath
};

enum PegType_t {
  kPegTypeNormal,
  kPegTypeRequired,
  kPegTypeSpecial
};

enum PegState_t {
  kPegStateUninitialised,
  kPegStateActive,
  kPegStateHit,
  kPegStateRemoved
};

enum PegSpecial_t {
  kPegSpecialNotSpecial,
  kPegSpecialAim,
  kPegSpecialSecondTry,
  kPegSpecialBlast,
  kPegSpecialMultiball,
  kPegSpecialBounce,
  kPegSpecialPenetrate,
  kNPegSpecial
}; // magnets?

struct Peg_t {
  uint16_t id;
  cpShape* cpShape;
  cpBody* cpBody;
  LCDBitmap* bitmap;
  float angle; // in radians
  uint8_t iAngle;
  float x;
  float y;
  float minY; // Based on movement path
  float radius;
  uint8_t size;
  int16_t xBitmap; // Top left corner for rendering
  int16_t yBitmap; // Top left corner for rendering
  enum PegShape_t shape;
  enum PegMotion_t motion;
  enum PegType_t type;
  enum PegState_t state;
  bool queueRemove;

  // Motion
  float time;
  float speed;
  enum EasingFunction_t easing;
  bool doArcAngle;

  // kPegMotionEllipse
  float a, b;

  // kPegMotionPath
  uint8_t pathSteps;
  int16_t pathX[MAX_LINEAR_PATH_SEGMENTS];
  int16_t pathY[MAX_LINEAR_PATH_SEGMENTS];
  float pathLength[MAX_LINEAR_PATH_SEGMENTS];
  float totPathLength;
  uint8_t pathCurrent;

};

void pegDoInit(struct Peg_t* p, const enum PegShape_t s, const float x, const float y, const float a, const uint8_t size);

void pegDoClear(struct Peg_t* p);

void pegDoRemove(struct Peg_t* p);

void pegDoUpdate(struct Peg_t* p);

void pegSetType(struct Peg_t* p, const enum PegType_t type);

void pegSetMotionSpeed(struct Peg_t* p, const float s);

void pegSetMotionEasing(struct Peg_t* p, const enum EasingFunction_t e);

void pegSetMotionOffset(struct Peg_t* p, const float offset);

void pegSetMotionEllipse(struct Peg_t* p, const float a, const float b);

void pegSetMotionDoArcAngle(struct Peg_t* p, const bool doArcAngle);

void pegSetMotionStatic(struct Peg_t* p);

void pegAddMotionPath(struct Peg_t* p, const int16_t x, const int16_t y);

void pegDoMotionPathFinalise(struct Peg_t* p);

void pegDoHit(struct Peg_t* p);

bool pegDoCheckBurst(struct Peg_t* p, const float y);

const char* pegGetSpecialTxt(const uint8_t s);