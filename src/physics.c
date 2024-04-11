#include <stdio.h>
#include <stdlib.h>
#include "pdxalloc.h"
#include "physics.h"
#include "input.h"
#include "render.h"
#include "ui.h"
#include "peg.h"
#include "board.h"

cpSpace* m_space;

cpBody* m_ball[2];
cpShape* m_ballShape[2];

bool m_secondBallInPlay = false;

cpCollisionHandler* m_colliderHandle;

uint8_t cpCollisionBeginFunc_ballPeg(cpArbiter* arb, struct cpSpace* space, cpDataPointer data);

int16_t m_motionTrailX[2][MOTION_TRAIL_LEN];
int16_t m_motionTrailY[2][MOTION_TRAIL_LEN];

int16_t m_predictionTrailX[PREDICTION_TRACE_LEN];
int16_t m_predictionTrailY[PREDICTION_TRACE_LEN];


/// ///

uint8_t radToByte(float rad) { return (rad / M_2PIf) * 256.0f; }

uint8_t angToByte(float ang) { return (ang / 360.0f) * 256.0f; }

float angToRad(float ang) { return ang * (M_PIf / 180.0f); }

float len(const float x1, const float x2, const float y1, const float y2) {
  return sqrtf( len2(x1, x2, y1, y2) );
}

float len2(const float x1, const float x2, const float y1, const float y2) {
  return powf(x1 - x2, 2) + powf(y1 - y2, 2);
}

int16_t* motionTrailX(uint8_t n) { return m_motionTrailX[n]; }

int16_t* motionTrailY(uint8_t n) { return m_motionTrailY[n]; }

cpBody* getBall(uint8_t n) { return m_ball[n]; }

cpSpace* getSpace(void) { return m_space; }

void launchBall(float strength) {
  const float angleRad = angToRad(getTurretBarrelAngle());
  cpBodyApplyImpulseAtLocalPoint(m_ball[0], cpv(POOT_STRENGTH * sinf(angleRad) * strength, POOT_STRENGTH * -cosf(angleRad) * strength), cpvzero);
  if (getCurrentSpecial() == kPegSpecialBounce) {
    cpShapeSetElasticity(m_ballShape[0], ULTRA_BOUNCE);
  }

}

void initSpace(void) {
  m_space = cpSpaceNew();
  cpSpaceSetIterations(m_space, 10);
  cpSpaceSetGravity(m_space, G);
  cpSpaceSetSleepTimeThreshold(m_space, 0.5f);
  cpSpaceSetCollisionSlop(m_space, 0.5f); 

  // Ball
  const float moment = cpMomentForCircle(BALL_MASS, 0.0f, BALL_RADIUS, cpvzero);
  for (int i = 0; i < 2; ++i) {
    m_ball[i] = cpBodyNew(BALL_MASS, moment);
    cpBodySetPosition(m_ball[i], cpv(0, WFALL_PIX_Y*2));
    m_ballShape[i] = cpCircleShapeNew(m_ball[i], BALL_RADIUS, cpvzero);
    cpShapeSetFriction(m_ballShape[i], 0.0f);
    cpShapeSetElasticity(m_ballShape[i], ELASTICITY);
    cpShapeSetCollisionType(m_ballShape[i], FLAG_BALL);
  }
  cpSpaceAddShape(m_space, m_ballShape[0]);
  cpSpaceAddBody(m_space, m_ball[0]);
  resetBall(0);

  cpBody* walls = cpSpaceGetStaticBody(m_space);
  cpShape* top   = cpSegmentShapeNew(walls, cpv(0,            UI_OFFSET_TOP), cpv(DEVICE_PIX_X, UI_OFFSET_TOP),  3.0f);
  cpShape* left  = cpSegmentShapeNew(walls, cpv(0,            UI_OFFSET_TOP), cpv(0,            PHYSWALL_PIX_Y), 3.0f);
  cpShape* right = cpSegmentShapeNew(walls, cpv(DEVICE_PIX_X, UI_OFFSET_TOP), cpv(DEVICE_PIX_X, PHYSWALL_PIX_Y), 3.0f);
  cpShapeSetCollisionType(top, FLAG_WALL);
  cpShapeSetCollisionType(left, FLAG_WALL);
  cpShapeSetCollisionType(right, FLAG_WALL);
  cpShapeSetFriction(top, 0.0f);
  cpShapeSetElasticity(top, ELASTICITY);
  cpShapeSetFriction(left, 0.0f);
  cpShapeSetElasticity(left, ELASTICITY);
  cpShapeSetFriction(right, 0.0f);
  cpShapeSetElasticity(right, ELASTICITY);
  cpSpaceAddShape(m_space, top);
  cpSpaceAddShape(m_space, left);
  cpSpaceAddShape(m_space, right);

  m_colliderHandle = cpSpaceAddCollisionHandler(m_space, FLAG_BALL, FLAG_PEG);
  m_colliderHandle->beginFunc = cpCollisionBeginFunc_ballPeg;
}

void addSecondBall(void) {
  cpSpaceAddShape(m_space, m_ballShape[1]);
  cpSpaceAddBody(m_space, m_ball[1]);
  resetBall(1);
  m_secondBallInPlay = false;
}

void removeSecondBall(void) {
  cpSpaceRemoveShape(m_space, m_ballShape[1]);
  cpSpaceRemoveBody(m_space, m_ball[1]);
}

void setSecondBallInPlay(void) {
  m_secondBallInPlay = true;
  const cpVect pos = cpBodyGetPosition(m_ball[0]);
  const cpVect vel = cpBodyGetVelocity(m_ball[0]);
  cpBodySetPosition(m_ball[1], pos);
  cpBodySetVelocity(m_ball[1], cpv(vel.y, vel.x));
}

bool getSecondBallInPlay(void) {
  return m_secondBallInPlay;
}

uint8_t cpCollisionBeginFunc_ballPeg(cpArbiter* arb, struct cpSpace* space, cpDataPointer data) {
  cpBody* cpBall;
  cpBody* cpPeg;
  cpArbiterGetBodies(arb, &cpBall, &cpPeg);
  struct Peg_t* p = (struct Peg_t*) cpBodyGetUserData(cpPeg);
  hitPeg(p);
  return 1;
}

void secondTryBall(void) {
  const cpVect pos = cpBodyGetPosition(m_ball[0]);
  cpBodySetPosition(m_ball[0], cpv(pos.x, getMinimumY() + BALL_RADIUS));
}

void resetBall(uint8_t n) {
  int16_t yMod = (n == 1 ? 1024 : 0);
  cpBodySetPosition(m_ball[n], cpv(HALF_DEVICE_PIX_X, getMinimumY() + TURRET_RADIUS - yMod));
  cpBodySetVelocity(m_ball[n], cpvzero);
  cpBodySetAngle(m_ball[n], 0);
  cpBodySetAngularVelocity(m_ball[n], 0);
  cpShapeSetElasticity(m_ballShape[n], ELASTICITY);
}

void updateSpace(int32_t fc, enum kFSM fsm) {
  cpSpaceStep(m_space, TIMESTEP);
  
  const cpVect pos = cpBodyGetPosition(m_ball[0]);

  if (fsm == kGameFSM_AimMode) {
    const int i = (getCurrentSpecial() == kPegSpecialAim ? fc % (PREDICTION_TRACE_LEN*2) : fc % PREDICTION_TRACE_LEN);
    if (i == 0) {
      resetBall(0);
      launchBall(1.0f);
    }
    setBallTrace(i, pos.x, pos.y);
  }

  m_motionTrailX[0][fc % MOTION_TRAIL_LEN] = pos.x;
  m_motionTrailY[0][fc % MOTION_TRAIL_LEN] = pos.y;

  if (getCurrentSpecial() == kPegSpecialAim) {
    const cpVect pos2 = cpBodyGetPosition(m_ball[1]);
    m_motionTrailX[1][fc % MOTION_TRAIL_LEN] = pos2.x;
    m_motionTrailY[1][fc % MOTION_TRAIL_LEN] = pos2.y;

    if (!m_secondBallInPlay) { resetBall(1); }
  }

}
