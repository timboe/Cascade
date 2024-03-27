#include "input.h"
#include "bitmap.h"
#include "render.h"
#include "sound.h"
#include "ui.h"
#include "io.h"
#include "physics.h"

uint8_t m_pressed[4] = {0};

float m_turretBarrelAngle = 0.0f;

void clickHandleWander(uint32_t _buttonPressed);

void clickHandleTitles(uint32_t _buttonPressed);

void clickHandleGameWindow(uint32_t _buttonPressed);

void rotateHandleGameWindow(float _angle);

void rotateHandleTitles(float _rotation);

bool aPressed(void);

uint16_t m_b, m_a, m_blockA;

/// ///


uint8_t getPressed(uint32_t _i) {
  return m_pressed[_i];
}

bool getPressedAny() {
  return m_pressed[0] || m_pressed[1] || m_pressed[2] || m_pressed[3];
}

bool bPressed() {
  return m_b;
}

bool aPressed() {
  return m_a;
}

void gameClickConfigHandler(uint32_t _buttonPressed) {
  switch (getGameMode()) {
    case kTitles: return clickHandleTitles(_buttonPressed);
    case kGameWindow: return clickHandleGameWindow(_buttonPressed);
    case kNGameModes: break;
  }
}


void clickHandleTitles(uint32_t _buttonPressed) {
  if (kButtonA == _buttonPressed) {
    //sfx(kSfxA);
    //doIO(kDoNothing, /*and then*/ kDoNewWorld, /*and finally*/ kDoNothing);
    setGameMode(kGameWindow);
  } else if (kButtonB == _buttonPressed) {
    //sfx(kSfxNo);
  }
}

void clickHandleGameWindow(uint32_t _buttonPressed) {
  if (kButtonA == _buttonPressed) {

    cpBodySetPosition(getBall(), cpv(HALF_DEVICE_PIX_X, UI_OFFSET_TOP));
    cpBodySetVelocity(getBall(), cpvzero);
    cpBodySetAngle(getBall(), 0);
    cpBodySetAngularVelocity(getBall(), 0);

    setBallInPlay(true);

    const float angleRad = m_turretBarrelAngle * (M_PIf / 180.0f);
    cpBodyApplyImpulseAtLocalPoint(getBall(), cpv(POOT_STRENGTH * sinf(angleRad), POOT_STRENGTH * -cosf(angleRad)), cpvzero);

  } else if (kButtonB == _buttonPressed) {

    setBallInPlay(false);

    cpBodySetPosition(getBall(), cpv(WFALL_PIX_X/2, WFALL_PIX_Y*4));


    for (int i = 0; i < N_OBST/2; ++i) {
      cpBodySetPosition(getObst(i), cpv( ((i+1)*32) % WFALL_PIX_X, ((i+1)*32) + UI_OFFSET_TOP));
      cpBodySetVelocity(getObst(i), cpv(32.0f, 0));

      cpBodySetPosition(getBox(i), cpv( WFALL_PIX_X - (((i+1)*32) % WFALL_PIX_X), ((i+1)*32) + 16 + UI_OFFSET_TOP));
      cpBodySetVelocity(getBox(i), cpv(-32.0f, 0));
      cpBodySetAngle(getBox(i), ((2*M_PIf)/(N_OBST/2))*i);
    }

    for (int i = N_OBST/2; i < N_OBST; ++i) {
      cpBodySetPosition(getObst(i), cpv(rand() % WFALL_PIX_X, (rand() % WFALL_PIX_Y) + UI_OFFSET_TOP));
      cpBodySetPosition(getBox(i), cpv(rand() % WFALL_PIX_X, (rand() % WFALL_PIX_Y) + UI_OFFSET_TOP));
      cpBodySetAngle(getBox(i), (M_PIf/180.0f) * (rand()%360));
    }

    // for (int i = 0; i < N_OBST; ++i) {
    //   cpBodySetPosition(getObst(i), cpvzero);
    //   cpBodySetPosition(getBox(i), cpvzero);
    //   cpBodySetVelocity(getObst(i), cpvzero);
    //   cpBodySetVelocity(getBox(i), cpvzero);
    // }
    // cpBodySetAngle(getBox(0), M_PIf/4.0f);
    // cpBodySetPosition(getBox(0), cpv(WFALL_PIX_X/2, 128 + UI_OFFSET_TOP));

  }
}

void rotateHandleTitles(float _rotation) {
 
}

void rotateHandleGameWindow(float _angle) {
  m_turretBarrelAngle = _angle;
}

float getTurretBarrelAngle(void) {
  return m_turretBarrelAngle;
}

void clickHandlerReplacement() {
  static uint8_t multiClickCount = 16, multiClickNext = 16;
  enum kGameMode gm = getGameMode();
  PDButtons current, pushed, released = 0;
  pd->system->getButtonState(&current, &pushed, &released);

  if (pushed & kButtonLeft) m_pressed[0] = 1;
  if (pushed & kButtonRight) m_pressed[1] = 1;
  if (pushed & kButtonUp) m_pressed[2] = 1;
  if (pushed & kButtonDown) m_pressed[3] = 1;

  if (pushed & kButtonUp) gameClickConfigHandler(kButtonUp);
  if (pushed & kButtonRight) gameClickConfigHandler(kButtonRight);
  if (pushed & kButtonDown) gameClickConfigHandler(kButtonDown);
  if (pushed & kButtonLeft) gameClickConfigHandler(kButtonLeft);
  if (current & kButtonB) ++m_b;
  if (released & kButtonB) {
    if (m_b < BUTTON_PRESSED_FRAMES) gameClickConfigHandler(kButtonB);
    m_b = 0;
  }
  if (released & kButtonA) {
    if (m_a < BUTTON_PRESSED_FRAMES) gameClickConfigHandler(kButtonA);
    multiClickCount = 8;
    multiClickNext = 8;
    m_a = 0;
    m_blockA = 0;
  } else if (current & kButtonA)  {
    ++m_a;
    // if (gm == kPlaceMode || gm == kPickMode || gm == kPlantMode || gm == kDestroyMode) {
    //   gameClickConfigHandler(kButtonA); // Special, allow pick/placing rows of conveyors
    // } else if (gm >= kMenuBuy) {
    //   if (--multiClickCount == 0) {
    //     gameClickConfigHandler(kButtonA); // Special, allow speed buying/selling
    //     if (multiClickNext > 2) --multiClickNext;
    //     multiClickCount = multiClickNext;
    //   }
    // }
  }

  if (released & kButtonLeft) m_pressed[0] = 0;
  if (released & kButtonRight) m_pressed[1] = 0;
  if (released & kButtonUp) m_pressed[2] = 0;
  if (released & kButtonDown) m_pressed[3] = 0;

  const float cc = pd->system->getCrankChange();
  if (cc) {
    switch (gm) {
      case kTitles: rotateHandleTitles(pd->system->getCrankChange()); break; 
      case kGameWindow: rotateHandleGameWindow(pd->system->getCrankAngle()); break;
      default: break;
    }
  }

}