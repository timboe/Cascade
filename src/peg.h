#pragma once
#include "game.h"
#include "physics.h"
#include "easing.h"

enum PegShape_t {
  kPegShapeBall,
  kPegShapeRect
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
  kPegStateActive,
  kPegStateHit,
  kPegStateRemoved
};

struct Peg_t {
  uint16_t m_id;
  cpShape* m_cpShape;
  cpBody* m_cpBody;
  LCDBitmap* m_bitmap;
  float m_angle; // in radians
  uint8_t m_iAngle;
  float m_x;
  float m_y;
  float m_radius;
  int16_t m_xBitmap; // Top left corner for rendering
  int16_t m_yBitmap; // Top left corner for rendering
  enum PegShape_t m_shape;
  enum PegMotion_t m_motion;
  enum PegType_t m_type;
  enum PegState_t m_state;

  // Motion
  float m_time;
  float m_speed;
  enum EasingFunction_t m_easing;

  // kPegMotionEllipse
  float m_a, m_b;

  // kPegMotionPath
  uint8_t m_pathSteps;
  int16_t m_pathX[MAX_PEG_PATHS];
  int16_t m_pathY[MAX_PEG_PATHS];
  float m_pathLength[MAX_PEG_PATHS];
  float m_totPathLength;
  uint8_t m_pathCurrent;

};

void initPeg(struct Peg_t* p, const enum PegShape_t s, const float x, const float y, const float a);

void clearPeg(struct Peg_t* p);

void updatePeg(struct Peg_t* p);

void setPegMotionSpeed(struct Peg_t* p, const float s);

void setPegMotionEasing(struct Peg_t* p, const enum EasingFunction_t e);

void setPegMotionOffset(struct Peg_t* p, const float offset);

void setPegMotionEllipse(struct Peg_t* p, const float a, const float b);

void addPegMotionPath(struct Peg_t* p, const uint16_t x, const uint16_t y);

void pegMotionPathFinalise(struct Peg_t* p);