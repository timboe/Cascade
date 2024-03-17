#include <stdio.h>
#include <stdlib.h>
#include "pdxalloc.h"

#include "physics.h"

#define TIMESTEP (1.0f / TICK_FREQUENCY)
#define G 400.0f
#define ELASTICITY 0.98f 
static cpSpace* SPACE;


static cpBody* m_ball;
static cpBody* m_obstacle[N_OBST];
static cpBody* m_walls;

cpBody* getBall(void) { return m_ball; }

cpBody* getObst(uint32_t i) { return m_obstacle[i]; }

void initSpace(void) {
  SPACE = cpSpaceNew();
  cpSpaceSetIterations(SPACE, 20);
  cpSpaceSetGravity(SPACE, cpv(0.0f, G));
  cpSpaceSetSleepTimeThreshold(SPACE, 0.5f);
  cpSpaceSetCollisionSlop(SPACE, 0.5f);

  // Ball
  float moment = cpMomentForCircle(BALL_MASS, 0.0f, BALL_RADIUS, cpvzero);
  m_ball = cpBodyNew(BALL_MASS, moment);
  cpBodySetPosition(m_ball, cpv(HALF_DEVICE_PIX_X, UI_OFFSET_TOP + BALL_RADIUS));
  cpSpaceAddBody(SPACE, m_ball);
  cpShape* shape = cpCircleShapeNew(m_ball, BALL_RADIUS, cpvzero);
  cpShapeSetFriction(shape, 0.0f);
  cpShapeSetElasticity(shape, ELASTICITY);
  cpSpaceAddShape(SPACE, shape);

  for (int i = 0; i < N_OBST; ++i) {
    m_obstacle[i] = cpBodyNewKinematic();
    cpBodySetPosition(m_obstacle[i], cpv(rand() % DEVICE_PIX_X, (rand() % DEVICE_PIX_Y) + UI_OFFSET_TOP));
    cpSpaceAddBody(SPACE, m_obstacle[i]);
    cpShape* shape = cpCircleShapeNew(m_obstacle[i], BALL_RADIUS, cpvzero);
    cpShapeSetFriction(shape, 0.0f);
    cpShapeSetElasticity(shape, ELASTICITY);
    cpSpaceAddShape(SPACE, shape);
  }


  m_walls = cpBodyNewStatic();
  cpShape* top   = cpSegmentShapeNew(m_walls, cpv(0, UI_OFFSET_TOP), cpv(DEVICE_PIX_X, UI_OFFSET_TOP), 1.0f);
  cpShape* left  = cpSegmentShapeNew(m_walls, cpv(0, UI_OFFSET_TOP), cpv(0, PHYSWALL_PIX_Y), 1.0f);
  cpShape* right = cpSegmentShapeNew(m_walls, cpv(DEVICE_PIX_X, UI_OFFSET_TOP), cpv(DEVICE_PIX_X, PHYSWALL_PIX_Y), 1.0f);
  cpShapeSetFriction(top, 0.0f);
  cpShapeSetElasticity(top, ELASTICITY);
  cpShapeSetFriction(left, 0.0f);
  cpShapeSetElasticity(left, ELASTICITY);
  cpShapeSetFriction(right, 0.0f);
  cpShapeSetElasticity(right, ELASTICITY);
  cpSpaceAddShape(SPACE, top);
  cpSpaceAddShape(SPACE, left);
  cpSpaceAddShape(SPACE, right);

  // cpShape *shape;
  // cpBody *staticBody = cpSpaceGetStaticBody(SPACE);
  // shape = cpSpaceAddShape(SPACE,
  //  cpSegmentShapeNew(staticBody, cpv(0, 0), cpv(400, 0), 0.0f));
  // cpShapeSetElasticity(shape, 1.0f);
  // cpShapeSetFriction(shape, 1.0f);

  // shape = cpSpaceAddShape(SPACE,
  //  cpSegmentShapeNew(staticBody, cpv(400, 0), cpv(400, 240), 0.0f));
  // cpShapeSetElasticity(shape, 1.0f);
  // cpShapeSetFriction(shape, 1.0f);

  // shape = cpSpaceAddShape(SPACE,
  //  cpSegmentShapeNew(staticBody, cpv(400, 240), cpv(0, 240), 0.0f));
  // cpShapeSetElasticity(shape, 1.0f);
  // cpShapeSetFriction(shape, 1.0f);

  // shape = cpSpaceAddShape(SPACE,
  //  cpSegmentShapeNew(staticBody, cpv(0, 240), cpv(0, 0), 0.0f));
  // cpShapeSetElasticity(shape, 1.0f);
  // cpShapeSetFriction(shape, 1.0f);
}

void updateSpace(void) {
  cpSpaceStep(SPACE, TIMESTEP);
}