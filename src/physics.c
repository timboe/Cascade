#include <stdio.h>
#include <stdlib.h>
#include "pdxalloc.h"
#include "physics.h"
#include "input.h"

cpSpace* m_space;

cpBody* m_ball;
cpShape* m_ballShape;

bool m_ballInPlay = false;
bool m_doMotionPath = true;

uint8_t radToByte(float rad) { return (rad / (M_PIf * 2.0f)) * 256.0f; }

uint8_t angToByte(float ang) { return (ang / 360.0f) * 256.0f; }

float angToRad(float ang) { return ang * (M_PIf / 180.0f); }

cpBody* getBall(void) { return m_ball; }

cpShape* getBallShape(void) { return m_ballShape; }

bool ballInPlay(void) { return m_ballInPlay; }

void setBallInPlay(bool bip) { m_ballInPlay = bip; }

void setDoMotionPath(bool dmp) { m_doMotionPath = dmp; }

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
  cpSpaceAddShape(m_space, m_ballShape);

  // for (int i = 0; i < N_OBST; ++i) {
  //   m_obstacle[i] = cpBodyNewKinematic();
  //   cpBodySetPosition(m_obstacle[i], cpv(rand() % DEVICE_PIX_X, (rand() % DEVICE_PIX_Y) + UI_OFFSET_TOP));
  //   cpSpaceAddBody(m_space, m_obstacle[i]);
  //   cpShape* shape = cpCircleShapeNew(m_obstacle[i], BALL_RADIUS, cpvzero);
  //   cpShapeSetFriction(shape, 0.0f);
  //   cpShapeSetElasticity(shape, ELASTICITY);
  //   cpSpaceAddShape(m_space, shape);
  // }

  // for (int i = 0; i < N_OBST; i++) {
  //   //float moment = cpMomentForBox(BOX_MASS, BOX_WIDTH, BOX_HEIGHT);
  //   m_box[i] = cpBodyNewKinematic(/*BOX_MASS, moment*/);
  //   cpBodySetPosition(m_box[i], cpvzero);
  //   cpSpaceAddBody(m_space, m_box[i]);
  //   cpShape* shape = cpBoxShapeNew(m_box[i], BOX_WIDTH, BOX_HEIGHT, 0.0f);
  //   cpShapeSetFriction(shape, 0.0f);
  //   cpShapeSetElasticity(shape, ELASTICITY);
  //   cpSpaceAddShape(m_space, shape);
  // }


  cpBody* walls = cpSpaceGetStaticBody(m_space);
  cpShape* top   = cpSegmentShapeNew(walls, cpv(0,            UI_OFFSET_TOP), cpv(DEVICE_PIX_X, UI_OFFSET_TOP),  3.0f);
  cpShape* left  = cpSegmentShapeNew(walls, cpv(0,            UI_OFFSET_TOP), cpv(0,            PHYSWALL_PIX_Y), 3.0f);
  cpShape* right = cpSegmentShapeNew(walls, cpv(DEVICE_PIX_X, UI_OFFSET_TOP), cpv(DEVICE_PIX_X, PHYSWALL_PIX_Y), 3.0f);
  cpShapeSetFriction(top, 0.0f);
  cpShapeSetElasticity(top, ELASTICITY);
  cpShapeSetFriction(left, 0.0f);
  cpShapeSetElasticity(left, ELASTICITY);
  cpShapeSetFriction(right, 0.0f);
  cpShapeSetElasticity(right, ELASTICITY);
  cpSpaceAddShape(m_space, top);
  cpSpaceAddShape(m_space, left);
  cpSpaceAddShape(m_space, right);

}

void resetBall(void) {
  cpBodySetPosition(m_ball, cpv(HALF_DEVICE_PIX_X, UI_OFFSET_TOP));
  cpBodySetVelocity(m_ball, cpvzero);
  cpBodySetAngle(m_ball, 0);
  cpBodySetAngularVelocity(m_ball, 0);
}

void updateSpace(void) {

  // for (int i = 0; i < N_OBST/2; ++i) {
  //   float x = cpBodyGetPosition(m_obstacle[i]).x;
  //   if      (x >= DEVICE_PIX_X) cpBodySetVelocity(m_obstacle[i], cpv(-32.0f, 0));
  //   else if (x <= 0)            cpBodySetVelocity(m_obstacle[i], cpv(+32.0f, 0));

  //   x = cpBodyGetPosition(m_box[i]).x;
  //   if      (x >= DEVICE_PIX_X) cpBodySetVelocity(m_box[i], cpv(-32.0f, 0));
  //   else if (x <= 0)            cpBodySetVelocity(m_box[i], cpv(+32.0f, 0));
  // }

  if (!m_ballInPlay && !m_doMotionPath) {
    resetBall();
  }

  cpSpaceStep(m_space, TIMESTEP);
}