#include <stdio.h>
#include <stdlib.h>
#include "pdxalloc.h"

#include "physics.h"

#define TIMESTEP (1.0f / TICK_FREQUENCY)
#define G 256.0f
#define ELASTICITY 0.8f 
static cpSpace* SPACE;


static cpBody* m_ball;
static cpBody* m_obstacle[N_OBST];
static cpBody* m_box[N_OBST];

cpBody* getBall(void) { return m_ball; }

cpBody* getObst(uint32_t i) { return m_obstacle[i]; }

cpBody* getBox(uint32_t i) {return m_box[i];}

void initSpace(void) {
  SPACE = cpSpaceNew();
  cpSpaceSetIterations(SPACE, 10);
  cpSpaceSetGravity(SPACE, cpv(0.0f, G));
  cpSpaceSetSleepTimeThreshold(SPACE, 0.5f);
  cpSpaceSetCollisionSlop(SPACE, 0.5f); 

  // Ball
  float moment = cpMomentForCircle(BALL_MASS, 0.0f, BALL_RADIUS, cpvzero);
  m_ball = cpBodyNew(BALL_MASS, moment);
  cpBodySetPosition(m_ball, cpv(0, WFALL_PIX_Y*2));
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

  for (int i = 0; i < N_OBST; i++) {
    //float moment = cpMomentForBox(BOX_MASS, BOX_WIDTH, BOX_HEIGHT);
    m_box[i] = cpBodyNewKinematic(/*BOX_MASS, moment*/);
    cpBodySetPosition(m_box[i], cpvzero);
    cpSpaceAddBody(SPACE, m_box[i]);
    shape = cpBoxShapeNew(m_box[i], BOX_WIDTH, BOX_HEIGHT, 0.0f);
    cpShapeSetFriction(shape, 0.0f);
    cpShapeSetElasticity(shape, ELASTICITY);
    cpSpaceAddShape(SPACE, shape);
  }


  cpBody* walls = cpSpaceGetStaticBody(SPACE);
  cpShape* top   = cpSegmentShapeNew(walls, cpv(0,            UI_OFFSET_TOP), cpv(DEVICE_PIX_X, UI_OFFSET_TOP),  3.0f);
  cpShape* left  = cpSegmentShapeNew(walls, cpv(0,            UI_OFFSET_TOP), cpv(0,            PHYSWALL_PIX_Y), 3.0f);
  cpShape* right = cpSegmentShapeNew(walls, cpv(DEVICE_PIX_X, UI_OFFSET_TOP), cpv(DEVICE_PIX_X, PHYSWALL_PIX_Y), 3.0f);
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


  for (int i = 0; i < N_OBST/2; ++i) {
    float x = cpBodyGetPosition(m_obstacle[i]).x;
    if      (x >= DEVICE_PIX_X) cpBodySetVelocity(m_obstacle[i], cpv(-32.0f, 0));
    else if (x <= 0)            cpBodySetVelocity(m_obstacle[i], cpv(+32.0f, 0));

    x = cpBodyGetPosition(m_box[i]).x;
    if      (x >= DEVICE_PIX_X) cpBodySetVelocity(m_box[i], cpv(-32.0f, 0));
    else if (x <= 0)            cpBodySetVelocity(m_box[i], cpv(+32.0f, 0));
  }

  cpSpaceStep(SPACE, TIMESTEP);
}