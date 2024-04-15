#pragma once
#include "game.h"

#ifdef TAKE_SCREENSHOTS
void screenShotInit(void);

void screenShotDo(void);

bool screenShotGetInProgress(void);
#endif