#include "peg.h"
#include "bitmap.h"
#include "render.h"
#include "board.h"
#include "sshot.h"
#include "util.h"
#include "io.h"
#include "sound.h"

void pegDoUpdateAngle(struct Peg_t* p, const float angle);

void pegSetBitmapCoordinates(struct Peg_t* p);

void pegDoAddBody(struct Peg_t* p);

void pegDoAddedShape(struct Peg_t* p);

///   

void pegDoAddBody(struct Peg_t* p) {
  p->cpBody = cpBodyNewKinematic();
  // pd->system->logToConsole("DBG do ADD-BODY           for peg %i with cpBody %i. This is shape:%i, motion:%i, type:%i, state:%i", 
  //   p->id, p->cpBody,
  //   p->shape, p->motion, p->type, p->state);
  cpBodySetPosition(p->cpBody, cpv(p->x, p->y));
  cpBodySetAngle(p->cpBody, p->a);
  cpBodySetUserData(p->cpBody, (void*)p);
  cpSpaceAddBody(physicsGetSpace(), p->cpBody);
}

void pegDoAddedShape(struct Peg_t* p) {
  p->bitmap = bitmapGetPeg(p); // Call after setting shape, iAngle, size and tyoe
  pegSetBitmapCoordinates(p);
  cpShapeSetCollisionType(p->cpShape, FLAG_PEG);
  cpShapeSetFilter(p->cpShape, FILTER_PEG);
  cpShapeSetFriction(p->cpShape, FRICTION);
  cpShapeSetElasticity(p->cpShape, ELASTICITY);
  cpSpaceAddShape(physicsGetSpace(), p->cpShape);
  // pd->system->logToConsole("DBG do ADD-SHAPE-TO-SPACE for peg %i with cpBody %i and cpshape %i. This is shape:%i, motion:%i, type:%i, state:%i", 
  //   p->id, p->cpBody, p->cpShape,
  //   p->shape, p->motion, p->type, p->state);
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
    // pd->system->logToConsole("DBG do ADD-SHAPE          for peg %i with cpBody %i and cpShape %i (circle). This is shape:%i, motion:%i, type:%i, state:%i", 
    //   p->id, p->cpBody, p->cpShape,
    //   p->shape, p->motion, p->type, p->state);
    p->radius = BALL_RADIUS*scale;
  } else if (s == kPegShapeRect) {
    p->cpShape = cpBoxShapeNew(p->cpBody, BOX_HWIDTH*scale, BOX_HALF_HEIGHT*scale, 0.0f);
    // pd->system->logToConsole("DBG do ADD-SHAPE          for peg %i with cpBody %i and cpShape %i (box). This is shape:%i, motion:%i, type:%i, state:%i", 
    //   p->id, p->cpBody, p->cpShape,
    //   p->shape, p->motion, p->type, p->state);
    p->radius = BOX_HALF_MAX*scale;
  } else if (s == kPegShapeTri) {
    cpVect verts[3];
    const float angleAdvance = M_2PIf / 3.0f;
    const float scale = bitmapSizeToScale(p->size);
    for (uint8_t p = 0; p < 3; ++p) {
      const float angle = (angleAdvance * p) - degToRad(180);
      verts[p].x = (TRI_WIDTH/2) * scale * sinf(angle);
      verts[p].y = (TRI_WIDTH/2) * scale * cosf(angle);
    }
    p->cpShape = cpPolyShapeNew(p->cpBody, 3, verts, cpTransformIdentity, 0.0f);
    // pd->system->logToConsole("DBG do ADD-SHAPE          for peg %i with cpBody %i and cpShape %i (poly). This is shape:%i, motion:%i, type:%i, state:%i", 
    //   p->id, p->cpBody, p->cpShape,
    //   p->shape, p->motion, p->type, p->state);
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
  const uint16_t id = p->id; // Remember this
  pegDoRemove(p);
  memset(p, 0, sizeof(struct Peg_t));
  p->id = id;
}

void pegDoRemove(struct Peg_t* p) {
  p->queueRemove = false;
  // pd->system->logToConsole("DBG do REMOVE             for peg %i with cpBody %i and cpShape %i. This is shape:%i, motion:%i, type:%i, state:%i", 
    // p->id, p->cpBody, p->cpShape,
    // p->shape, p->motion, p->type, p->state);
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
  // pd->system->logToConsole("DBG done remove");
}

void pegSetType(struct Peg_t* p, const enum PegType_t type) {
  if (type == kPegTypeNormal) { return; }
  p->type = type; // TODO - before the return?
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
    // pd->system->logToConsole("DBG do ADD-SHAPE (spesh)  for peg %i with cpBody %i and cpShape %i (poly). This is shape:%i, motion:%i, type:%i, state:%i", 
    //   p->id, p->cpBody, p->cpShape,
    //   p->shape, p->motion, p->type, p->state);
    p->radius = HEX_MAX*scale;
    pegDoAddedShape(p);
  } else { // Required
    p->bitmap = bitmapGetPeg(p);
  }
}

void pegSetBitmapCoordinates(struct Peg_t* p) {
  p->xBitmap = p->x - p->radius;
  p->yBitmap = p->y - p->radius;
}

float bound(const float f) {
  if (f >= M_2PIf) { return f - M_2PIf; }
  else if (f < 0) { return f + M_2PIf; }
  return f;
}

void pegDoUpdateAngle(struct Peg_t* p, const float angle) {
  // Note we do NOT update the original p->angle, this is a constant for the peg
  if (p->shape == kPegShapeBall) {
    return; // Time saving, the angle is irrelevent for circles
  }
  const float bAngle = bound(angle);
  // pd->system->logToConsole("PDUA p=%i, A=%f, bA=%f", p->id, angle, bAngle);
  p->iAngle = radToByte(bAngle);
  cpBodySetAngle(p->cpBody, bAngle);
  p->bitmap = bitmapGetPeg(p);
}

void pegDoUpdate(struct Peg_t* p, const float timeStep) {
  if (p->queueRemove) {
    p->state = kPegStateRemoved;
    pegDoRemove(p);
  }
  if (p->state == kPegStateRemoved) {
    return;
  }

  p->time = bound(p->time + (timeStep * p->speed)); // Remember speed can be -ve

  if (p->motion == kPegMotionStatic) {

    return; // noop - and we can skip below too

  } else if (p->motion == kPegMotionEllipse) {

    // Think this is broken, especially the angle bit. TODO - fix easing for ellipse?
    // const float easing = getEasing(p->easing, p->time / M_2PIf) * M_2PIf;
    // p->x = p->pathX[0] + (p->a * cosf(easing));
    // p->y = p->pathY[0] + (p->b * sinf(easing));
    // if (p->doArcAngle) {
    //   pegDoUpdateAngle(p, easing + p->angle );
    // }
    p->x = p->pathX[0] + (p->a * cosf(p->time));
    p->y = p->pathY[0] + (p->b * sinf(p->time));
    if (p->doArcAngle) { pegDoUpdateAngle(p, p->time + p->angle ); }


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

  if (p->y < p->minY && p->x > 0 && p->x < DEVICE_PIX_X && p->y > 0) { p->minY = p->y; }

  const cpVect pos = cpBodyGetPosition(p->cpBody);
  if (p->speed == 0.0f) {
    cpBodySetPosition(p->cpBody, cpv(p->x, p->y));
    cpBodySetVelocity(p->cpBody, cpvzero);
    // No need for more updates
    p->motion = kPegMotionStatic;
    pegSetMotionStatic(p);
  } else {
    cpBodySetVelocity(p->cpBody, cpv((p->x - pos.x)/TIMESTEP, (p->y - pos.y)/TIMESTEP));
  }
  pegSetBitmapCoordinates(p);
  

  // Removes collisions?
  // if (p->speed == 0.0f) {
  //   // No need for more updates
  //   p->motion = kPegMotionStatic;
  // }
}

void pegSetMotionSpeed(struct Peg_t* p, const float s) { 
  p->speed = s;
  p->minY = 10000.0; // Force recomputation
}

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
    // pd->system->logToConsole("     cur=%i, prev len:%f, tot len:%f", cur, p->pathLength[prv], p->totPathLength);
  }
  p->pathSteps++;
}

void pegDoMotionPathFinalise(struct Peg_t* p) {
  // Close the loop
  pegAddMotionPath(p, p->pathX[0], p->pathY[0]);
  p->motion = kPegMotionPath;
}

void pegDoHit(struct Peg_t* p) {
  const bool winToast = (FSMGet() == kGameFSM_WinningToastA || FSMGet() == kGameFSM_WinningToastB);
  if (p->state == kPegStateActive 
    && FSMGetBallInPlay()
    && p->y < IOGetCurrentHoleHeight()
    && p->y > 0
  ) {
    p->state = kPegStateHit;
    boardDoPegHit();
    FSMDoResetBallStuckCounter();
    //
    const enum PegSpecial_t special = boardGetCurrentSpecial();
    if (special == kPegSpecialMultiball && !physicsGetSecondBallInPlay()) {
      physicsSetSecondBallInPlay();
      soundDoSfx(kSplitSfx);
    } else if (special == kPegSpecialBlast) {
      boardDoClearSpecial(); // Do this first, it's going to recurse when we start to hit pegs in the explosion radius!
      soundSetDoingExplosion(true); // Also do this first - we don't want to play the ping sfx from all of the hit pegs
      boardDoSpecialBlast();
      renderAddTrauma(TRAUMA_BLAST_HIT);
      renderDoAddSpecialBlast(p->cpBody);
      soundDoSfx(kPlingSfx1);
      soundDoSfx(kExplosionSfx);
    }
    //
    if (p->type == kPegTypeRequired) {
      renderAddTrauma(TRAUMA_REQUIRED_HIT);
      renderAddFreeze(FREEZE_REQUIRED_HIT);
      boardDoRequiredPegHit();
      if (!winToast) { soundDoSfx(kDingSfx1); }
    } else if (p->type == kPegTypeSpecial) {
      renderAddTrauma(TRAUMA_SPECIAL_HIT);
      renderAddFreeze(FREEZE_SPECIAL_HIT);
      const enum PegSpecial_t special = boardDoAddSpecial(/*activate = */false);
      renderDoAddSpecial(p->cpBody, special);
      if (!winToast) { 
        soundDoSfx(kDingSfx1);
        soundDoSfx(kDingSfx1);
      } 
    } else {
      renderAddTrauma(TRAUMA_PEG_HIT);
      renderAddFreeze(FREEZE_PEG_HIT);
      if (!winToast) { soundDoSfx(kPlingSfx1); }
    }
  }
  if (p->state == kPegStateHit && winToast) {
    p->queueRemove = true; // Can't remove in the middle of a physics callback
    p->popFrame = 0;
    soundDoSfx(kPopSfx1);
  }
}

bool pegDoCheckBurst(struct Peg_t* p, const float y) {
  if (p->state == kPegStateHit) {
    if (p->popFrame == -1 && p->y >= y - POP_EARLY_Y) {
      p->popFrame = 0;
      soundDoSfx(kPopSfx1);
      // pd->system->logToConsole("DBG do pop peg %i", p->id);
    }
    if (p->y >= y) {
      // pd->system->logToConsole("DBG do REMOVE peg %i with cpBody %i and cpShape %i. This is shape:%i, motion:%i, type:%i, state:%i", 
      //   p->id, p->cpBody, p->cpShape,
      //   p->shape, p->motion, p->type, p->state);
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
