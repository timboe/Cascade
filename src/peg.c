#include "peg.h"
#include "bitmap.h"
#include "render.h"
#include "board.h"
#include "sshot.h"
#include "util.h"
#include "io.h"

void pegDoUpdateAngle(struct Peg_t* p, const float angle);

void pegSetBitmapCoordinates(struct Peg_t* p);

void pegDoAddBody(struct Peg_t* p);

void pegDoAddedShape(struct Peg_t* p);

///   

void pegDoAddBody(struct Peg_t* p) {
  p->cpBody = cpBodyNewKinematic();
  cpBodySetPosition(p->cpBody, cpv(p->x, p->y));
  cpBodySetAngle(p->cpBody, p->a);
  cpBodySetUserData(p->cpBody, (void*)p);
  cpSpaceAddBody(physicsGetSpace(), p->cpBody);
}

void pegDoAddedShape(struct Peg_t* p) {
  p->bitmap = bitmapGetPeg(p); // Call after setting shape, iAngle, size and tyoe
  pegSetBitmapCoordinates(p);
  cpShapeSetCollisionType(p->cpShape, FLAG_PEG);
  cpShapeSetFriction(p->cpShape, FRICTION);
  cpShapeSetElasticity(p->cpShape, ELASTICITY);
  cpSpaceAddShape(physicsGetSpace(), p->cpShape);
}

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
  p->queueRemove = false;
  p->popAnim = rand() % MAX_POPS;
  p->popFrame = -1;
  const float scale = bitmapSizeToScale(size);

  pegDoAddBody(p);
  if (s == kPegShapeBall) {
    p->cpShape = cpCircleShapeNew(p->cpBody, BALL_RADIUS*scale, cpvzero);
    p->radius = BALL_RADIUS*scale;
  } else if (s == kPegShapeRect) {
    p->cpShape = cpBoxShapeNew(p->cpBody, BOX_HALF_WIDTH*scale, BOX_HALF_HEIGHT*scale, 0.0f);
    p->radius = BOX_HALF_MAX*scale;
  } else if (s == kPegShapeTri) {
    cpVect verts[3];
    const float angleAdvance = M_2PIf / 3.0f;
    const float scale = bitmapSizeToScale(p->size);
    for (uint8_t p = 0; p < 3; ++p) {
      const float angle = (angleAdvance * p) - degToRad(90);
      verts[p].x = (TRI_WIDTH/2) * scale * sinf(angle);
      verts[p].y = (TRI_WIDTH/2) * scale * cosf(angle);
    }
    p->cpShape = cpPolyShapeNew(p->cpBody, 3, verts, cpTransformIdentity, 0.0f);
    p->radius = TRI_MAX*scale;
  } else {    
    pd->system->error("Error pegDoInit called with unknown peg shape");
    pegDoClear(p);
    return;
  }
  pegDoAddedShape(p);
  pegDoUpdateAngle(p, p->angle);

  pegAddMotionPath(p, x, y); // Motion path location 0 always holds the origin coordinate
  p->pathCurrent = 1; // This then pre-assumes pegAddMotionPath will be called >0 times for kPegMotionPath
}

void pegDoClear(struct Peg_t* p) {
  pegDoRemove(p);
  memset(p, 0, sizeof(struct Peg_t));
}

void pegDoRemove(struct Peg_t* p) {
  p->queueRemove = false;
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

void pegSetType(struct Peg_t* p, const enum PegType_t type) {
  if (type == kPegTypeNormal) { return; }
  p->type = type;
  if (type == kPegTypeSpecial) {
    pegDoRemove(p); // Update physics object
    pegDoAddBody(p);
    p->shape = kPegShapeHex;
    cpVect verts[6];
    const float angleAdvance = M_2PIf / 6.0f;
    const float scale = bitmapSizeToScale(p->size);
    for (uint8_t p = 0; p < 6; ++p) {
      const float angle = (angleAdvance * p) + degToRad(30);
      verts[p].x = (HEX_WIDTH/2) * scale * sinf(angle);
      verts[p].y = (HEX_WIDTH/2) * scale * cosf(angle);
    }
    p->cpShape = cpPolyShapeNew(p->cpBody, 6, verts, cpTransformIdentity, 0.0f);
    p->radius = HEX_MAX*scale;
    pegDoAddedShape(p);
  } else {
    p->bitmap = bitmapGetPeg(p);
  }
}

void pegSetBitmapCoordinates(struct Peg_t* p) {
  p->xBitmap = p->x - p->radius;
  p->yBitmap = p->y - p->radius;
}

void pegDoUpdateAngle(struct Peg_t* p, const float angle) {
  // Note we do NOT update the original p->angle, this is a constant for the peg
  if (p->shape == kPegShapeBall) {
    return; // Time saving, the angle is irrelevent for circles
  }
  p->iAngle = radToByte(angle);
  cpBodySetAngle(p->cpBody, angle);
  p->bitmap = bitmapGetPeg(p);
}

void pegDoUpdate(struct Peg_t* p) {
  if (p->queueRemove) {
    p->state = kPegStateRemoved;
    pegDoRemove(p);
  }
  if (p->state == kPegStateRemoved) {
    return;
  }

  const float tsm = physicsGetTimestepMultiplier();
  p->time += (TIMESTEP * tsm * p->speed);
  if (p->time >= M_2PIf) { p->time -= M_2PIf; } // Keep this one bounded
  else if (p->time < -M_2PIf) { p->time += M_2PIf; } // Speed might be -ve

  if (p->motion == kPegMotionStatic) {

    return; // noop - and we can skip below too

  } else if (p->motion == kPegMotionEllipse) {

    const float easing = getEasing(p->easing, p->time / M_2PIf) * M_2PIf;
    p->x = p->pathX[0] + (p->a * cosf(easing - p->angle));
    p->y = p->pathY[0] + (p->b * sinf(easing - p->angle));
    if (p->doArcAngle) {
      pegDoUpdateAngle(p, easing + p->angle );
    }

  } else if (p->motion == kPegMotionPath) {

    uint8_t pathStep = 0;
    float stepLenFrac = 0;
    float totLenFrac = (p->time / M_2PIf);
    
    // TODO - this isn't great, no easing
    if (p->doArcAngle) {
      pegDoUpdateAngle(p, (totLenFrac * M_2PIf) + p->angle );
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
  if (p->speed == 0.0f) {
    cpBodySetPosition(p->cpBody, cpv(p->x, p->y));
    cpBodySetVelocity(p->cpBody, cpvzero);
    // No need for more updates
    p->motion = kPegMotionStatic;
  } else {
    cpBodySetVelocity(p->cpBody, cpv((p->x - pos.x)/TIMESTEP, (p->y - pos.y)/TIMESTEP));
  }
  pegSetBitmapCoordinates(p);
  
  if (p->y < p->minY) { p->minY = p->y; }

  // Removes collisions?
  // if (p->speed == 0.0f) {
  //   // No need for more updates
  //   p->motion = kPegMotionStatic;
  // }
}

void pegSetMotionSpeed(struct Peg_t* p, const float s) { p->speed = s; }

void pegSetMotionEasing(struct Peg_t* p, const enum EasingFunction_t e) { p->easing = e; }

void pegSetMotionOffset(struct Peg_t* p, const float offset) { p->time = offset; }

void emptyCpBodyVelocityFunc(cpBody* body, cpVect gravity, cpFloat damping, cpFloat dt) {}

void emptyCpBodyPositionFunc(cpBody* body, cpFloat dt) {};

void pegSetMotionStatic(struct Peg_t* p) {
  if (!p->cpBody) pd->system->error("Error called pegSetMotionStatic on a peg with no body");
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
  if (p->state == kPegStateActive 
    && FSMGetBallInPlay()
    && p->y < IOGetCurrentHoleHeight()
    && p->y > 0
  ) {
    p->state = kPegStateHit;
    FSMDoResetBallStuckCounter();
    if (p->type == kPegTypeRequired) {
      renderAddTrauma(TRAUMA_REQUIRED_HIT);
      renderAddFreeze(FREEZE_REQUIRED_HIT);
      boardDoRequiredPegHit();
    } else if (p->type == kPegTypeSpecial) {
      renderAddTrauma(TRAUMA_SPECIAL_HIT);
      renderAddFreeze(FREEZE_SPECIAL_HIT);
      const enum PegSpecial_t special = boardDoAddSpecial(false); // activate = false
      renderDoAddSpecial(p->cpBody, special);
    } else {
      renderAddTrauma(TRAUMA_PEG_HIT);
      renderAddFreeze(FREEZE_PEG_HIT);
    }
    //
    const enum PegSpecial_t special = boardGetCurrentSpecial();
    if (special == kPegSpecialMultiball && !physicsGetSecondBallInPlay()) {
      physicsSetSecondBallInPlay();
    } else if (special == kPegSpecialBlast) {
      // pd->system->logToConsole("BLAST");
      boardDoClearSpecial(); // Do this first, it's going to recurse!
      boardDoSpecialBlast();
      renderAddTrauma(TRAUMA_BLAST_HIT);
      renderDoAddBlast(p->cpBody);
    }
  }
  if (p->state == kPegStateHit && FSMGet() == kGameFSM_WinningToast) {
    p->queueRemove = true; // Can't remove in the middle of a physics callback
    p->popFrame = 0;
  }
  // pd->system->logToConsole("bam!");
}

bool pegDoCheckBurst(struct Peg_t* p, const float y) {
  if (p->state == kPegStateHit) {
    if (p->popFrame == -1 && p->y >= y - POP_EARLY_Y) {
      p->popFrame = 0;
    }
    if (p->y >= y) {
      p->state = kPegStateRemoved;
      pegDoRemove(p);
      return true;
    }
  }
  return false;
}

const char* pegGetSpecialTxt(const uint8_t s) {
  switch (s) {
    case kPegSpecialAim: return "AIMSHOT";
    case kPegSpecialSecondTry: return "SECONDSHOT";
    case kPegSpecialBlast: return "BLASTBALL";
    case kPegSpecialMultiball: return "MULTIBALL";
    case kPegSpecialBounce: return "ELASTIBALL";
    case kPegSpecialPenetrate: return "GHOSTBALL";
    default: break;
  }
  return "";
}
