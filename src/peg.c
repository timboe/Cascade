#include "peg.h"
#include "bitmap.h"
#include "render.h"
#include "board.h"

void updateAngle(struct Peg_t* p, float angle);

void setPegBitmapCoordinates(struct Peg_t* p);

///

void initPeg(struct Peg_t* p, const enum PegShape_t s, const float x, const float y, const float a, const uint8_t size) {
  if (p->m_cpBody) {
    pd->system->error("Error initPeg called on an already initalised peg");
    return;
  }
  if (size >= MAX_PEG_SIZE) {
    pd->system->error("Error initPeg called with too large size %i", size);
    return;
  }

  p->m_shape = s;
  p->m_motion = kPegMotionStatic;
  p->m_type = kPegTypeNormal;
  p->m_state = kPegStateActive;
  p->m_x = x;
  p->m_y = y;
  p->m_minY = y;
  p->m_angle = a;
  p->m_iAngle = radToByte(a);
  p->m_speed = 1.0f;
  p->m_size = size;
  p->m_easing = EaseLinear;
  const float scale = sizeToScale(size);

  p->m_cpBody = cpBodyNewKinematic();
  cpBodySetPosition(p->m_cpBody, cpv(x, y));
  cpBodySetAngle(p->m_cpBody, a);
  cpBodySetUserData(p->m_cpBody, (void*)p);
  cpSpaceAddBody(getSpace(), p->m_cpBody);
  
  if (s == kPegShapeBall) {
    p->m_cpShape = cpCircleShapeNew(p->m_cpBody, BALL_RADIUS*scale, cpvzero);
    p->m_radius = BALL_RADIUS*scale;
  } else if (s == kPegShapeRect) {
    p->m_cpShape = cpBoxShapeNew(p->m_cpBody, BOX_WIDTH*scale, BOX_HEIGHT*scale, 0.0f);
    p->m_radius = BOX_MAX*scale;
  } else {    
    pd->system->error("Error initPeg called with unknown peg shape");
    clearPeg(p);
    return;
  }
  p->m_bitmap = getBitmapPeg(p); // Call after setting m_shape, m_iAngle and m_size
  setPegBitmapCoordinates(p);
  cpShapeSetCollisionType(p->m_cpShape, FLAG_PEG);
  cpShapeSetFriction(p->m_cpShape, FRICTION);
  cpShapeSetElasticity(p->m_cpShape, ELASTICITY);
  cpSpaceAddShape(getSpace(), p->m_cpShape);

  addPegMotionPath(p, x, y); // Motion path location 0 always holds the origin coordinate
  p->m_pathCurrent = 1; // This then pre-assumes addPegMotionPath will be called >0 times for kPegMotionPath
}

void clearPeg(struct Peg_t* p) {
  removePeg(p);
  memset(p, 0, sizeof(struct Peg_t));
}

void removePeg(struct Peg_t* p) {
  if (p->m_cpBody) {
    cpSpaceRemoveBody(getSpace(), p->m_cpBody);
    cpBodyFree(p->m_cpBody);
    p->m_cpBody = NULL;
  }
  if (p->m_cpShape) {
    cpSpaceRemoveShape(getSpace(), p->m_cpShape);
    cpShapeFree(p->m_cpShape);
    p->m_cpShape = NULL;
  }
}

void setPegType(struct Peg_t* p, enum PegType_t type) {
  p->m_type = type;
  p->m_bitmap = getBitmapPeg(p);
}

void setPegBitmapCoordinates(struct Peg_t* p) {
  p->m_xBitmap = p->m_x - p->m_radius;
  p->m_yBitmap = p->m_y - p->m_radius;
}

void updateAngle(struct Peg_t* p, float angle) {
  if (p->m_shape == kPegShapeBall) {
    return; // Time saving, the angle is irrelevent for circles
  }
  p->m_angle = angle;
  p->m_iAngle = radToByte(angle);
  cpBodySetAngle(p->m_cpBody, angle);
  p->m_bitmap = getBitmapPeg(p);
}

void updatePeg(struct Peg_t* p) {
  if (p->m_state == kPegStateRemoved) {
    return;
  }

  p->m_time += (TIMESTEP * p->m_speed);
  if (p->m_time >= M_2PIf) { p->m_time -= M_2PIf; } // Keep this one bounded

  if (p->m_motion == kPegMotionStatic) {

    return; // noop - and we can skip below too

  } else if (p->m_motion == kPegMotionEllipse) {

    const float easing = getEasing(p->m_easing, p->m_time / M_2PIf) * M_2PIf;
    p->m_x = p->m_pathX[0] + (p->m_a * cosf(easing));
    p->m_y = p->m_pathY[0] + (p->m_b * sinf(easing));
    updateAngle(p, easing + (M_PIf * 0.5f) ); // Offset to point inwards

  } else if (p->m_motion == kPegMotionPath) {

    uint8_t pathStep = 0;
    float stepLenFrac = 0;
    float totLenFrac = (p->m_time / M_2PIf);
    
    // TODO - cache this for calls after the first

    while (true) {
      stepLenFrac = p->m_pathLength[pathStep] / p->m_totPathLength;
      if (totLenFrac < stepLenFrac) {
        break;
      } else {
        totLenFrac -= stepLenFrac;
        ++pathStep;
      }
    }

    // totLenFrac is now 0-X, where X is this step's fraction of the total path length: stepLenFrac.
    // we want to scale this back into the range 0-1 and apply easing
    const float lenFrac = getEasing(p->m_easing, totLenFrac / stepLenFrac);

    // and we scale the difference between the points by this value
    const float dx = (p->m_pathX[pathStep+1] - p->m_pathX[pathStep]) * lenFrac;
    const float dy = (p->m_pathY[pathStep+1] - p->m_pathY[pathStep]) * lenFrac;

    p->m_x = p->m_pathX[pathStep] + dx;
    p->m_y = p->m_pathY[pathStep] + dy;

  } else {

    pd->system->error("Error updatePeg called with unknown peg motion");

  }

  const cpVect pos = cpBodyGetPosition(p->m_cpBody);
  cpBodySetVelocity(p->m_cpBody, cpv((p->m_x - pos.x)/TIMESTEP, (p->m_y - pos.y)/TIMESTEP));
  // cpBodySetPosition(p->m_cpBody, cpv(p->m_x, p->m_y));
  setPegBitmapCoordinates(p);
  if (p->m_y < p->m_minY) { p->m_minY = p->m_y; }
}

void setPegMotionSpeed(struct Peg_t* p, const float s) { p->m_speed = s; }

void setPegMotionEasing(struct Peg_t* p, const enum EasingFunction_t e) { p->m_easing = e; }

void setPegMotionOffset(struct Peg_t* p, const float offset) { p->m_time = offset; }

void emptyCpBodyVelocityFunc(cpBody* body, cpVect gravity, cpFloat damping, cpFloat dt) {}

void emptyCpBodyPositionFunc(cpBody* body, cpFloat dt) {};

void setPegMotionStatic(struct Peg_t* p) {
  cpBodySetVelocityUpdateFunc(p->m_cpBody, emptyCpBodyVelocityFunc);
  cpBodySetPositionUpdateFunc(p->m_cpBody, emptyCpBodyPositionFunc);
}

void setPegMotionEllipse(struct Peg_t* p, const float a, const float b) {
  p->m_a = a;
  p->m_b = b;
  p->m_motion = kPegMotionEllipse;
}

void addPegMotionPath(struct Peg_t* p, const int16_t x, const int16_t y) {
  if (p->m_pathSteps == MAX_LINEAR_PATH_SEGMENTS) {
    pd->system->error("Error addPegMotionPath has reached the max of %i paths", MAX_LINEAR_PATH_SEGMENTS);
    return;
  }
  p->m_pathX[p->m_pathSteps] = x;
  p->m_pathY[p->m_pathSteps] = y;
  if (p->m_pathSteps) {
    const uint8_t cur = p->m_pathSteps;
    const uint8_t prv = cur - 1;
    p->m_pathLength[prv] = len(p->m_pathX[cur], p->m_pathX[prv], p->m_pathY[cur], p->m_pathY[prv]);
    p->m_totPathLength += p->m_pathLength[prv];
  }
  p->m_pathSteps++;
}

void pegMotionPathFinalise(struct Peg_t* p) {
  // Close the loop
  addPegMotionPath(p, p->m_pathX[0], p->m_pathY[0]);
  p->m_motion = kPegMotionPath;
}

void renderPeg(const struct Peg_t* p) {
  if (p->m_state == kPegStateRemoved) {
    return;
  } else if (p->m_state == kPegStateHit) {
    pd->graphics->setDrawMode(kDrawModeInverted);
  }
  pd->graphics->drawBitmap(p->m_bitmap, p->m_xBitmap, p->m_yBitmap, kBitmapUnflipped);
  pd->graphics->setDrawMode(kDrawModeCopy);
  if (!ballInPlay()) {
    if (p->m_motion == kPegMotionEllipse) {
      pd->graphics->fillEllipse(p->m_pathX[0]-3, p->m_pathY[0]-3, 6, 6, 0.0f, 360.0f, kColorWhite);
      pd->graphics->fillEllipse(p->m_pathX[0]-2, p->m_pathY[0]-2, 4, 4, 0.0f, 360.0f, kColorBlack);
    } else if (p->m_motion == kPegMotionPath) {
      for (int j = 1; j < p->m_pathSteps; ++j) {
        pd->graphics->drawLine(p->m_pathX[j], p->m_pathY[j], p->m_pathX[j-1], p->m_pathY[j-1], 2, kColorWhite);
      }
    }
  }
}

void hitPeg(struct Peg_t* p) {
  if (p->m_state == kPegStateActive && ballInPlay()) {
    p->m_state = kPegStateHit;
    resetBallStuckCounter();
    if (p->m_type == kPegTypeRequired) {
      addTrauma(TRAMA_REQUIRED_HIT);
      addFreeze(FREEZE_REQUIRED_HIT);
      requiredPegHit();
    } else {
      addTrauma(TRAMA_PEG_HIT);
      addFreeze(FREEZE_PEG_HIT);
    }
  }
  // pd->system->logToConsole("bam!");
}

bool checkPopPeg(struct Peg_t* p, float y) {
  if (p->m_state == kPegStateHit && p->m_y >= y) {
    p->m_state = kPegStateRemoved;
    removePeg(p);
    return true;
  }
  return false;
}