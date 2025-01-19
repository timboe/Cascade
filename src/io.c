#include "io.h"
#include "fsm.h"
#include "render.h"
#include "input.h"
#include "sound.h"
#include "peg.h"
#include "board.h"
#include "physics.h"
#include "util.h"
#include "bitmap.h"

///

enum DecodeType_t {
  kDecodeStatic,
  kDecodeElliptic,
  kDecodeLinear
};

enum DecodeType_t m_decodeType;

///

char m_nameStatic[128];
uint16_t m_staticID = 0;
struct StaticLoader_t m_static = {0};

///

char m_nameElliptic[128];
uint16_t m_ellipticID = 0;
struct EllipticLoader_t m_elliptic = {0};

///

char m_nameLinear[128];
uint16_t m_linearID = 0;
struct LinearLoader_t m_linear = {0};

///

char m_namePegContainer[128];
uint16_t m_pegContainerID = 0;

char m_nameLineContainer[128];
uint16_t m_lineContainerID = 0;

///

uint16_t m_preloading = 0;

uint16_t m_player = 0;
uint16_t m_level = 0;
uint16_t m_hole = 0;

const uint16_t SAVE_SIZE_V1 = sizeof(uint32_t) * SAVE_FORMAT_1_MAX_PLAYERS * SAVE_FORMAT_1_MAX_LEVELS * SAVE_FORMAT_1_MAX_HOLES;
uint32_t m_persistent_data[SAVE_FORMAT_1_MAX_PLAYERS][SAVE_FORMAT_1_MAX_LEVELS][SAVE_FORMAT_1_MAX_HOLES] = {0};

// The +1 is to allow for credits
#define MAX_CHARACTERS 32
uint16_t m_hole_par[MAX_LEVELS+1][MAX_HOLES] = {0};
uint16_t m_hole_height[MAX_LEVELS+1][MAX_HOLES] = {0};
uint16_t m_hole_foreground[MAX_LEVELS+1][MAX_HOLES] = {0};
uint16_t m_hole_background[MAX_LEVELS+1][MAX_HOLES] = {0};
enum PegSpecial_t m_hole_special[MAX_LEVELS+1][MAX_HOLES] = {0};
char m_hole_name[MAX_LEVELS+1][MAX_HOLES][MAX_CHARACTERS] = {0};
char m_hole_author[MAX_LEVELS+1][MAX_HOLES][MAX_CHARACTERS] = {0};

int IODoRead(void* userdata, uint8_t* buf, int bufsize);

void IODoWrite(void* userdata, const char* str, int len);

void IODecodeError(json_decoder* jd, const char* error, int linenum);

void IODoScanLevels(void);

void IOWriteCustomLevelInstructions(void);

void IOReadVolumePreferences(void);

///

int IOShouldDecodeScan(json_decoder* jd, const char* key);

void IODidDecodeScan(json_decoder* jd, const char* key, json_value value);

///

void IOWillDecodeLevel(json_decoder* jd, const char* key, json_value_type type);

void IODidDecodeLevel(json_decoder* jd, const char* key, json_value value);

void* IOFinishDecodeLevel(json_decoder* jd, const char* key, json_value_type type);

/// ///

bool IOIsCredits(void) { return m_level == MAX_LEVELS; }

bool IOGetIsPreloading(void) { return m_preloading != PRELOADING_STEPS; }

float IOGetPreloadingProgress(void) {
  return m_preloading / (float)PRELOADING_STEPS;
}

void IODonePreloading(void) {
  // soundWaterfallDoInit(); - now handled by IOReadVolumePreferences
  gameDoPopulateMenuTitles();
}

void IODoUpdatePreloading(void) {
  if (!IOGetIsPreloading()) { return; }

  const uint32_t before = pd->system->getCurrentTimeMilliseconds();
  switch (m_preloading) {
    case 0: IODoScanLevels(); break;
    case 1: bitmapDoPreloadA(); break;
    case 2: bitmapDoPreloadB(0); break;
    case 3: bitmapDoPreloadB(1); break;
    case 4: bitmapDoPreloadB(2); break;
    case 5: bitmapDoPreloadB(3); break;
    case 6: bitmapDoPreloadB(4); break;
    case 7: bitmapDoPreloadB(5); break;
    case 8: bitmapDoPreloadB(6); break;
    case 9: bitmapDoPreloadB(7); break; // TURRET_LAUNCH_FRAMES
    case 10: bitmapDoPreloadC(); break;
    case 11: bitmapDoPreloadD(); break;
    case 12: bitmapDoPreloadE(); break;
    case 13: bitmapDoPreloadF(); break;
    case 14: bitmapDoPreloadG(0); break;
    case 15: bitmapDoPreloadG(1); break;
    case 16: bitmapDoPreloadG(2); break; // MAX_PEG_SIZE
    case 17: bitmapDoPreloadH(0); break;
    case 18: bitmapDoPreloadH(1); break;
    case 19: bitmapDoPreloadH(2); break; // MAX_PEG_SIZE
    case 20: bitmapDoPreloadH2(0); break;
    case 21: bitmapDoPreloadH2(1); break;
    case 22: bitmapDoPreloadH2(2); break; // MAX_PEG_SIZE
    case 23: bitmapDoPreloadI(); break;
    case 24: bitmapDoPreloadJ(); break;
    case 25: bitmapDoPreloadK(); break;
    case 26: bitmapDoPreloadL(); break;
    case 27: bitmapDoPreloadM(0); break;
    case 28: bitmapDoPreloadM(1); break;
    case 29: bitmapDoPreloadM(2); break;
    case 30: bitmapDoPreloadM(3); break; // POND_WATER_SIZE
    case 31: IOReadVolumePreferences(); IOWriteCustomLevelInstructions(); break;
  }
  const uint32_t after = pd->system->getCurrentTimeMilliseconds();
  #ifdef DEV
  pd->system->logToConsole("Preload %i took %i ms", (int)m_preloading, (int)(after - before));
  #endif
  ++m_preloading;
  if (!IOGetIsPreloading()) { IODonePreloading(); }
}

uint16_t IOGetWaterfallBackground(const uint16_t level, const uint16_t hole) { return m_hole_background[level][hole]; }
uint16_t IOGetWaterfallForeground(const uint16_t level, const uint16_t hole) { return m_hole_foreground[level][hole]; }

uint16_t IOGetCurrentHoleWaterfallBackground(const enum GameMode_t gm) { 
  return IOGetWaterfallBackground(m_level, (gm == kTitles ? 0 : m_hole));
}

uint16_t IOGetCurrentHoleWaterfallForeground(const enum GameMode_t gm) {
  return IOGetWaterfallForeground(m_level, (gm == kTitles ? 0 : m_hole));
}

uint16_t IOGetCurrentPlayer(void) { return m_player; }

void IODoPreviousPlayer(void) {
  if (!m_player) { m_player = MAX_PLAYERS - 1; }
  else --m_player;
  #ifdef DEV
  pd->system->logToConsole("Now player %i - got to un-played hole", (int)m_player+1);
  #endif
  IODoGoToNextUnplayedLevel();
}

void IODoNextPlayer(void) {
  m_player = (m_player + 1) % MAX_PLAYERS;
  #ifdef DEV
  pd->system->logToConsole("Now player %i - got to un-played hole", (int)m_player+1);
  #endif
  IODoGoToNextUnplayedLevel();
}

void IOResetPlayerSave(const uint16_t player) {
  const uint8_t currentPlayer = m_player;
  for (int l = 0; l < MAX_LEVELS; ++l) {
    for (int h = 0; h < MAX_HOLES; ++h) {
      m_persistent_data[player][l][h] = 0; 
    }
  }
  if (player == currentPlayer) { // Resetting current player
    IODoGoToNextUnplayedLevel();
  }
}

bool IOGetIsTutorial(void) { return (m_level == 0 && m_hole == 0); }

uint16_t IOGetCurrentLevel(void) { return m_level; }

uint16_t IOGetPreviousLevel(void) {
  if (IOIsCredits()) { return MAX_LEVELS; }
  int16_t level = m_level;
  while (true) {
    if (--level < 0) { level += MAX_LEVELS; }
    if (m_hole_par[level][0]) return level;
  }
  return 0;
}

uint16_t IOGetNextLevel(void) {
  if (IOIsCredits()) { return MAX_LEVELS; }
  int16_t level = m_level;
  while (true) {
    level = (level + 1) % MAX_LEVELS;
    if (m_hole_par[level][0]) return level;
  }
  return 0;
}

uint16_t IOGetPreviousHole(void) {
  if (IOIsCredits()) { return 0; }
  int16_t hole = m_hole;
  while (true) {
    if (--hole < 0) { hole += MAX_HOLES; }
    if (m_hole_par[m_level][hole]) return hole;
  }
  return 0;
}

uint16_t IOGetNextHole(void) {
  if (IOIsCredits()) { return 0; }
  int16_t hole = (m_hole + 1) % MAX_HOLES;
  if (m_hole_par[m_level][hole]) return hole;
  return 0;
}

void IODoPreviousLevel(void) { 
  m_level = IOGetPreviousLevel();
  soundDoWaterfall(m_level);
  #ifdef DEV
  pd->system->logToConsole("Pre level called (player %i, level %i, hole %i)", (int)m_player+1, (int)m_level+1, (int)m_hole+1);
  #endif
}

void IODoNextLevel(void) {
  m_level = IOGetNextLevel();
  soundDoWaterfall(m_level);
  #ifdef DEV
  pd->system->logToConsole("Next level called (player %i, level %i, hole %i)", (int)m_player+1, (int)m_level+1, (int)m_hole+1);
  #endif
}

void IODoPreviousHole(void) { 
  m_hole = IOGetPreviousHole();
  #ifdef DEV
  pd->system->logToConsole("Prev hole called (player %i, level %i, hole %i)", (int)m_player+1, (int)m_level+1, (int)m_hole+1);
  #endif
}

void IODoNextHole(void) {
  m_hole = IOGetNextHole();
  #ifdef DEV
  pd->system->logToConsole("Next hole called (player %i, level %i, hole %i)", (int)m_player+1, (int)m_level+1, (int)m_hole+1);
  #endif
}

void IODoNextHoleWithLevelWrap(void) {
  IODoNextHole();
  if (m_hole == 0) { 
    IODoNextLevel();
    #ifdef DEV
    pd->system->logToConsole("Note: level wrap just activated");
    #endif
  }
}

void IOSetPlayer(uint16_t player) {
  m_player = player;
}

void IOSetLevelHole(uint16_t level, uint16_t hole) {
  m_level = level;
  m_hole = hole;
  soundDoWaterfall(m_level);
  #ifdef DEV
  pd->system->logToConsole("Set-level-hole called (player %i, level %i, hole %i)", (int)m_player+1, (int)m_level+1, (int)m_hole+1);
  #endif
}

void IOSetCurrentHoleScore(const uint16_t score) {
  if (!score) { return; }
  if (!m_persistent_data[m_player][m_level][m_hole] || score < m_persistent_data[m_player][m_level][m_hole]) {
    m_persistent_data[m_player][m_level][m_hole] = score;
  }
}

void IODoGoToNextUnplayedLevel(void) {
  bool found = false;
  uint16_t incomingLevel = m_level;
  // Find highest played level number
  for (int l = MAX_LEVELS-1; l >= 0; --l) {
    for (int h = MAX_HOLES-1; h >= 0; --h) {
      if (m_persistent_data[m_player][l][h] && m_hole_par[l][h]) {
        m_level = l;
        m_hole = h;
        found = true;
        break;
      }
    }
    if (found) { break; }
  }
  if (found) {
    IODoNextHoleWithLevelWrap();
    // Update the waterfall sfx here as it will only automatically trigger if ticking over from hole 9 to 0
    if (m_level != incomingLevel) { soundDoWaterfall(m_level); }
  } else {
    m_level = 0;
    m_hole = 0;
    soundDoWaterfall(m_level);
    #ifdef DEV
    pd->system->logToConsole("! No levels played - set to level 1 hole 1");
    #endif
  }
}

void IOGetLevelStatistics(const uint16_t level, uint16_t* score, uint16_t* par) {
  for (int h = 0; h < MAX_HOLES; ++h) {
    if (m_hole_par[level][h] == 0) { // h-1 was the last hole in this level
      return;
    }
    if (m_persistent_data[m_player][level][h] == 0) { // the player hasn't finished all holes in the level
      return;
      *score = 0;
      *par = 0;
    }
    *score += m_persistent_data[m_player][level][h];
    *par += m_hole_par[level][h];
  }
}

void IOGetHoleStatistics(uint16_t level, uint16_t hole, uint16_t* score, uint16_t* par) {
  *par = m_hole_par[level][hole];
  *score = m_persistent_data[m_player][level][hole];
}

uint16_t IOGetCurrentHole(void) { return m_hole; }

uint16_t IOGetPar(const uint16_t level, const uint16_t hole) { return m_hole_par[level][hole]; }

uint16_t IOGetScore(const uint16_t level, const uint16_t hole) { return m_persistent_data[m_player][level][hole]; }

uint16_t IOGetCurrentHolePar(void) { return IOGetPar(m_level, m_hole); }

uint16_t IOGetCurrentHoleScore(void) { return IOGetScore(m_level, m_hole); }

uint16_t IOGetCurrentHoleHeight(void) { return m_hole_height[m_level][m_hole]; }

enum PegSpecial_t IOGetCurrentHoleSpecial(void) { return m_hole_special[m_level][m_hole]; }

const char* IOGetCurrentHoleAuthor(void) { return m_hole_author[m_level][m_hole]; }

const char* IOGetCurrentHoleName(void) { return m_hole_name[m_level][m_hole]; }

///

int IODoRead(void* userdata, uint8_t* buf, int bufsize) {
  return pd->file->read((SDFile*)userdata, buf, bufsize);
}

void IODoWrite(void* userdata, const char* str, int len) {
  pd->file->write((SDFile*)userdata, str, len);
}

void IODecodeError(json_decoder* jd, const char* error, int linenum) {
  #ifdef DEV
  pd->system->logToConsole("decode error line %i: %s", linenum, error);
  #endif
  FSMDo(kTitlesFSM_DisplayTitles);
}

///

int IOShouldDecodeScan(json_decoder* jd, const char* key) {
  return (! strcmp(key, "body") == 0); // Decode everything in the header
}

void IODidDecodeScan(json_decoder* jd, const char* key, json_value value) {
  const int valueI = json_intValue(value);
  // pd->system->logToConsole("IODidDecodeScan deode %s", key);
  if (strcmp(key, "par") == 0) {
    m_hole_par[m_level][m_hole] = valueI;
    // pd->system->logToConsole("m_hole_par[%i][%i] = %i", m_level, m_hole, m_hole_par[m_level][m_hole]);
  } else if (strcmp(key, "foreground") == 0) {
    m_hole_foreground[m_level][m_hole] = valueI;
    // pd->system->logToConsole("m_hole_foreground[%i][%i] = %i", m_level, m_hole, m_hole_foreground[m_level][m_hole]);
  } else if (strcmp(key, "background") == 0) {
    m_hole_background[m_level][m_hole] = valueI;
    // pd->system->logToConsole("m_hole_background[%i][%i] = %i", m_level, m_hole, m_hole_background[m_level][m_hole]);
  } else if (strcmp(key, "special") == 0) {
    m_hole_special[m_level][m_hole] = (enum PegSpecial_t) valueI;
  } else if (strcmp(key, "height") == 0) {
    m_hole_height[m_level][m_hole] = valueI;
  } else if (strcmp(key, "name") == 0) {
    strcpy(m_hole_name[m_level][m_hole], json_stringValue(value)); 
    // pd->system->logToConsole("decoded name %s", m_hole_name[m_level][m_hole]);
  } else if (strcmp(key, "author") == 0) {
    strcpy(m_hole_author[m_level][m_hole], json_stringValue(value)); 
  } else if (strcmp(key, "level") == 0 && valueI-1 != m_level) {
    #ifdef DEV
    pd->system->logToConsole("IODidDecodeScan WARNING LEVEL MISSMATCH got:%i expecting:%i", valueI-1, (int)m_level);
    #endif
  } else if (strcmp(key, "hole") == 0 && valueI-1 != m_hole) {
    #ifdef DEV
    pd->system->logToConsole("IODidDecodeScan HOLE MISSMATCH got:%i expecting:%i", valueI-1, (int)m_hole);
    #endif
  }
}

void IODoScanLevels() {
  char filePath[128];
  for (int32_t l = 0; l < MAX_LEVELS+1; ++l) {
    m_level = l;
    for (int32_t h = 0; h < MAX_HOLES; ++h) {
      m_hole = h;
      snprintf(filePath, 128, "holes/level_%i_hole_%i.json", (int)l+1, (int)h+1);
      SDFile* file = pd->file->open(filePath, kFileRead);
      if (!file) {
        // Look for user-supplied levels instead
        snprintf(filePath, 128, "level_%i_hole_%i.json", (int)l+1, (int)h+1);
        file = pd->file->open(filePath, kFileReadData);
      }
      if (!file) {
        break; // Holes must be sequentially numbered, 1, 2, 3, etc.
      }
      static json_decoder jd = {
        .decodeError = IODecodeError,
        .didDecodeTableValue = IODidDecodeScan,
        .shouldDecodeTableValueForKey = IOShouldDecodeScan
      };
      #ifdef DEV
      pd->system->logToConsole("decoding header for %s", filePath);
      #endif
      pd->json->decode(&jd, (json_reader){ .read = IODoRead, .userdata = file }, NULL);
      int32_t status = pd->file->close(file);
      file = NULL;
    }
  }
  m_level = 0;
  m_hole = 0;

  // Check for save-game file
  snprintf(filePath, 128, SAVE_FORMAT_1_NAME);
  SDFile* file = pd->file->open(filePath, kFileReadData);
  if (file) { // Load save data
    int result = pd->file->read(file, m_persistent_data, SAVE_SIZE_V1);
    #ifdef DEV
    pd->system->logToConsole("Reading %i bytes of save data, result %i", SAVE_SIZE_V1, result);
    #endif
    pd->file->close(file);
    file = NULL;
  } else {
    #ifdef DEV
    pd->system->logToConsole("No save-game data at %s", SAVE_FORMAT_1_NAME);
    #endif
  }
}

void IOReadVolumePreferences() {
  char filePath[128];
  snprintf(filePath, 128, SOUND_PREFERENCE_NAME);
  SDFile* file = pd->file->open(filePath, kFileReadData);
  int32_t result = 0;
  uint8_t payload = 0;
  if (file) {
    result = pd->file->read(file, &payload, 1);
    pd->file->close(file);
    file = NULL;
  }
  if (result == 1) {
    #ifdef DEV
    pd->system->logToConsole("Read volume preference %i", payload);
    #endif
    soundSetSetting(payload);
  } else {
    #ifdef DEV
    pd->system->logToConsole("No volume preference file");
    #endif
    soundSetSetting(0); // music:true, sfx:true
  }
}

////

void IOWillDecodeLevel(json_decoder* jd, const char* key, json_value_type type) {
  // pd->system->logToConsole("IOWillDecodeLevel %s", key);
  if (strcmp(key, m_nameStatic) == 0) {
    m_decodeType = kDecodeStatic;
    return;
  }

  if (strcmp(key, m_nameElliptic) == 0) {
    m_decodeType = kDecodeElliptic;
    m_pegContainerID = 0;
    snprintf(m_namePegContainer, 128, "PegContainer%i", (int)m_pegContainerID+1);
    return;
  }

  if (strcmp(key, m_nameLinear) == 0) {
    m_decodeType = kDecodeLinear;
    m_pegContainerID = 0;
    snprintf(m_namePegContainer, 128, "PegContainer%i", (int)m_pegContainerID+1);
    m_lineContainerID = 0;
    snprintf(m_nameLineContainer, 128, "LineContainer%i", (int)m_lineContainerID+1);
    return;
  }
}

void IODidDecodeLevel(json_decoder* jd, const char* key, json_value value) {
  const int valueI = json_intValue(value);
  const float valueF = json_floatValue(value);

  if (strcmp(key, "shape") == 0) {
    // pd->system->logToConsole("IODDL - shape - DT:%i = %i", m_decodeType, valueI);
    switch (m_decodeType) {
      case kDecodeStatic:   m_static.shape   = (enum PegShape_t) valueI; return;
      case kDecodeElliptic: m_elliptic.shape = (enum PegShape_t) valueI; return;
      case kDecodeLinear:   m_linear.shape   = (enum PegShape_t) valueI; return;
    }
  }

  if (strcmp(key, "shape_override") == 0) {
  // pd->system->logToConsole("IODDL - shape_override - DT:%i cID:%i = %i", m_decodeType, m_pegContainerID, valueI);
    switch (m_decodeType) {
      case kDecodeStatic:   pd->system->error("Static shape_override?"); return;
      case kDecodeElliptic: m_elliptic.shapeOverride[m_pegContainerID] = valueI; return;
      case kDecodeLinear:   m_linear.shapeOverride[m_pegContainerID]   = valueI; return;
    }
  }

  if (strcmp(key, "n_pegs") == 0) {
    // pd->system->logToConsole("IODDL - n_pegs - DT:%i = %i", m_decodeType, valueI);
    switch (m_decodeType) {
      case kDecodeStatic:   pd->system->error("Static n_pegs?"); return;
      case kDecodeElliptic: m_elliptic.nPegs = valueI; return;
      case kDecodeLinear:   m_linear.nPegs   = valueI; return;
    }
  }

  if (strcmp(key, "n_lines") == 0) {
    // pd->system->logToConsole("IODDL - n_lines = %i", valueI);
    m_linear.nLines = valueI;
    return;
  }

  if (strcmp(key, "x") == 0) {
    // pd->system->logToConsole("IODDL - x - DT:%i = %i", m_decodeType, valueI);
    switch (m_decodeType) {
      case kDecodeStatic:   m_static.x   = valueI; return;
      case kDecodeElliptic: m_elliptic.x = valueI; return;
      case kDecodeLinear:   m_linear.x   = valueI; return;
    }
  }

  if (strcmp(key, "y") == 0) {
    // pd->system->logToConsole("IODDL - y - DT:%i = %i", m_decodeType, valueI);
    switch (m_decodeType) {
      case kDecodeStatic:   m_static.y   = valueI; return;
      case kDecodeElliptic: m_elliptic.y = valueI; return;
      case kDecodeLinear:   m_linear.y   = valueI; return;
    }
  }

  if (strcmp(key, "lc_x") == 0) {
    // pd->system->logToConsole("IODDL - lc_x - lID:%i = %i", m_lineContainerID, valueI);
    m_linear.pathX[m_lineContainerID] = valueI;
    return;
  }

  if (strcmp(key, "lc_y") == 0) {
    // pd->system->logToConsole("IODDL - lc_x - lID:%i = %i", m_lineContainerID, valueI);
    m_linear.pathY[m_lineContainerID] = valueI;
    return;
  }

  if (strcmp(key, "a") == 0) {
    // pd->system->logToConsole("IODDL - a = %i", valueI);
    m_elliptic.a = valueI;
    return;
  }

  if (strcmp(key, "b") == 0) {
    // pd->system->logToConsole("IODDL - b = %i", valueI);
    m_elliptic.b = valueI;
    return;
  }

  if (strcmp(key, "angle") == 0) {
    int v = valueI;
    if (v < 0) { v += 360; } // Needed for device only, why?
    // pd->system->logToConsole("IODDL - angle - DT:%i = %i (degToRad %f)", m_decodeType, v, degToRad(v));
    switch (m_decodeType) {
      case kDecodeStatic:   m_static.angle = degToRad(v); return;
      case kDecodeElliptic: m_elliptic.angle = degToRad(v); return;
      case kDecodeLinear:   m_linear.angle = degToRad(v); return;
    }
  }

  if (strcmp(key, "arc") == 0) {
    // pd->system->logToConsole("IODDL - arc - DT:%i = %i (degToRad %f)", m_decodeType, valueI, degToRad(valueI));
    switch (m_decodeType) {
      case kDecodeStatic:   pd->system->error("Static arc?"); return;
      case kDecodeElliptic: m_elliptic.maxAngle = degToRad(valueI); return;
      case kDecodeLinear:   m_linear.maxAngle   = degToRad(valueI); return;
    }
  }

  if (strcmp(key, "use_arc") == 0) {
    // pd->system->logToConsole("IODDL - use_arc - DT:%i = %i", m_decodeType, valueI);
    switch (m_decodeType) {
      case kDecodeStatic:   pd->system->error("Static use_arc?"); return;
      case kDecodeElliptic: m_elliptic.useArc = valueI; return;
      case kDecodeLinear:   m_linear.useArc   = valueI; return;
    }
  }

  if (strcmp(key, "speed") == 0) {
    // pd->system->logToConsole("IODDL - speed - DT:%i = %f", m_decodeType, valueF);
    switch (m_decodeType) {
      case kDecodeStatic:   pd->system->error("Static speed?"); return;
      case kDecodeElliptic: m_elliptic.speed = valueF; return;
      case kDecodeLinear:   m_linear.speed   = valueF; return;
    }
  }

  if (strcmp(key, "size") == 0) {
    // pd->system->logToConsole("IODDL - size - DT:%i = %i", m_decodeType, valueI);
    switch (m_decodeType) {
      case kDecodeStatic:   m_static.size   = valueI; return;
      case kDecodeElliptic: m_elliptic.size = valueI; return;
      case kDecodeLinear:   m_linear.size   = valueI; return;
    }
  }

  if (strcmp(key, "size_override") == 0) {
    // pd->system->logToConsole("IODDL - size_override - DT:%i cID:%i = %i", m_decodeType, m_pegContainerID, valueI);
    switch (m_decodeType) {
      case kDecodeStatic:   pd->system->error("Static size_override?"); return;
      case kDecodeElliptic: m_elliptic.sizeOverride[m_pegContainerID] = valueI; return;
      case kDecodeLinear:   m_linear.sizeOverride[m_pegContainerID]   = valueI; return;
    }
  }

  if (strcmp(key, "type") == 0) {
    // pd->system->logToConsole("IODDL - type - DT:%i cID:%i = %i", m_decodeType, m_pegContainerID, valueI);
    switch (m_decodeType) {
      case kDecodeStatic:   m_static.type                      = (enum PegType_t) valueI; return;
      case kDecodeElliptic: m_elliptic.types[m_pegContainerID] = (enum PegType_t) valueI; return;
      case kDecodeLinear:   m_linear.types[m_pegContainerID]   = (enum PegType_t) valueI; return;
    }
  }

  if (strcmp(key, "easing") == 0) {
    // pd->system->logToConsole("IODDL - easing - DT:%i = %i", m_decodeType, valueI);
    switch (m_decodeType) {
      case kDecodeStatic:   pd->system->error("Static easing?"); return;
      case kDecodeElliptic: m_elliptic.easing = (enum EasingFunction_t)valueI; return;
      case kDecodeLinear:   m_linear.easing   = (enum EasingFunction_t)valueI; return;
    }
  }

  // pd->system->logToConsole("IODDL - NOT HANDLED %s = %i", key, valueI);
}


void* IOFinishDecodeLevel(json_decoder* jd, const char* key, json_value_type type) {

  if (strcmp(key, m_nameStatic) == 0) {
    boardDoAddStatic(&m_static);
    // pd->system->logToConsole("IOFinishDecodeLevel %s", m_nameStatic);
    m_staticID++;
    snprintf(m_nameStatic, 128, "StaticControl%i", (int)m_staticID+1);
    memset(&m_static, 0, sizeof(struct StaticLoader_t));
    return NULL;
  }

  if (strcmp(key, m_nameElliptic) == 0) {
    boardDoAddWheel(&m_elliptic);
    // pd->system->logToConsole("IOFinishDecodeLevel %s", m_nameElliptic);
    m_ellipticID++;
    snprintf(m_nameElliptic, 128, "EllipticControl%i", (int)m_ellipticID+1);
    memset(&m_elliptic, 0, sizeof(struct EllipticLoader_t));
    return NULL;
  }

  if (strcmp(key, m_nameLinear) == 0) {
    boardDoAddLinear(&m_linear);
    // pd->system->logToConsole("IOFinishDecodeLevel %s", m_nameLinear);
    m_linearID++;
    snprintf(m_nameLinear, 128, "LinearControl%i", (int)m_linearID+1);
    memset(&m_linear, 0, sizeof(struct LinearLoader_t));
    return NULL;
  }

  if (strcmp(key, m_namePegContainer) == 0) {
    // pd->system->logToConsole("IOFinishDecodeLevel %s", m_namePegContainer);
    m_pegContainerID++;
    snprintf(m_namePegContainer, 128, "PegContainer%i", (int)m_pegContainerID+1);
    return NULL;
  }

  if (strcmp(key, m_nameLineContainer) == 0) {
    // pd->system->logToConsole("IOFinishDecodeLevel %s", m_nameLineContainer);
    m_lineContainerID++;
    snprintf(m_nameLineContainer, 128, "LineContainer%i", (int)m_lineContainerID+1);
    return NULL;
  }

  // pd->system->logToConsole("FINISH DECODE %s ", key);
  return NULL;
}

void IODoLoadCurrentHole() {
  char filePath[128];
  snprintf(filePath, 128, "holes/level_%i_hole_%i.json", (int)m_level+1, (int)m_hole+1);
  // Always give preference to the built in levels first
  SDFile* file = pd->file->open(filePath, kFileRead);
  if (!file) {
    // Otherwise, look for user-supplied levels instead
    snprintf(filePath, 128, "level_%i_hole_%i.json", (int)m_level+1, (int)m_hole+1);
    file = pd->file->open(filePath, kFileReadData);
  }
  if (!file) {
    #ifdef DEV
    pd->system->error("IODoLoadCurrentHole CANNOT FIND LEVEL %i %i", (int)m_level+1, (int)m_hole+1);
    #endif
    FSMDo(kTitlesFSM_DisplayTitles);
    return;
  }
  static json_decoder jd = {
    .decodeError = IODecodeError,
    .willDecodeSublist = IOWillDecodeLevel,
    .didDecodeTableValue = IODidDecodeLevel,
    .didDecodeSublist = IOFinishDecodeLevel
  };

  m_staticID = 0;
  snprintf(m_nameStatic, 128, "StaticControl%i", (int)m_staticID+1);
  memset(&m_static, 0, sizeof(struct StaticLoader_t));
  m_ellipticID = 0;
  snprintf(m_nameElliptic, 128, "EllipticControl%i", (int)m_ellipticID+1);
  memset(&m_elliptic, 0, sizeof(struct EllipticLoader_t));
  m_linearID = 0;
  snprintf(m_nameLinear, 128, "LinearControl%i", (int)m_linearID+1);
  memset(&m_linear, 0, sizeof(struct LinearLoader_t));

  pd->json->decode(&jd, (json_reader){ .read = IODoRead, .userdata = file }, NULL);
  int status = pd->file->close(file);
  file = NULL;

  // Check playable (does not apply to the credits, however)
  if (m_level != MAX_LEVELS && (boardGetRequiredPegsInPlay() == 0 || boardGetNPegs() == 0)) {
    boardDoClear();
    boardDoTestLevel();
    #ifdef DEV
    pd->system->logToConsole("Unplayable! Load test level!");
    #endif
  }

}

///

void IOWriteCustomLevelInstructions() {
  char fileContent[1024] = {0};
  const int32_t sz = snprintf(fileContent, 1024,
    "Create custom levels on https://timboe.itch.io/cascada\n"
    "Use level numbers which are not being used by the base game.\n"
    "Place levels in this folder, they should be named: level_XX_hole_YY.json\n"
    "where XX is between 1-99 and YY is between 1-9.\n"
    "Each level should have sequentially numbered holes, starting from hole 1.\n"
  );
  SDFile* file = pd->file->open(CUSTOM_LEVEL_INSTRUCTIONS_NAME, kFileWrite);
  const int32_t wrote = pd->file->write(file, fileContent, sz);
  pd->file->close(file);
  file = NULL;
}

///

void IODoSave() {
  char filePath[128];
  snprintf(filePath, 128, SAVE_FORMAT_1_NAME);
  SDFile* file = pd->file->open(filePath, kFileWrite);
  const int32_t wrote1 = pd->file->write(file, m_persistent_data, SAVE_SIZE_V1);
  #ifdef DEV
  pd->system->logToConsole("SAVE wrote %i bytes, expected to write %i bytes", wrote1, SAVE_SIZE_V1);
  #endif
  pd->file->close(file);
  file = NULL;

  // Write volume preference
  snprintf(filePath, 128, SOUND_PREFERENCE_NAME);
  file = pd->file->open(filePath, kFileWrite);
  const uint8_t soundSetting = soundGetSetting();
  const int32_t wrote2 = pd->file->write(file, &soundSetting, 1);
  pd->file->close(file);
  file = NULL;
}

///