#include <stdio.h>
#include <stdlib.h>
#include "pdxalloc.h"
#include "physics.h"
#include "input.h"
#include "render.h"
#include "ui.h"
#include "peg.h"

cpSpace* m_space;

cpBody* m_ball;
cpShape* m_ballShape;

cpCollisionHandler* m_colliderHandle;

uint8_t cpCollisionBeginFunc_ballPeg(cpArbiter* arb, struct cpSpace* space, cpDataPointer data);

bool m_ballInPlay = false;

int16_t m_motionTrailX[MOTION_TRAIL_LEN];
int16_t m_motionTrailY[MOTION_TRAIL_LEN];

int16_t m_predictionTrailX[PREDICTION_TRAIL_LEN];
int16_t m_predictionTrailY[PREDICTION_TRAIL_LEN];


/// ///

uint8_t radToByte(float rad) { return (rad / M_2PIf) * 256.0f; }

uint8_t angToByte(float ang) { return (ang / 360.0f) * 256.0f; }

float angToRad(float ang) { return ang * (M_PIf / 180.0f); }

float len(const float x1, const float x2, const float y1, const float y2) {
  return sqrtf( len2(x1, x2, y1, y2) );
}

float len2(const float x1, const float x2, const float y1, const float y2) {
  return powf(x1 - x2, 2) + powf(y1 - y2, 2);
}

int16_t* motionTrailX(void) { return m_motionTrailX; }

int16_t* motionTrailY(void) { return m_motionTrailY; }

cpBody* getBall(void) { return m_ball; }

cpShape* getBallShape(void) { return m_ballShape; }

bool ballInPlay(void) { return m_ballInPlay; }

void setBallInPlay(bool bip) { m_ballInPlay = bip; }

cpSpace* getSpace(void) { return m_space; }

void launchBall(void) {
  const float angleRad = angToRad(getTurretBarrelAngle());
  cpBodyApplyImpulseAtLocalPoint(m_ball, cpv(POOT_STRENGTH * sinf(angleRad), POOT_STRENGTH * -cosf(angleRad)), cpvzero);
}

void initSpace(void) {
  m_space = cpSpaceNew();
  cpSpaceSetIterations(m_space, 10);
  cpSpaceSetGravity(m_space, G);
  cpSpaceSetSleepTimeThreshold(m_space, 0.5f);
  cpSpaceSetCollisionSlop(m_space, 0.5f); 

  // Ball
  float moment = cpMomentForCircle(BALL_MASS, 0.0f, BALL_RADIUS, cpvzero);
  m_ball = cpBodyNew(BALL_MASS, moment);
  cpBodySetPosition(m_ball, cpv(0, WFALL_PIX_Y*2));
  cpSpaceAddBody(m_space, m_ball);
  m_ballShape = cpCircleShapeNew(m_ball, BALL_RADIUS, cpvzero);
  cpShapeSetFriction(m_ballShape, 0.0f);
  cpShapeSetElasticity(m_ballShape, ELASTICITY);
  cpShapeSetCollisionType(m_ballShape, FLAG_BALL);
  cpSpaceAddShape(m_space, m_ballShape);
  resetBall();

  cpBody* walls = cpSpaceGetStaticBody(m_space);
  cpShape* top   = cpSegmentShapeNew(walls, cpv(0,            UI_OFFSET_TOP), cpv(DEVICE_PIX_X, UI_OFFSET_TOP),  3.0f);
  cpShape* left  = cpSegmentShapeNew(walls, cpv(0,            UI_OFFSET_TOP), cpv(0,            PHYSWALL_PIX_Y), 3.0f);
  cpShape* right = cpSegmentShapeNew(walls, cpv(DEVICE_PIX_X, UI_OFFSET_TOP), cpv(DEVICE_PIX_X, PHYSWALL_PIX_Y), 3.0f);
  cpShapeSetCollisionType(top, FLAG_WALL);
  cpShapeSetCollisionType(left, FLAG_WALL);
  cpShapeSetCollisionType(right, FLAG_WALL);
  cpShapeSetFriction(top, 0.0f);
  cpShapeSetElasticity(top, ELASTICITY);
  cpShapeSetFriction(left, 0.0f);
  cpShapeSetElasticity(left, ELASTICITY);
  cpShapeSetFriction(right, 0.0f);
  cpShapeSetElasticity(right, ELASTICITY);
  cpSpaceAddShape(m_space, top);
  cpSpaceAddShape(m_space, left);
  cpSpaceAddShape(m_space, right);

  m_colliderHandle = cpSpaceAddCollisionHandler(m_space, FLAG_BALL, FLAG_PEG);
  m_colliderHandle->beginFunc = cpCollisionBeginFunc_ballPeg;
}

uint8_t cpCollisionBeginFunc_ballPeg(cpArbiter* arb, struct cpSpace* space, cpDataPointer data) {
  cpBody* cpBall;
  cpBody* cpPeg;
  cpArbiterGetBodies(arb, &cpBall, &cpPeg);
  struct Peg_t* p = (struct Peg_t*) cpBodyGetUserData(cpPeg);
  hitPeg(p);
  return 1;
}

void resetBall(void) {
  cpBodySetPosition(m_ball, cpv(HALF_DEVICE_PIX_X, getMinimumY() + TURRET_RADIUS));
  cpBodySetVelocity(m_ball, cpvzero);
  cpBodySetAngle(m_ball, 0);
  cpBodySetAngularVelocity(m_ball, 0);
}

void updateSpace(int32_t fc) {
  cpSpaceStep(m_space, TIMESTEP);
  
  const float y = cpBodyGetPosition(m_ball).y;

  if (y > PHYSWALL_PIX_Y + BALL_RADIUS) {
    setBallInPlay(false);
    activateBallEndSweep();
  } 

  if (m_ballInPlay) {
    setScrollOffset(y - HALF_DEVICE_PIX_Y); // TODO - move this call
  }

  const cpVect pos = cpBodyGetPosition(m_ball);

  if (!m_ballInPlay) {
    const int i = fc % PREDICTION_TRAIL_LEN;
    if (i == 0) {
      resetBall();
      launchBall();
    }
    setBallTrace(i, pos.x, pos.y);
  }

  m_motionTrailX[fc % MOTION_TRAIL_LEN] = pos.x;
  m_motionTrailY[fc % MOTION_TRAIL_LEN] = pos.y;

}
