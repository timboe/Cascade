#include <stdio.h>
#include <stdlib.h>
#include "pdxalloc.h"
#include "physics.h"
#include "input.h"
#include "render.h"
#include "fsm.h"
#include "peg.h"
#include "board.h"
#include "util.h"

cpSpace* m_space;

cpBody* m_ball[2];
cpShape* m_ballShape[2];

bool m_secondBallInPlay = false;

cpCollisionHandler* m_colliderHandle;

float m_timestep = TIMESTEP;

uint8_t cpCollisionBeginFunc_ballPeg(cpArbiter* arb, struct cpSpace* space, cpDataPointer data);

int16_t m_physicsGetMotionTrailX[2][MOTION_TRAIL_LEN];
int16_t m_physicsGetMotionTrailY[2][MOTION_TRAIL_LEN];

int16_t m_predictionTrailX[PREDICTION_TRACE_LEN];
int16_t m_predictionTrailY[PREDICTION_TRACE_LEN];


/// ///

void physicsSetTimestepMultiplier(const float tsm) { m_timestep = TIMESTEP * tsm; }
float physicsGetTimestepMultiplier(void) { return m_timestep; }

int16_t* physicsGetMotionTrailX(const uint8_t n) { return m_physicsGetMotionTrailX[n]; }
int16_t* physicsGetMotionTrailY(const uint8_t n) { return m_physicsGetMotionTrailY[n]; }

cpBody* physicsGetBall(const uint8_t n) { return m_ball[n]; }

cpSpace* physicsGetSpace(void) { return m_space; }

void physicsDoLaunchBall(const float strength) {
  const float angleRad = degToRad(gameGetTurretBarrelAngle());
  cpBodyApplyImpulseAtLocalPoint(m_ball[0], cpv(POOT_STRENGTH * sinf(angleRad) * strength, POOT_STRENGTH * -cosf(angleRad) * strength), cpvzero);
  if (boardGetCurrentSpecial() == kPegSpecialBounce) {
    cpShapeSetElasticity(m_ballShape[0], ULTRA_BOUNCE);
  }
}

void physicsDoInit(void) {
  m_space = cpSpaceNew();
  cpSpaceSetIterations(m_space, 10);
  cpSpaceSetGravity(m_space, G);
  cpSpaceSetSleepTimeThreshold(m_space, 0.5f);
  cpSpaceSetCollisionSlop(m_space, 0.5f); 

  // Ball
  const float moment = cpMomentForCircle(BALL_MASS, 0.0f, BALL_RADIUS, cpvzero);
  for (int i = 0; i < 2; ++i) {
    m_ball[i] = cpBodyNew(BALL_MASS, moment);
    cpBodySetPosition(m_ball[i], cpv(0, WF_PIX_Y*2));
    m_ballShape[i] = cpCircleShapeNew(m_ball[i], BALL_RADIUS, cpvzero);
    cpShapeSetFriction(m_ballShape[i], 0.0f);
    cpShapeSetElasticity(m_ballShape[i], ELASTICITY);
    cpShapeSetCollisionType(m_ballShape[i], FLAG_BALL);
  }
  cpSpaceAddShape(m_space, m_ballShape[0]);
  cpSpaceAddBody(m_space, m_ball[0]);
  physicsDoResetBall(0);

  cpBody* walls = cpSpaceGetStaticBody(m_space);
  cpShape* top   = cpSegmentShapeNew(walls, cpv(0,            0), cpv(DEVICE_PIX_X, 0),  3.0f);
  cpShape* left  = cpSegmentShapeNew(walls, cpv(0,            0), cpv(0,            PHYSWALL_PIX_Y), 3.0f);
  cpShape* right = cpSegmentShapeNew(walls, cpv(DEVICE_PIX_X, 0), cpv(DEVICE_PIX_X, PHYSWALL_PIX_Y), 3.0f);
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

void physicsDoAddSecondBall(void) {
  cpSpaceAddShape(m_space, m_ballShape[1]);
  cpSpaceAddBody(m_space, m_ball[1]);
  physicsDoResetBall(1);
  m_secondBallInPlay = false;
}

void physicsDoRemoveSecondBall(void) {
  cpSpaceRemoveShape(m_space, m_ballShape[1]);
  cpSpaceRemoveBody(m_space, m_ball[1]);
}

void physicsSetSecondBallInPlay(void) {
  m_secondBallInPlay = true;
  const cpVect pos = cpBodyGetPosition(m_ball[0]);
  const cpVect vel = cpBodyGetVelocity(m_ball[0]);
  cpBodySetPosition(m_ball[1], pos);
  cpBodySetVelocity(m_ball[1], cpv(vel.y, vel.x));
}

bool physicsGetSecondBallInPlay(void) {
  return m_secondBallInPlay;
}

uint8_t cpCollisionBeginFunc_ballPeg(cpArbiter* arb, struct cpSpace* space, cpDataPointer data) {
  cpBody* cpBall;
  cpBody* cpPeg;
  cpArbiterGetBodies(arb, &cpBall, &cpPeg);
  struct Peg_t* p = (struct Peg_t*) cpBodyGetUserData(cpPeg);
  pegDoHit(p);
  if (boardGetCurrentSpecial() == kPegSpecialPenetrate) {
    return false;
  }
  return true;
}

void physicsDoSecondTryBall(void) {
  const cpVect pos = cpBodyGetPosition(m_ball[0]);
  cpBodySetPosition(m_ball[0], cpv(pos.x, gameGetMinimumY() + BALL_RADIUS));
}

void physicsDoResetBall(uint8_t n) {
  int16_t yMod = (n == 1 ? 1024 : 0);
  cpBodySetPosition(m_ball[n], cpv(HALF_DEVICE_PIX_X, gameGetMinimumY() + TURRET_RADIUS - yMod));
  cpBodySetVelocity(m_ball[n], cpvzero);
  cpBodySetAngle(m_ball[n], 0);
  cpBodySetAngularVelocity(m_ball[n], 0);
  cpShapeSetElasticity(m_ballShape[n], ELASTICITY);
}

void physicsDoUpdate(const int32_t fc, const enum FSM_t fsm) {
  cpSpaceStep(m_space, m_timestep);
  
  const cpVect pos = cpBodyGetPosition(m_ball[0]);

  if (fsm == kGameFSM_AimMode) {
    const int i = (boardGetCurrentSpecial() == kPegSpecialAim ? fc % (PREDICTION_TRACE_LEN*2) : fc % PREDICTION_TRACE_LEN);
    if (i == 0) {
      physicsDoResetBall(0);
      physicsDoLaunchBall(1.0f);
    }
    renderSetBallTrace(i, pos.x, pos.y);
  }

  m_physicsGetMotionTrailX[0][fc % MOTION_TRAIL_LEN] = pos.x;
  m_physicsGetMotionTrailY[0][fc % MOTION_TRAIL_LEN] = pos.y;

  if (boardGetCurrentSpecial() == kPegSpecialAim) {
    const cpVect pos2 = cpBodyGetPosition(m_ball[1]);
    m_physicsGetMotionTrailX[1][fc % MOTION_TRAIL_LEN] = pos2.x;
    m_physicsGetMotionTrailY[1][fc % MOTION_TRAIL_LEN] = pos2.y;

    if (!m_secondBallInPlay) { physicsDoResetBall(1); }
  }

}
