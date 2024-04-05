#include "io.h"
#include "ui.h"
#include "render.h"
#include "input.h"
#include "sound.h"


enum kSaveLoadRequest m_doFirst = kDoNothing, m_andThen = kDoNothing, m_andFinally = kDoNothing;

int8_t m_ioStage = 0;

uint32_t m_actionProgress = 0;

SDFile* m_file;

uint16_t m_player = 0;
uint16_t m_level = 0;
uint16_t m_hole = 0;

uint32_t m_player_score[SAVE_FORMAT_1_MAX_PLAYERS][SAVE_FORMAT_1_MAX_LEVELS][SAVE_FORMAT_1_MAX_HOLES] = {0};

uint32_t m_level_par[MAX_LEVELS][MAX_HOLES] = {0};


int doRead(void* _userdata, uint8_t* _buf, int _bufsize);

void doWrite(void* _userdata, const char* _str, int _len);

bool doLoad(void);

bool doNewWorld(void);

bool doSaveDelete(void);

bool doTitle(void);

void decodeError(json_decoder* jd, const char* _error, int _linenum);

void willDecodeSublist(json_decoder* jd, const char* _name, json_value_type _type);

void scanDidDecodeSaveFormat(json_decoder* jd, const char* _key, json_value _value);

void deserialiseValueSaveFormat(json_decoder* jd, const char* _key, json_value _value);

int scanShouldDecodeTableValueForKey(json_decoder* jd, const char* _key);

// // json_encoder m_je;

// json_decoder m_jd_p = {
//   // .decodeError = decodeError,
//   // .didDecodeTableValue = didDecodeTableValuePlayer,
//   // .didDecodeArrayValue = deserialiseArrayValuePlayer
// };

// json_decoder m_jd = {
//   // .decodeError = decodeError,
//   // .willDecodeSublist = willDecodeSublist
// };


/// ///

uint16_t getCurrentPlayer(void) { return m_player; }

uint16_t getCurrentLevel(void) { return m_level; }

uint16_t getCurrentHole(void) { return m_hole; }

uint16_t getPar(uint16_t level, uint16_t hole) { return /*m_level_par[level][hole];*/ 3; }

uint16_t getScore(uint16_t level, uint16_t hole) { return /*m_player_score[m_player][level][hole];*/ 4; }

uint16_t getCurrentLevelPar(void) { return getPar(m_level, m_hole); }

uint16_t getCurrentLevelScore(void) { return getScore(m_level, m_hole); }

void nextHole(uint16_t currentHoleScore) {
  if (currentHoleScore < m_player_score[m_player][m_level][m_hole]) {
    m_player_score[m_player][m_level][m_hole] = currentHoleScore;
  }

  if (++m_hole == MAX_HOLES) {
    m_hole = 0;
    ++m_level;
  }
}

void doIO(enum kSaveLoadRequest _first, enum kSaveLoadRequest _andThen, enum kSaveLoadRequest _andFinally) {
  if (IOOperationInProgress()) {
    return;
  }
  m_doFirst = _first;
  m_andThen = _andThen;
  m_andFinally = _andFinally;
  m_ioStage = 0;
  m_actionProgress = 0;
  pauseMusic();
  #ifdef DEV
  pd->system->logToConsole("IO: Requested %i, %i, %i", _first, _andThen, _andFinally);
  #endif
}

bool IOOperationInProgress() {
  return m_doFirst != kDoNothing;
}

enum kSaveLoadRequest currentIOAction() {
  switch (m_ioStage) {
    case 0: return m_doFirst;
    case 1: return m_andThen;
    case 2: return m_andFinally;
    default: return kDoNothing;
  }
  return kDoNothing;
}

void enactIO() {
  #ifdef DEV
  pd->system->logToConsole("IO: Enact IO - %i", currentIOAction());
  #endif

  bool finished = false;
  switch (currentIOAction()) {
    case kDoNothing: finished = true; break;
    case kDoSave: finished = doSave(); break;
    case kDoLoad: finished = doLoad(); break;
    case kDoTitle: finished = doTitle(); break;
    case kDoSaveDelete: finished = doSaveDelete(); break;
    case kDoScanSlots: finished = true; scanLevels(); break;
  }

  if (finished) {
    if (m_ioStage == 0) {
      #ifdef DEV
      pd->system->logToConsole("IO: Stage 1 Finished");
      #endif
      ++m_ioStage;
      m_actionProgress = 0;
    } else if (m_ioStage == 1) {
      #ifdef DEV
      pd->system->logToConsole("IO: Stage 2 Finished");
      #endif
      ++m_ioStage;
      m_actionProgress = 0;
    } else {
      // Specials...
      #ifdef DEV
      pd->system->logToConsole("IO: Stage 3 Finished");
      #endif
      m_ioStage = 0;
      m_actionProgress = 0;
      m_doFirst = kDoNothing;
      m_andThen = kDoNothing;
      m_andFinally = kDoNothing;
      resumeMusic();
    }
  }
}

int doRead(void* _userdata, uint8_t* _buf, int _bufsize) {
  return pd->file->read((SDFile*)_userdata, _buf, _bufsize);
}

void doWrite(void* _userdata, const char* _str, int _len) {
  pd->file->write((SDFile*)_userdata, _str, _len);
}

void decodeError(json_decoder* jd, const char* _error, int _linenum) {
  pd->system->logToConsole("decode error line %i: %s", _linenum, _error);
  doIO(kDoTitle, /*and then*/ kDoNothing, /*and finally*/ kDoNothing);
}


///

bool doTitle() {

  reset();
  setGameMode(kTitles);

  populateMenuTitle();
  updateMusic(/*isTitle=*/true);

  return true;
}

///

bool doSaveDelete() {
  #ifdef DEV
  pd->system->logToConsole("DELETE SAVE (via rename to delete_world and delete_save)");
  #endif

  // char filePathDelete[32];

  // // Don't actually delete, just rename to "deleted_"
  // for (uint16_t ss = 0; ss < WORLD_SAVE_SLOTS; ++ss) {
  //   snprintf(m_filePath, 32, "world_%i_%i.json", m_save+1, ss+1);
  //   snprintf(filePathDelete, 32, "deleted_world_%i_%i.json", m_save+1, ss+1);
  //   // First unlink before moving
  //   int status = pd->file->unlink(filePathDelete, 0);
  //   #ifdef DEV
  //   pd->system->logToConsole("DELETE: unlink previous world save %s, status %i", filePathDelete, status);
  //   #endif
  //   pd->file->rename(m_filePath, filePathDelete);
  // }
  // snprintf(m_filePath, 32, "player_%i.json", m_save+1);
  // snprintf(filePathDelete, 32, "deleted_player_%i.json", m_save+1);
  // int status = pd->file->unlink(filePathDelete, 0);
  // #ifdef DEV
  // pd->system->logToConsole("DELETE unlink previous player save %s, status %i", filePathDelete, status);
  // #endif
  // pd->file->rename(m_filePath, filePathDelete);

  // Finished
  return true;
}

///

FileOptions fileReadMode(uint16_t level) {
  if (level < 10) return kFileRead; // First 10 levels are built-in
  return kFileRead|kFileReadData; // Later levels might be side loaded
}

int shouldDecodeScan(json_decoder* jd, const char* key) {
  return (strcmp(key, "par") == 0);
}

void didDecodeScan(json_decoder* jd, const char* key, json_value value) {
  if (strcmp(key, "par") == 0) {
    m_level_par[m_level][m_hole] = json_intValue(value);
    pd->system->logToConsole("m_level_par[%i][%i] = %i", m_level, m_hole, m_level_par[m_level][m_hole]);
  } else {
    pd->system->error("didDecodeScan DECODE ISSUE, %s", key);
  }
}

void scanLevels() {
  char filePath[128];
  for (int32_t l = 0; l < MAX_LEVELS; ++l) {
    m_level = l;
    bool abort = false;
    for (int32_t h = 0; h < MAX_HOLES; ++h) {
      m_hole = h;
      snprintf(filePath, 128, "levels/fall_%i_hole_%i.json", (int)l+1, (int)h+1);
      SDFile* file = pd->file->open(filePath, fileReadMode(l));
      if (!file) {
        if (!h) { abort = true; } // If missing the first fall of this level then stop looking here
        break;
      }
      static json_decoder jd = {
        .decodeError = decodeError,
        .didDecodeTableValue = didDecodeScan,
        .shouldDecodeTableValueForKey = shouldDecodeScan
      };
      pd->json->decode(&jd, (json_reader){ .read = doRead, .userdata = file }, NULL);
      int status = pd->file->close(file);
    }
    if (abort) { break; }
  }



  // for (m_scanSlot = 0; m_scanSlot < WORLD_SAVE_SLOTS; ++m_scanSlot) {
  //   m_worldVersions[m_save][m_scanSlot] = -1;

  //   snprintf(m_filePath, 32, "world_%i_%i.json", m_save+1, m_scanSlot+1);
  //   SDFile* file = pd->file->open(m_filePath, kFileRead|kFileReadData);
  //   if (!file) {
  //     #ifdef DEV
  //     pd->system->logToConsole("Scan world: Save:%i, Slot:%i, No Save", m_save, m_scanSlot);
  //     #endif
  //     m_worldExists[m_save][m_scanSlot] = false;
  //     continue;
  //   }

  //   json_decoder jd = {
  //     .decodeError = decodeError,
  //     .didDecodeTableValue = scanDidDecodeSaveFormat,
  //     .shouldDecodeTableValueForKey = scanShouldDecodeTableValueForKey
  //   };

  //   pd->json->decode(&jd, (json_reader){ .read = doRead, .userdata = file }, NULL);
  //   int status = pd->file->close(file);
  //   if (status) pd->system->error("SCAN SLOTS ERROR: wold file->close status code: %i", status);

  //   #ifdef DEV
  //   pd->system->logToConsole("Scan world: Save:%i, Slot:%i, Version:%i", m_save, m_scanSlot, m_worldVersions[m_save][m_scanSlot]);
  //   #endif
  //   m_worldExists[m_save][m_scanSlot] = true;
  //   m_foundSaveData[m_save] = true;
  // }

  // // Filter
  // for (uint16_t ss = 0; ss < WORLD_SAVE_SLOTS; ++ss) {
  //   if (m_worldExists[m_save][ss]) {
  //     if (m_worldVersions[m_save][ss] == -1) {
  //       pd->system->error("Scan world: Unable to determine save version for save file %i", m_save);
  //       m_foundSaveData[m_save] = false;
  //       m_worldExists[m_save][ss] = false;
  //     } else if (m_worldVersions[m_save][ss] < EARLIEST_SUPPORTED_SAVE_FORMAT) {
  //       #ifdef DEV
  //       pd->system->logToConsole("Scan world: PRE-BETA WORLD DETECTED! Version %i < %i."
  //         " ACTION: Delete everything and start again", m_worldVersions[m_save][ss], EARLIEST_SUPPORTED_SAVE_FORMAT);
  //       #endif
  //       m_foundSaveData[m_save] = false;
  //       m_worldExists[m_save][ss] = false;
  //       doSaveDelete();
  //     }
  //   }
  // }

  // {
  //   snprintf(m_filePath, 32, "player_%i.json", m_save+1);
  //   SDFile* file = pd->file->open(m_filePath, kFileRead|kFileReadData);
  //   if (!file) {
  //     m_foundSaveData[m_save] = false;
  //     #ifdef DEV
  //     pd->system->logToConsole("Scan world: No Player data found!");
  //     #endif
  //   } else {
  //     int status = pd->file->close(file);
  //     if (status) pd->system->error("SCAN SLOTS ERROR: player file->close status code: %i", status);
  //     #ifdef DEV
  //     pd->system->logToConsole("Scan world: Player data found");
  //     #endif
  //   }
  // }

}

// int scanShouldDecodeTableValueForKey(json_decoder* jd, const char* _key) {
//   return (strcmp(_key, "sf") == 0);
// }

// void scanDidDecodeSaveFormat(json_decoder* jd, const char* _key, json_value _value) {
//   if (strcmp(_key, "sf") == 0) {
//     m_worldVersions[m_save][m_scanSlot] = json_intValue(_value);
//   } else {
//     pd->system->error("scanDidDecodeSaveFormat DECODE ISSUE, %s", _key);
//   }
// }

// void deserialiseValueSaveFormat(json_decoder* jd, const char* _key, json_value _value) {
//   if (strcmp(_key, "sf") == 0) {
//     m_worldVersions[m_save][m_slot] = json_intValue(_value);
//   } else {
//     pd->system->error("deserialiseValueSaveFormat DECODE ISSUE, %s", _key);
//   }
// }



bool doSave() {
  uint8_t pretty = 0;
  #ifdef DEV
  pretty = 1;
  pd->system->logToConsole("SAVE", m_actionProgress);
  #endif

  // if (m_actionProgress == 0) {

  //   // Should get the latest export averages, guaranteed to be between 60 and 120s worth of data
  //   snprintf(m_filePath, 32, "TMP_player_%i.json", m_save+1);

  //   // This file should not already exist, double check
  //   int status = pd->file->unlink(m_filePath, 0);
  //   #ifdef DEV
  //   pd->system->logToConsole("SAVE: unlink previous TMP_player %s, status %i (expect this to fail, err: %s)", m_filePath, status, pd->file->geterr());
  //   #endif

  //   if (m_file) pd->system->error("SAVE ERROR: tmp player error: overwriting exiting file ptr");
  //   m_file = pd->file->open(m_filePath, kFileWrite);
  //   if (!m_file) pd->system->error("SAVE ERROR: tmp player file creation error: %s", pd->file->geterr());

  //   pd->json->initEncoder(&m_je, doWrite, m_file, pretty);

  // } else if (m_actionProgress == 1) {

  //   m_je.startTable(&m_je);
  //   serialisePlayer(&m_je);
  //   m_je.endTable(&m_je);

  //   int status = pd->file->close(m_file);
  //   if (status) pd->system->error("SAVE ERROR: tmp player file->close status code: %i, err: %s", status, pd->file->geterr());
  //   m_file = NULL;

  // } else if (m_actionProgress == 2) {

  //   snprintf(m_filePath, 32, "TMP_world_%i_%i.json", m_save+1, m_slot+1);

  //   // This file should not already exist, double check
  //   int status = pd->file->unlink(m_filePath, 0);
  //   #ifdef DEV
  //   pd->system->logToConsole("SAVE: unlink previous TMP_world %s, status %i (expect this to fail, err: %s)", m_filePath, status, pd->file->geterr());
  //   #endif

  //   if (m_file) pd->system->error("SAVE ERROR: tmp world error: overwriting exiting file ptr");
  //   m_file = pd->file->open(m_filePath, kFileWrite);
  //   if (!m_file) pd->system->error("SAVE ERROR: tmp world file creation error: %s", pd->file->geterr());

  //   pd->json->initEncoder(&m_je, doWrite, m_file, pretty);

  //   m_je.startTable(&m_je);

  //   m_je.addTableMember(&m_je, "sf", 2);
  //   m_je.writeInt(&m_je, CURRENT_SAVE_FORMAT);

  // } else if (m_actionProgress == 3) {
  //   serialiseCargo(&m_je);
  // } else if (m_actionProgress == 4) {
  //   serialiseBuilding(&m_je);
  // } else if (m_actionProgress == 5) {
  //   serialiseLocation(&m_je);
  // } else if (m_actionProgress == 6) {
  //   serialiseWorld(&m_je);
  // } else if (m_actionProgress == 7) {
  //   m_je.endTable(&m_je);

  //   int status = pd->file->close(m_file);
  //   if (status) pd->system->error("SAVE ERROR: tmp world file->close status code: %i, err %s", status, pd->file->geterr());
  //   m_file = NULL;

  //   // Create backup
  //   char filePathBackup[32] = {0};
  //   FileStat fs;

  //   // We can only backup the player if we are not resetting them (new game)
  //   snprintf(filePathBackup, 32, "backup_player_%i.json", m_save+1);
  //   status = pd->file->stat(filePathBackup, &fs); // Check also we have a player to backup
  //   if (m_doFirst != kDoResetPlayer && status == 0) {
  //     snprintf(m_filePath, 32, "player_%i.json", m_save+1);
  //     status = pd->file->unlink(filePathBackup, 0);
  //     #ifdef DEV
  //     pd->system->logToConsole("SAVE: unlink previous player backup %s, status %i", filePathBackup, status);
  //     pd->system->logToConsole("SAVE: Backup: %s -> %s", m_filePath, filePathBackup);
  //     #endif
  //     status = pd->file->rename(m_filePath, filePathBackup);
  //     if (status) pd->system->error("SAVE ERROR: backup player file->rename status code: %i, err: %s", status, pd->file->geterr());
  //   }

  //   // We can only backup a previous world file if we are not in worldgen mode
  //   snprintf(filePathBackup, 32, "backup_world_%i_%i.json", m_save+1, m_slot+1);
  //   status = pd->file->stat(filePathBackup, &fs); // Check also we have a world to backup
  //   if (m_andThen != kDoNewWorld && status == 0) {
  //     snprintf(m_filePath, 32, "world_%i_%i.json", m_save+1, m_slot+1);
  //     status = pd->file->unlink(filePathBackup, 0);
  //     #ifdef DEV
  //     pd->system->logToConsole("SAVE: unlink previous world backup %s, status %i", filePathBackup, status);
  //     pd->system->logToConsole("SAVE: Backup: %s -> %s", m_filePath, filePathBackup);
  //     #endif
  //     status = pd->file->rename(m_filePath, filePathBackup);
  //     if (status) pd->system->error("SAVE ERROR: backup world file->rename status code: %i, err: %s", status, pd->file->geterr());
  //   }

  //   // Finish by moving into location
  //   {
  //     char filePathFinal[32];
  //     snprintf(m_filePath, 32, "TMP_player_%i.json", m_save+1);
  //     snprintf(filePathFinal, 32, "player_%i.json", m_save+1);
  //     status = pd->file->unlink(filePathFinal, 0);
  //     #ifdef DEV
  //     pd->system->logToConsole("SAVE: unlink previous player %s, status %i (expect this to fail, err: %s)", filePathFinal, status, pd->file->geterr());
  //     pd->system->logToConsole("SAVE: Finalise: %s -> %s", m_filePath, filePathFinal);
  //     #endif
  //     status = pd->file->rename(m_filePath, filePathFinal);
  //     if (status) pd->system->error("SAVE ERROR: mv player file->rename status code: %i, err: %s", status, pd->file->geterr());

  //     snprintf(m_filePath, 32, "TMP_world_%i_%i.json", m_save+1, m_slot+1);
  //     snprintf(filePathFinal, 32, "world_%i_%i.json", m_save+1, m_slot+1);
  //     status = pd->file->unlink(filePathFinal, 0);
  //     #ifdef DEV
  //     pd->system->logToConsole("SAVE: unlink previous world %s, status %i (expect this to fail, err: %s)", filePathFinal, status, pd->file->geterr());
  //     pd->system->logToConsole("SAVE: Finalise: %s -> %s", m_filePath, filePathFinal);
  //     #endif
  //     status = pd->file->rename(m_filePath, filePathFinal);
  //     if (status) pd->system->error("SAVE ERROR: mv world file->rename status code: %i, err: %s", status, pd->file->geterr());
  //   }

  //   #ifdef DEV
  //   pd->system->logToConsole("SAVE: Saved to Save:%i, Slot %u, save status %i", m_save, m_slot, status);
  //   #endif

  // } else if (m_actionProgress == 8) {
    
  //   scanSlots();

  //   // Finished
  //   #ifdef SLOW_LOAD
  //   float f; for (int32_t i = 0; i < 10000; ++i) for (int32_t j = 0; j < 10000; ++j) { f*=i*j; }
  //   #endif

  //   return true;
  // }

  ++m_actionProgress;
  return true;
}

///

bool doLoad() {

  #ifdef DEV
  pd->system->logToConsole("LOAD: Progress %i", m_actionProgress);
  #endif

  // if (m_actionProgress == 0) {

  //   // Clear in preparation for load (including player, if we haven't been given a slot override)
  //   const bool resetThePlayerToo = (m_forceSlot == -1);
  //   reset(resetThePlayerToo);

  //   snprintf(m_filePath, 32, "player_%i.json", m_save+1);
  //   if (m_file) pd->system->error("LOAD ERROR: read player error: overwriting exiting file ptr");
  //   m_file = pd->file->open(m_filePath, kFileRead|kFileReadData);
  //   if (!m_file) pd->system->error("LOAD ERROR: read player error: %s", pd->file->geterr());

  //   pd->json->decode(&m_jd_p, (json_reader){ .read = doRead, .userdata = m_file }, NULL);

  //   int status = pd->file->close(m_file);
  //   if (status) pd->system->error("LOAD ERROR: player file->close status code: %i", status);
  //   m_file = NULL;

  // } else if (m_actionProgress >= 1 && m_actionProgress <= 4) {

  //   // We have now loaded the correct slot number for this player-save.
  //   // But we might want to be loading into a different world 
  //   if (m_forceSlot != -1) {
  //     #ifdef DEV
  //     pd->system->logToConsole("LOAD: Save:%i, Slot Override from %i to %i", m_save, m_slot, m_forceSlot);
  //     #endif
  //     setSlot(m_forceSlot);
  //   }

  //   snprintf(m_filePath, 32, "world_%i_%i.json", m_save+1, m_slot+1);
  //   if (m_file) pd->system->error("LOAD ERROR: read world error: overwriting exiting file ptr");
  //   m_file = pd->file->open(m_filePath, kFileRead|kFileReadData);
  //   if (!m_file) pd->system->error("LOAD ERROR: read world error: %s", pd->file->geterr());

  //   pd->json->decode(&m_jd, (json_reader){ .read = doRead, .userdata = m_file }, NULL);

  //   int status = pd->file->close(m_file);
  //   if (status) pd->system->error("LOAD ERROR: world file->close status code: %i", status);
  //   m_file = NULL;

  // } else if (m_actionProgress == 5) {

  //   // SCHEMA EVOLUTION - V4 to V5 (v1.0 to v1.1)
  //   // Factories now save their production time internally. Need to compute this for all existing factories.
  //   if (m_worldVersions[m_save][m_slot] == V1p0_SAVE_FORMAT) {
  //     m_worldVersions[m_save][m_slot] = V1p1_SAVE_FORMAT;
  //     uint16_t nFacsUpdated = 0;
  //     for (uint16_t i = 0; i < TOT_CARGO_OR_BUILDINGS; ++i) {
  //       struct Building_t* b = buildingManagerGetByIndex(i);
  //       if (!b) continue;
  //       if (b->m_type != kFactory) continue;
  //       ++nFacsUpdated;
  //       updateFactoryUpgrade(b);
  //     } 
  //     pd->system->logToConsole("-- Performed world schema evolution from v%i to v%i (Save:%i, World:%i), updated %i factories", 
  //       V1p0_SAVE_FORMAT, V1p1_SAVE_FORMAT, m_save, m_slot, nFacsUpdated);
  //   }


  //   // Things which need to run post-load
  //   setGameMode(kWanderMode);

  //   // need to refresh conveyor sprite connections
  //   for (uint16_t i = 0; i < TOT_CARGO_OR_BUILDINGS; ++i) {
  //     conveyorUpdateSprite(buildingManagerGetByIndex(i));
  //   } 

  //   addObstacles();
  //   doWetness(/*for titles = */ false);
  //   setChunkBackgrounds(/*for title = */ false);
  //   showTutorialMsg(getTutorialStage());

  //   // Update audio settings
  //   updateSfx();
  //   updateMusic(/*isTitle=*/ false);
  //   updateMusicVol();

  //   populateMenuGame();

  //   forceTorus();

  //   // Finished
  //   #ifdef SLOW_LOAD
  //   float f; for (int32_t i = 0; i < 10000; ++i) for (int32_t j = 0; j < 10000; ++j) { f*=i*j; }
  //   #endif

  //   return true;
  // }

  ++m_actionProgress;    
  return true;
}

// void willDecodeSublist(json_decoder* jd, const char* _name, json_value_type _type) {

//   static char truncated[6];
//   strncpy(truncated, _name, 5);
//   truncated[5] = '\0';

//   if (strcmp(truncated, "sf") == 0 && _type == kJSONTable) {
//     jd->didDecodeTableValue = deserialiseValueSaveFormat;
//   } else if (m_actionProgress == 1 && strcmp(truncated, "cargo") == 0 && _type == kJSONTable) {
//     jd->didDecodeTableValue = deserialiseValueCargo;
//     jd->didDecodeSublist = deserialiseStructDoneCargo;
//   } else if (m_actionProgress == 2 && strcmp(truncated, "build") == 0 && _type == kJSONTable) {
//     jd->didDecodeTableValue = deserialiseValueBuilding;
//     jd->didDecodeSublist = deserialiseStructDoneBuilding;
//   } else if (m_actionProgress == 3 && strcmp(truncated, "locat") == 0 && _type == kJSONTable) {
//     jd->didDecodeTableValue = deserialiseValueLocation;
//     jd->didDecodeSublist = deserialiseStructDoneLocation;
//   } else if (m_actionProgress == 4 && strcmp(truncated, "world") == 0 && _type == kJSONArray) {
//     jd->didDecodeTableValue = NULL;
//     jd->didDecodeSublist = NULL;
//     jd->didDecodeArrayValue = deserialiseArrayValueWorld;
//   } else {
//     jd->didDecodeTableValue = NULL;
//   }
  
// }
