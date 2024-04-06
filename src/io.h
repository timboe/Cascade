#pragma once
#include "game.h"

enum kSaveLoadRequest {kDoNothing, kDoSave, kDoLoad, kDoSaveDelete, kDoScanSlots, kDoTitle};

void scanLevels(void);


void hardReset(void);

void doIO(enum kSaveLoadRequest _first, enum kSaveLoadRequest _andThen, enum kSaveLoadRequest _andFinally);

enum kSaveLoadRequest currentIOAction(void);

bool IOOperationInProgress(void);

void enactIO(void);

bool doSave(void);

uint16_t getCurrentLevel(void);

uint16_t getPreviousLevel(void);

uint16_t getNextLevel(void);

void doPreviousLevel(void);

void doNextLevel(void);

uint16_t getCurrentHole(void);

uint16_t getPreviousHole(void);

uint16_t getNextHole(void);

void doPreviousHole(void);

void doNextHole(void);

void setHoleScore(uint16_t score);

uint16_t getPar(uint16_t level, uint16_t hole);

uint16_t getCurrentLevelPar(void);

uint16_t getScore(uint16_t level, uint16_t hole);

uint16_t getCurrentLevelScore(void);
