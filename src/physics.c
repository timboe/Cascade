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
cpVect m_ballPosition[2];

bool m_secondBallInPlay = false;

cpCollisionHandler* m_colliderHandle;

float m_timestep = TIMESTEP;

uint8_t cpCollisionBeginFunc_ballPeg(cpArbiter* arb, struct cpSpace* space, cpDataPointer data);

int16_t m_physicsGetMotionTrailX[2][MOTION_TRAIL_LEN];
int16_t m_physicsGetMotionTrailY[2][MOTION_TRAIL_LEN];

int16_t m_predictionTrailX[PREDICTION_TRACE_LEN];
int16_t m_predictionTrailY[PREDICTION_TRACE_LEN];

struct cpShapeFilter FILTER_BALL;
struct cpShapeFilter FILTER_PEG;
struct cpShapeFilter FILTER_WALL;

void physicsSetBallPosition(const uint8_t ball, const cpVect position);

/// ///

void physicsSetTimestepMultiplier(const float tsm) { 
  m_timestep = TIMESTEP * tsm;
}
float physicsGetTimestepMultiplier(void) { return m_timestep / TIMESTEP; }

int16_t* physicsGetMotionTrailX(const uint8_t n) { return m_physicsGetMotionTrailX[n]; }
int16_t* physicsGetMotionTrailY(const uint8_t n) { return m_physicsGetMotionTrailY[n]; }

cpBody* physicsGetBall(const uint8_t n) { return m_ball[n]; }

cpVect physicsGetBallPosition(const uint8_t n) { return m_ballPosition[n]; }

cpSpace* physicsGetSpace(void) { return m_space; }

void physicsDoLaunchBall(const float strength) {
  const float angleRad = degToRad(gameGetTurretBarrelAngle());
  cpBodyApplyImpulseAtLocalPoint(m_ball[0], cpv(POOT_STRENGTH * sinf(angleRad) * strength, POOT_STRENGTH * -cosf(angleRad) * strength), cpvzero);
  if (boardGetCurrentSpecial() == kPegSpecialBounce) {
    cpShapeSetElasticity(m_ballShape[0], ULTRA_BOUNCE);
  }
}

void physicsDoInit(void) {
  FILTER_BALL.categories |= FLAG_BALL;
  FILTER_BALL.mask |= FLAG_BALL | FLAG_WALL | FLAG_PEG;

  FILTER_PEG.group = GROUP_FURNATURE;
  FILTER_PEG.categories |= FLAG_PEG;
  FILTER_PEG.mask |= FLAG_BALL;

  FILTER_WALL.group = GROUP_FURNATURE;
  FILTER_WALL.categories |= FLAG_WALL;
  FILTER_WALL.mask |= FLAG_BALL;

  m_space = cpSpaceNew();
  cpSpaceSetIterations(m_space, 10);
  cpSpaceSetGravity(m_space, G);
  cpSpaceSetSleepTimeThreshold(m_space, 0.5f);
  cpSpaceSetCollisionSlop(m_space, 0.5f); 

  // Ball
  const float moment = cpMomentForCircle(BALL_MASS, 0.0f, BALL_RADIUS, cpvzero);
  for (int i = 0; i < MAX_BALLS; ++i) {
    m_ball[i] = cpBodyNew(BALL_MASS, moment);
    physicsSetBallPosition(i, cpv(0, DEVICE_PIX_Y*10));
    m_ballShape[i] = cpCircleShapeNew(m_ball[i], BALL_RADIUS, cpvzero);
    cpShapeSetFriction(m_ballShape[i], 0.0f);
    cpShapeSetElasticity(m_ballShape[i], ELASTICITY);
    cpShapeSetCollisionType(m_ballShape[i], FLAG_BALL);
    cpShapeSetFilter(m_ballShape[i], FILTER_BALL);
  }
  cpSpaceAddShape(m_space, m_ballShape[0]);
  cpSpaceAddBody(m_space, m_ball[0]);
  physicsDoResetBall(0);

  cpBody* walls  = cpSpaceGetStaticBody(m_space);
  cpShape* top   = cpSegmentShapeNew(walls, cpv(0,            0), cpv(DEVICE_PIX_X, 0),  3.0f);
  cpShape* left  = cpSegmentShapeNew(walls, cpv(0,            0), cpv(0,            PHYSWALL_PIX_Y), 3.0f);
  cpShape* right = cpSegmentShapeNew(walls, cpv(DEVICE_PIX_X, 0), cpv(DEVICE_PIX_X, PHYSWALL_PIX_Y), 3.0f);
  cpShapeSetFriction(top, 0.0f);
  cpShapeSetElasticity(top, ELASTICITY);
  cpShapeSetFriction(left, 0.0f);
  cpShapeSetElasticity(left, ELASTICITY);
  cpShapeSetFriction(right, 0.0f);
  cpShapeSetElasticity(right, ELASTICITY);
  cpShapeSetFilter(top, FILTER_WALL);
  cpShapeSetFilter(left, FILTER_WALL);
  cpShapeSetFilter(right, FILTER_WALL);
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
  m_secondBallInPlay = false;
}

void physicsSetSecondBallInPlay(void) {
  m_secondBallInPlay = true;
  const cpVect pos = cpBodyGetPosition(m_ball[0]);
  cpVect vel = cpBodyGetVelocity(m_ball[0]);
  if (vel.x > 0.0f && vel.x <  10.0f) { vel.x =  10.0f; }
  if (vel.x < 0.0f && vel.x > -10.0f) { vel.x = -10.0f; }
  if (vel.y > 0.0f && vel.y <  10.0f) { vel.y =  10.0f; }
  if (vel.y < 0.0f && vel.y > -10.0f) { vel.y = -10.0f; }
  physicsSetBallPosition(1, pos);
  cpBodySetVelocity(m_ball[1], cpv(-vel.x, -vel.y));
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
  const cpVect vel = cpBodyGetVelocity(m_ball[0]);
  physicsSetBallPosition(0, cpv(pos.x, gameGetMinimumY() + BALL_RADIUS));
  cpBodySetVelocity(m_ball[0], cpv(vel.x, vel.y / 2.0f));
}

void physicsDoResetBall(uint8_t n) {
  int16_t yMod = (n == 1 ? 1024 : 0);
  physicsSetBallPosition(n, cpv(HALF_DEVICE_PIX_X, gameGetMinimumY() + TURRET_RADIUS - yMod));
  cpBodySetVelocity(m_ball[n], cpvzero);
  cpBodySetAngle(m_ball[n], 0);
  cpBodySetAngularVelocity(m_ball[n], 0);
  cpShapeSetElasticity(m_ballShape[n], ELASTICITY);
}

void physicsSetBallPosition(const uint8_t ball, const cpVect position) {
  cpBodySetPosition(m_ball[ball], position);
  m_ballPosition[ball] = position;
}

void physicsDoUpdate(const int32_t fc, const enum FSM_t fsm) {

  // Slow motion block
  static uint32_t frame = 0;
  if (fsm == kGameFSM_CloseUp || fsm == kGameFSM_WinningToastA) { // 25% if in closeup - keep a consistent timestep for phys. 50% for ToastA
    static cpVect ballPrev[2];
    static cpVect ballCur[2];
    const uint8_t nSteps = (fsm == kGameFSM_CloseUp ? 4 : 2);
    const uint8_t step = (frame++ % nSteps);
    // Run physics update only on 1/step frames
    if (!step) {
      ballPrev[0] = cpBodyGetPosition(m_ball[0]);
      ballPrev[1] = cpBodyGetPosition(m_ball[1]);
      cpSpaceStep(m_space, m_timestep);
      ballCur[0] = cpBodyGetPosition(m_ball[0]);
      ballCur[1] = cpBodyGetPosition(m_ball[1]);
    }
    // Apply linear interpolation on the other frames
    const float multiple = step / (float)nSteps;
    m_ballPosition[0].x = ballPrev[0].x + (ballCur[0].x - ballPrev[0].x) * multiple;
    m_ballPosition[0].y = ballPrev[0].y + (ballCur[0].y - ballPrev[0].y) * multiple;
    m_ballPosition[1].x = ballPrev[1].x + (ballCur[1].x - ballPrev[1].x) * multiple;
    m_ballPosition[1].y = ballPrev[1].y + (ballCur[1].y - ballPrev[1].y) * multiple;
  } else {
    // Regular - one physics update per frame
    cpSpaceStep(m_space, m_timestep);
    m_ballPosition[0] = cpBodyGetPosition(m_ball[0]);
    m_ballPosition[1] = cpBodyGetPosition(m_ball[1]);
    frame = 0;
  }


  const bool twoBalls = (boardGetCurrentSpecial() == kPegSpecialMultiball);
  
  cpVect pos[2];
  for (uint8_t b = 0; b < 2; ++b) {
    if (b == 1 && !twoBalls) { continue; }
    pos[b] = cpBodyGetPosition(m_ball[b]);
    if (pos[b].x < 4) { physicsSetBallPosition(b, cpv(4, pos[b].y)); }
    else if (pos[b].x > DEVICE_PIX_X - 4) { physicsSetBallPosition(b, cpv(DEVICE_PIX_X - 4, pos[b].y)); }
  }

  if (FSMGetIsAimMode()) {
    const int i = (boardGetCurrentSpecial() == kPegSpecialAim ? fc % (PREDICTION_TRACE_LEN*2) : fc % PREDICTION_TRACE_LEN);
    if (i == 0) {
      physicsDoResetBall(0);
      physicsDoLaunchBall(1.0f);
    }
    renderSetMarbleTrace(pos[0], i);
  }

  m_physicsGetMotionTrailX[0][fc % MOTION_TRAIL_LEN] = m_ballPosition[0].x;
  m_physicsGetMotionTrailY[0][fc % MOTION_TRAIL_LEN] = m_ballPosition[0].y;

  if (twoBalls) {
    m_physicsGetMotionTrailX[1][fc % MOTION_TRAIL_LEN] = m_ballPosition[1].x;
    m_physicsGetMotionTrailY[1][fc % MOTION_TRAIL_LEN] = m_ballPosition[1].y;
    if (!m_secondBallInPlay) { physicsDoResetBall(1); }
  }

}
