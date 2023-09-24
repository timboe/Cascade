#include "input.h"
#include "sprite.h"
#include "render.h"
#include "sound.h"
#include "ui.h"
#include "io.h"

uint8_t m_pressed[4] = {0};

void clickHandleWander(uint32_t _buttonPressed);

void clickHandleTitles(uint32_t _buttonPressed);

void rotateHandleWander(float _rotation);

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
    case kNGameModes: break;
  }
}


void clickHandleTitles(uint32_t _buttonPressed) {
  if (kButtonA == _buttonPressed) {
    //sfx(kSfxA);
    //doIO(kDoNothing, /*and then*/ kDoNewWorld, /*and finally*/ kDoNothing);
  } else if (kButtonB == _buttonPressed) {
    //sfx(kSfxNo);
  }
}


void rotateHandleTitles(float _rotation) {
 
}

void clickHandlerReplacement() {
  static uint8_t multiClickCount = 16, multiClickNext = 16;
  enum kGameMode gm = getGameMode();
  PDButtons current, pushed, released = 0;
  pd->system->getButtonState(&current, &pushed, &released);
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

  switch (gm) {
    case kTitles: rotateHandleTitles(pd->system->getCrankChange()); break; 
    default: break;
  }

}