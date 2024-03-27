#include <stdio.h>
#include <stdlib.h>
#include "pdxalloc.h"

#include "physics.h"


static cpSpace* m_space;

static cpBody* m_ball;
static cpShape* m_ballShape;

static cpBody* m_obstacle[N_OBST];
static cpBody* m_box[N_OBST];

bool m_ballInPlay = false;

cpBody* getBall(void) { return m_ball; }

cpShape* getBallShape(void) { return m_ballShape; }

cpBody* getObst(uint32_t i) { return m_obstacle[i]; }

cpBody* getBox(uint32_t i) {return m_box[i];}

bool ballInPlay(void) { return m_ballInPlay; }

void setBallInPlay(bool bip) { m_ballInPlay = bip; }

cpSpace* getSpace(void) { return m_space; }

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

  for (int i = 0; i < N_OBST; ++i) {
    m_obstacle[i] = cpBodyNewKinematic();
    cpBodySetPosition(m_obstacle[i], cpv(rand() % DEVICE_PIX_X, (rand() % DEVICE_PIX_Y) + UI_OFFSET_TOP));
    cpSpaceAddBody(m_space, m_obstacle[i]);
    cpShape* shape = cpCircleShapeNew(m_obstacle[i], BALL_RADIUS, cpvzero);
    cpShapeSetFriction(shape, 0.0f);
    cpShapeSetElasticity(shape, ELASTICITY);
    cpSpaceAddShape(m_space, shape);
  }

  for (int i = 0; i < N_OBST; i++) {
    //float moment = cpMomentForBox(BOX_MASS, BOX_WIDTH, BOX_HEIGHT);
    m_box[i] = cpBodyNewKinematic(/*BOX_MASS, moment*/);
    cpBodySetPosition(m_box[i], cpvzero);
    cpSpaceAddBody(m_space, m_box[i]);
    cpShape* shape = cpBoxShapeNew(m_box[i], BOX_WIDTH, BOX_HEIGHT, 0.0f);
    cpShapeSetFriction(shape, 0.0f);
    cpShapeSetElasticity(shape, ELASTICITY);
    cpSpaceAddShape(m_space, shape);
  }


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

void updateSpace(void) {


  for (int i = 0; i < N_OBST/2; ++i) {
    float x = cpBodyGetPosition(m_obstacle[i]).x;
    if      (x >= DEVICE_PIX_X) cpBodySetVelocity(m_obstacle[i], cpv(-32.0f, 0));
    else if (x <= 0)            cpBodySetVelocity(m_obstacle[i], cpv(+32.0f, 0));

    x = cpBodyGetPosition(m_box[i]).x;
    if      (x >= DEVICE_PIX_X) cpBodySetVelocity(m_box[i], cpv(-32.0f, 0));
    else if (x <= 0)            cpBodySetVelocity(m_box[i], cpv(+32.0f, 0));
  }

  if (!m_ballInPlay) {
    cpBodySetPosition(getBall(), cpv(HALF_DEVICE_PIX_X, UI_OFFSET_TOP));
    cpBodySetVelocity(getBall(), cpvzero);
    cpBodySetAngle(getBall(), 0);
    cpBodySetAngularVelocity(getBall(), 0);
  }

  cpSpaceStep(m_space, TIMESTEP);
}