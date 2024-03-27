#include "peg.h"
#include "bitmap.h"

void initPeg(struct Peg_t* p, const enum PegShape_t s, const enum PegMotion_t m, const enum PegType_t t, const float x, const float y, const float a) {
  if (p->m_cpBody) {
    pd->system->error("Error initPeg called on an already initalised peg");
    return;
  }

  p->m_shape = s;
  p->m_motion = m;
  p->m_type = t;
  p->m_state = kPegStateActive;
  p->m_angle = a;
  p->m_iAngle = radToByte(a);
  
  p->m_cpBody = cpBodyNewKinematic();
  cpBodySetPosition(p->m_cpBody, cpv(x, y));
  cpBodySetAngle(p->m_cpBody, a);
  cpBodySetUserData(p->m_cpBody, (void*)p);
  cpSpaceAddBody(getSpace(), p->m_cpBody);
  
  if (s == kPegShapeBall) {
    p->m_cpShape = cpCircleShapeNew(p->m_cpBody, BALL_RADIUS, cpvzero);
    p->m_xBitmap = x - BALL_RADIUS;
    p->m_yBitmap = y - BALL_RADIUS;
    p->m_bitmap = getBitmapBall(p->m_iAngle);
  } else if (s == kPegShapeRect) {
    p->m_cpShape = cpBoxShapeNew(p->m_cpBody, BOX_WIDTH, BOX_HEIGHT, 0.0f);
    p->m_xBitmap = x - BOX_MAX;
    p->m_yBitmap = y - BOX_MAX;
    p->m_bitmap = getBitmapRect(p->m_iAngle);
  } else {    
    pd->system->error("Error initPeg called with unknown peg shape");
    clearPeg(p);
    return;
  }
  cpShapeSetFriction(p->m_cpShape, FRICTION);
  cpShapeSetElasticity(p->m_cpShape, ELASTICITY);
  cpSpaceAddShape(getSpace(), p->m_cpShape);

  addPegMotionPath(p, x, y);
  p->m_pathCurrent = 1;
}

void clearPeg(struct Peg_t* p) {
  if (p->m_cpBody) {
    cpSpaceRemoveBody(getSpace(), p->m_cpBody);
    cpBodyFree(p->m_cpBody);
  }
  if (p->m_cpShape) {
    cpSpaceRemoveShape(getSpace(), p->m_cpShape);
    cpShapeFree(p->m_cpShape);
  }
  memset(p, 0, sizeof(struct Peg_t));
}

// void pegPositionFunc(cpBody* body, cpFloat dt) {
  // if (!pegMotionOn()) { 
  //   return;
  // }

  // struct Peg_t* p = (struct Peg_t*) cpBodyGetUserData(body);

void updatePeg(struct Peg_t* p) {

  if (p->m_motion == kPegMotionStatic) {
    // noop - and we can skip below too
    return;
  } else if (p->m_motion == kPegMotionEllipse) {
    p->m_time += (TIMESTEP * p->m_speed);
    p->m_x = p->m_a * cosf(p->m_time) + p->m_h;
    p->m_y = p->m_b * sinf(p->m_time) + p->m_k;
  } else if (p->m_motion == kPegMotionPath) {
    // TODO
  } else {
    pd->system->error("Error updatePeg called with unknown peg motion");
  }

  cpBodySetPosition(p->m_cpBody, cpv(p->m_x, p->m_y));

  if (p->m_shape == kPegShapeBall) {
    p->m_xBitmap = p->m_x - BALL_RADIUS;
    p->m_yBitmap = p->m_y - BALL_RADIUS;
  } else if (p->m_shape == kPegShapeRect) {
    p->m_xBitmap = p->m_x - BOX_MAX;
    p->m_yBitmap = p->m_y - BOX_MAX;
  }
}

void setPegMotionSpeed(struct Peg_t* p, const float s) { p->m_speed = s; }

void setPegMotionEllipse(struct Peg_t* p, const float a, const float b, const float k, const float h) {
  p->m_a = a;
  p->m_b = b;
  p->m_k = k;
  p->m_h = h;
}

void addPegMotionPath(struct Peg_t* p, const uint16_t x, const uint16_t y) {
  if (p->m_pathSteps == MAX_PEG_PATHS) {
    pd->system->error("Error addPegMotionPath has reached the max of %i paths", MAX_PEG_PATHS);
    return;
  }
  p->m_pathX[p->m_pathSteps] = x;
  p->m_pathY[p->m_pathSteps] = y;
  p->m_pathSteps++;
}