#include <math.h>

#include "game.h"
#include "board.h"
#include "bitmap.h"
#include "render.h"
#include "sound.h"
#include "input.h"
#include "ui.h"
#include "io.h"
#include "physics.h"

int32_t m_frameCount = 0;


////////////

int getFrameCount() { 
  return m_frameCount; 
}


int gameLoop(void* _data) {
  ++m_frameCount;
  pd->graphics->setBackgroundColor(kColorWhite);

  if (IOOperationInProgress()) { 
    pd->sprite->removeAllSprites();
    enactIO();
    pd->sprite->drawSprites();
    return 1;
  }

  const enum kGameMode gm = getGameMode();

  clickHandlerReplacement();

  // if (gm < kTitles) {
  //   movePlayer(/*forceUpdate*/ false);

  //   if (getFrameCount() % 2 == 0) {
  //     tickProjectiles();
  //     // Update UI
  //     pd->sprite->drawSprites();
  //   } else {
  //     tickEnemies();
  //     pd->sprite->drawSprites();
  //   }

  // }

  // if(m_frameCount % 8 == 0) {
  //   updateRenderList();
  // }

  // if (gm == kTitles) {
  //   updateUITitles(m_frameCount);
  //   pd->sprite->updateAndDrawSprites();
  // } else { 
  //   updateUI(m_frameCount);
  // }

  if (gm == kGameWindow) {

    float diffY = 0;
    if      (getPressed(2)) diffY = -SCREEN_ACC;
    else if (getPressed(3)) diffY =  SCREEN_ACC;
    modScrollVelocity(diffY);

    applyScrollEasing();



    updateBoard();
    updateSpace(TIMESTEP);

    // if (m_frameCount % 8 == 0 && !ballInPlay()) {
    //   updatePath();
    // }
  }


  //if (m_frameCount % 2)
  render(m_frameCount);

      doBallEndSweep();

  return 1;
}

void menuOptionsCallbackMenu(void* blank) {
  if (IOOperationInProgress()) { return; }
  pdxlog("menuOptionsCallbackMenu");
}

// Call prior to loading anything
void reset() {
  resetUI();
}

void populateMenuTitle() {
  pd->system->removeAllMenuItems();
  //pd->system->addMenuItem("delete save 3", menuOptionsCallbackDelete, (void*)2);
}

void populateMenuGame() {
  pd->system->removeAllMenuItems();
  //pd->system->addMenuItem("menu", menuOptionsCallbackMenu, NULL);
}

void initGame() {
  initSpace();
}

char* ftos(float _value, int16_t _size, char* _dest) {
  char* tmpSign = (_value < 0) ? "-" : "";
  float tmpVal = (_value < 0) ? -_value : _value;

  int16_t tmpInt1 = tmpVal;
  float tmpFrac = tmpVal - tmpInt1;
  int16_t tmpInt2 = trunc(tmpFrac * 10000);

  char truncStr[8];
  snprintf (_dest, _size, "%02d", tmpInt2);
  snprintf (truncStr, 8, "%.2s", _dest);

  snprintf (_dest, _size, "%s%d.%s", tmpSign, tmpInt1, truncStr);
  return _dest;
}