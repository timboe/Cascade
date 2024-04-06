#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"
#include "game.h"
#include "bitmap.h"
#include "board.h"
#include "sound.h"
#include "ui.h"
#include "io.h"

#ifdef _WINDLL
__declspec(dllexport)
#endif

static void init(void) {
  initBoard();
  initBitmap();
  initiUI();
  initSound();
  initGame();
  initSpace();

  scanLevels();

  doIO(kDoTitle, /*and then*/ kDoNothing, /*and finally*/ kDoNothing);
}

static void deinit(void) {
}

int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg) {
  switch (event) {
    case kEventInit:;
      pdxlog("EH: init");
      setPDPtr(playdate);
      init();
      playdate->display->setRefreshRate(TICK_FREQUENCY);
      playdate->system->setUpdateCallback(gameLoop, NULL);
      break;
    case kEventTerminate:; case kEventLock:; case kEventLowPower:;
      pdxlog("EH: terminate/lock/low-p");
      //
      #ifdef SYNCHRONOUS_SAVE_ENABLED
      if (!IOOperationInProgress()) {
        synchronousSave();
      }
      #endif
      //
      if (event == kEventTerminate) {
        deinit();
      }
      break;
    case kEventUnlock:;
      pdxlog("EH: unlock");
      break;
    case kEventPause:;
      pdxlog("EH: pause");
      //playdate->system->setMenuImage(getPauseImage(), 0); // TODO
      break;
    case kEventResume:;
      pdxlog("EH: resume");
      break;
    case kEventKeyPressed:;
      #ifdef DEV
      playdate->system->logToConsole("EH: pressed %i", arg);
      #endif
      break;
    case kEventKeyReleased:;
      #ifdef DEV
      playdate->system->logToConsole("EH: released %i", arg);
      #endif
      break;
    default:
      #ifdef DEV
      playdate->system->logToConsole("EH: unknown event %i with arg %i", event, arg);
      #endif
      break;
  }
  
  return 0;
}