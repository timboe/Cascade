#include <math.h>

#include "game.h"
#include "sprite.h"
#include "render.h"
#include "sound.h"
#include "input.h"
#include "ui.h"
#include "io.h"

PlaydateAPI* pd = NULL;

int32_t m_frameCount = 0;

uint8_t m_tickID;

////////////

void setPDPtr(PlaydateAPI* _p) {
  pd = _p;
}

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


  render();

  return 1;
}

void menuOptionsCallbackMenu(void* blank) {
  if (IOOperationInProgress()) { return; }
  #ifdef DEV
  pd->system->logToConsole("menuOptionsCallbackMenu");
  #endif
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