#pragma once
#include "game.h"

enum kSaveLoadRequest {kDoNothing, kDoSave, kDoLoad, kDoSaveDelete, kDoScanSlots, kDoTitle};

void scanSlots(void);

bool hasSaveData(void);

void hardReset(void);

void doIO(enum kSaveLoadRequest _first, enum kSaveLoadRequest _andThen, enum kSaveLoadRequest _andFinally);

enum kSaveLoadRequest currentIOAction(void);

bool IOOperationInProgress(void);

void enactIO(void);

bool doSave(void);

uint16_t getCurrentLevel(void);

uint16_t getCurrentHole(void);

uint16_t getPar(uint16_t level, uint16_t hole);

uint16_t getCurrentPar(void);