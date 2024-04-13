#include "peg.h"
#include "bitmap.h"
#include "render.h"
#include "board.h"
#include "sshot.h"
#include "util.h"

void pegDoUpdateAngle(struct Peg_t* p, float angle);

void pegSetBitmapCoordinates(struct Peg_t* p);

///   

void pegDoInit(struct Peg_t* p, const enum PegShape_t s, const float x, const float y, const float a, const uint8_t size) {
  if (p->cpBody) {
    pd->system->error("Error pegDoInit called on an already initalised peg");
    return;
  }
  if (size >= MAX_PEG_SIZE) {
    pd->system->error("Error pegDoInit called with too large size %i", size);
    return;
  }

  p->shape = s;
  p->motion = kPegMotionStatic;
  p->type = kPegTypeNormal;
  p->state = kPegStateActive;
  p->x = x;
  p->y = y;
  p->minY = y;
  p->angle = a;
  p->iAngle = radToByte(a);
  p->speed = 1.0f;
  p->size = size;
  p->easing = kEaseLinear;
  const float scale = sizeToScale(size);

  p->cpBody = cpBodyNewKinematic();
  cpBodySetPosition(p->cpBody, cpv(x, y));
  cpBodySetAngle(p->cpBody, a);
  cpBodySetUserData(p->cpBody, (void*)p);
  cpSpaceAddBody(physicsGetSpace(), p->cpBody);
  
  if (s == kPegShapeBall) {
    p->cpShape = cpCircleShapeNew(p->cpBody, BALL_RADIUS*scale, cpvzero);
    p->radius = BALL_RADIUS*scale;
  } else if (s == kPegShapeRect) {
    p->cpShape = cpBoxShapeNew(p->cpBody, BOX_WIDTH*scale, BOX_HEIGHT*scale, 0.0f);
    p->radius = BOX_MAX*scale;
  } else {    
    pd->system->error("Error pegDoInit called with unknown peg shape");
    pegDoClear(p);
    return;
  }
  p->bitmap = getBitmapPeg(p); // Call after setting shape, iAngle and size
  pegSetBitmapCoordinates(p);
  cpShapeSetCollisionType(p->cpShape, FLAG_PEG);
  cpShapeSetFriction(p->cpShape, FRICTION);
  cpShapeSetElasticity(p->cpShape, ELASTICITY);
  cpSpaceAddShape(physicsGetSpace(), p->cpShape);

  pegAddMotionPath(p, x, y); // Motion path location 0 always holds the origin coordinate
  p->pathCurrent = 1; // This then pre-assumes pegAddMotionPath will be called >0 times for kPegMotionPath
}

void pegDoClear(struct Peg_t* p) {
  pegDoRemove(p);
  memset(p, 0, sizeof(struct Peg_t));
}

void pegDoRemove(struct Peg_t* p) {
  if (p->cpBody) {
    cpSpaceRemoveBody(physicsGetSpace(), p->cpBody);
    cpBodyFree(p->cpBody);
    p->cpBody = NULL;
  }
  if (p->cpShape) {
    cpSpaceRemoveShape(physicsGetSpace(), p->cpShape);
    cpShapeFree(p->cpShape);
    p->cpShape = NULL;
  }
}

void petSetType(struct Peg_t* p, const enum PegType_t type) {
  p->type = type;
  p->bitmap = getBitmapPeg(p);
}

void pegSetBitmapCoordinates(struct Peg_t* p) {
  p->xBitmap = p->x - p->radius;
  p->yBitmap = p->y - p->radius;
}

void pegDoUpdateAngle(struct Peg_t* p, float angle) {
  if (p->shape == kPegShapeBall) {
    return; // Time saving, the angle is irrelevent for circles
  }
  p->angle = angle;
  p->iAngle = radToByte(angle);
  cpBodySetAngle(p->cpBody, angle);
  p->bitmap = getBitmapPeg(p);
}

void pegDoUpdate(struct Peg_t* p) {
  if (p->state == kPegStateRemoved) {
    return;
  }

  p->time += (TIMESTEP * p->speed);
  if (p->time >= M_2PIf) { p->time -= M_2PIf; } // Keep this one bounded

  if (p->motion == kPegMotionStatic) {

    return; // noop - and we can skip below too

  } else if (p->motion == kPegMotionEllipse) {

    const float easing = getEasing(p->easing, p->time / M_2PIf) * M_2PIf;
    p->x = p->pathX[0] + (p->a * cosf(easing));
    p->y = p->pathY[0] + (p->b * sinf(easing));
    if (p->doArcAngle) {
      pegDoUpdateAngle(p, easing + (M_PIf * 0.5f) ); // Offset to point inwards
    }

  } else if (p->motion == kPegMotionPath) {

    uint8_t pathStep = 0;
    float stepLenFrac = 0;
    float totLenFrac = (p->time / M_2PIf);
    
    // TODO - this isn't great, no easing
    if (p->doArcAngle) {
      pegDoUpdateAngle(p, (totLenFrac * M_2PIf) + (M_PIf * 0.5f) ); // Offset to point inwards
    }

    // TODO - cache this for calls after the first

    while (true) {
      stepLenFrac = p->pathLength[pathStep] / p->totPathLength;
      if (totLenFrac < stepLenFrac) {
        break;
      } else {
        totLenFrac -= stepLenFrac;
        ++pathStep;
      }
    }

    // totLenFrac is now 0-X, where X is this step's fraction of the total path length: stepLenFrac.
    // we want to scale this back into the range 0-1 and apply easing
    const float lenFrac = getEasing(p->easing, totLenFrac / stepLenFrac);


    // and we scale the difference between the points by this value
    const float dx = (p->pathX[pathStep+1] - p->pathX[pathStep]) * lenFrac;
    const float dy = (p->pathY[pathStep+1] - p->pathY[pathStep]) * lenFrac;

    p->x = p->pathX[pathStep] + dx;
    p->y = p->pathY[pathStep] + dy;

  } else {

    pd->system->error("Error pegDoUpdate called with unknown peg motion");

  }

  const cpVect pos = cpBodyGetPosition(p->cpBody);
  cpBodySetVelocity(p->cpBody, cpv((p->x - pos.x)/TIMESTEP, (p->y - pos.y)/TIMESTEP));
  // cpBodySetPosition(p->cpBody, cpv(p->x, p->y));
  pegSetBitmapCoordinates(p);
  if (p->y < p->minY) { p->minY = p->y; }
}

void pegSetMotionSpeed(struct Peg_t* p, const float s) { p->speed = s; }

void pegSetMotionEasing(struct Peg_t* p, const enum EasingFunction_t e) { p->easing = e; }

void pegSetMotionOffset(struct Peg_t* p, const float offset) { p->time = offset; }

void emptyCpBodyVelocityFunc(cpBody* body, cpVect gravity, cpFloat damping, cpFloat dt) {}

void emptyCpBodyPositionFunc(cpBody* body, cpFloat dt) {};

void pegSetMotionStatic(struct Peg_t* p) {
  cpBodySetVelocityUpdateFunc(p->cpBody, emptyCpBodyVelocityFunc);
  cpBodySetPositionUpdateFunc(p->cpBody, emptyCpBodyPositionFunc);
}

void pegSetMotionDoArcAngle(struct Peg_t* p, const bool doArcAngle) {
  p->doArcAngle = doArcAngle;
}

void pegSetMotionEllipse(struct Peg_t* p, const float a, const float b) {
  p->a = a;
  p->b = b;
  p->motion = kPegMotionEllipse;
}

void pegAddMotionPath(struct Peg_t* p, const int16_t x, const int16_t y) {
  if (p->pathSteps == MAX_LINEAR_PATH_SEGMENTS) {
    pd->system->error("Error pegAddMotionPath has reached the max of %i paths", MAX_LINEAR_PATH_SEGMENTS);
    return;
  }
  p->pathX[p->pathSteps] = x;
  p->pathY[p->pathSteps] = y;
  if (p->pathSteps) {
    const uint8_t cur = p->pathSteps;
    const uint8_t prv = cur - 1;
    p->pathLength[prv] = len(p->pathX[cur], p->pathX[prv], p->pathY[cur], p->pathY[prv]);
    p->totPathLength += p->pathLength[prv];
  }
  p->pathSteps++;
}

void pegDoMotionPathFinalise(struct Peg_t* p) {
  // Close the loop
  pegAddMotionPath(p, p->pathX[0], p->pathY[0]);
  p->motion = kPegMotionPath;
}

void pegDoHit(struct Peg_t* p) {
  if (p->state == kPegStateActive && FSMGetBallInPlay()) {
    p->state = kPegStateHit;
    FSMDoResetBallStuckCounter();
    if (p->type == kPegTypeRequired) {
      renderAddTrauma(TRAMA_REQUIRED_HIT);
      renderAddFreeze(FREEZE_REQUIRED_HIT);
      boardDoRequiredPegHit();
    } else {
      renderAddTrauma(TRAMA_PEG_HIT);
      renderAddFreeze(FREEZE_PEG_HIT);
    }
    const enum PegSpecial_t special = boardGetCurrentSpecial();
    if (special == kPegSpecialMultiball && !physicsGetSecondBallInPlay()) {
      physicsSetSecondBallInPlay();
    } else if (special == kPegSpecialBurst) {
      boardDoClearSpecial(); // Do this first, it's going to recurse!
      boardDoSpecialBurst();
    }
  }
  // pd->system->logToConsole("bam!");
}

bool pegDoCheckBurst(struct Peg_t* p, const float y) {
  if (p->state == kPegStateHit && p->y >= y) {
    p->state = kPegStateRemoved;
    pegDoRemove(p);
    return true;
  }
  return false;
}