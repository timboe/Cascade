#pragma once
#include "game.h"
#include "physics.h"

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
  cpShape* m_cpShape;
  cpBody* m_cpBody;
  LCDBitmap* m_bitmap;
  float m_angle; // in radians
  uint8_t m_iAngle;
  float m_x;
  float m_y;
  uint16_t m_xBitmap; // Top left corner for rendering
  uint16_t m_yBitmap; // Top left corner for rendering
  enum PegShape_t m_shape;
  enum PegMotion_t m_motion;
  enum PegType_t m_type;
  enum PegState_t m_state;

  // Motion
  float m_speed;

  // kPegMotionEllipse
  float m_time;
  float m_a, m_b, m_k, m_h;

  // kPegMotionPath
  uint8_t m_pathSteps;
  uint16_t m_pathX[MAX_PEG_PATHS];
  uint16_t m_pathY[MAX_PEG_PATHS];
  uint8_t m_pathCurrent;

};

void initPeg(struct Peg_t* p, const enum PegShape_t s, const enum PegMotion_t m, const enum PegType_t t, const float x, const float y, const float a);

void clearPeg(struct Peg_t* p);

void updatePeg(struct Peg_t* p);

void setPegMotionSpeed(struct Peg_t* p, const float s);

void setPegMotionEllipse(struct Peg_t* p, const float a, const float b, const float k, const float h);

void addPegMotionPath(struct Peg_t* p, const uint16_t x, const uint16_t y);