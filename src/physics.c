#include <stdio.h>
#include <stdlib.h>
#include "pdxalloc.h"
#include "physics.h"
#include "input.h"
#include "render.h"
#include "ui.h"

cpSpace* m_space;

cpBody* m_ball;
cpShape* m_ballShape;

bool m_ballInPlay = false;
bool m_doMotionPath = false;

uint8_t radToByte(float rad) { return (rad / M_2PIf) * 256.0f; }

uint8_t angToByte(float ang) { return (ang / 360.0f) * 256.0f; }

float angToRad(float ang) { return ang * (M_PIf / 180.0f); }

float len(const float x1, const float x2, const float y1, const float y2) {
  return sqrtf( len2(x1, x2, y1, y2) );
}

float len2(const float x1, const float x2, const float y1, const float y2) {
  return powf(x1 - x2, 2) + powf(y1 - y2, 2);
}

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
  cpSpaceAddShape(m_space, m_ballShape);

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
  cpBodySetPosition(m_ball, cpv(HALF_DEVICE_PIX_X, UI_OFFSET_TOP + BALL_RADIUS));
  cpBodySetVelocity(m_ball, cpvzero);
  cpBodySetAngle(m_ball, 0);
  cpBodySetAngularVelocity(m_ball, 0);
}

void updateSpace(float timestep) {
  cpSpaceStep(m_space, timestep);
  
  const float y = cpBodyGetPosition(m_ball).y;
  if (y > PHYSWALL_PIX_Y + BALL_RADIUS) {
    setBallInPlay(false);
  } 

  if (m_ballInPlay) {
    setScrollOffset(y - HALF_DEVICE_PIX_Y); // TODO - smooth this and move this call
  }

  if (!m_ballInPlay && !m_doMotionPath) {
    resetBall();
  }
}

void updatePath(void) {
  m_doMotionPath = true;
  const cpVect center = cpBodyGetPosition(m_ball);
  uint8_t iTrace = 0;
  setBallTrace(iTrace++, center.x, center.y);
  launchBall();
  for(int i=0; i<16; i++){
    updateSpace(TIMESTEP*4);
    if(i % 4 == 0){
      const cpVect center = cpBodyGetPosition(m_ball);
      setBallTrace(iTrace++, center.x, center.y);
    }
  }
  resetBall();
  m_doMotionPath = false;
}