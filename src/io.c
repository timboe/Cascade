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

#define MAX_CHARACTERS 32
uint16_t m_hole_par[MAX_LEVELS][MAX_HOLES] = {0};
uint16_t m_hole_height[MAX_LEVELS][MAX_HOLES] = {0};
uint16_t m_hole_foreground[MAX_LEVELS][MAX_HOLES] = {0};
uint16_t m_hole_background[MAX_LEVELS][MAX_HOLES] = {0};
enum PegSpecial_t m_hole_special[MAX_LEVELS][MAX_HOLES] = {0};
char m_hole_name[MAX_LEVELS][MAX_HOLES][MAX_CHARACTERS] = {0};
char m_hole_author[MAX_LEVELS][MAX_HOLES][MAX_CHARACTERS] = {0};

int IODoRead(void* userdata, uint8_t* buf, int bufsize);

void IODoWrite(void* userdata, const char* str, int len);

void IODecodeError(json_decoder* jd, const char* error, int linenum);

void IODoScanLevels(void);

///

int IOShouldDecodeScan(json_decoder* jd, const char* key);

void IODidDecodeScan(json_decoder* jd, const char* key, json_value value);

///

void IOWillDecodeLevel(json_decoder* jd, const char* key, json_value_type type);

void IODidDecodeLevel(json_decoder* jd, const char* key, json_value value);

void* IOFinishDecodeLevel(json_decoder* jd, const char* key, json_value_type type);

/// ///

bool IOGetIsPreloading(void) { return m_preloading != PRELOADING_STEPS; }

float IOGetPreloadingProgress(void) {
  return m_preloading / (float)PRELOADING_STEPS;
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
    case 16: bitmapDoPreloadG(2); break;
    case 17: bitmapDoPreloadG(3); break; // MAX_PEG_SIZE
    case 18: bitmapDoPreloadH(0); break;
    case 19: bitmapDoPreloadH(1); break;
    case 20: bitmapDoPreloadH(2); break;
    case 21: bitmapDoPreloadH(3); break; // MAX_PEG_SIZE
    case 22: bitmapDoPreloadI(); break;
    case 23: bitmapDoPreloadJ(); break;
    case 24: bitmapDoPreloadK(); break;
    case 25: bitmapDoPreloadL(0); break;
    case 26: bitmapDoPreloadL(1); break;
  }
  const uint32_t after = pd->system->getCurrentTimeMilliseconds();
  pd->system->logToConsole("Preload %i took %i ms", (int)m_preloading, (int)(after - before));

  ++m_preloading;
}

uint16_t IOGetWaterfallBackground(const uint16_t level, const uint16_t hole) { return m_hole_background[level][hole]; }
uint16_t IOGetWaterfallForeground(const uint16_t level, const uint16_t hole) { return m_hole_foreground[level][hole]; }

uint16_t IOGetCurrentHoleWaterfallBackground(void) { return m_hole_background[m_level][m_hole]; }
uint16_t IOGetCurrentHoleWaterfallForeground(void) { return m_hole_foreground[m_level][m_hole]; }

uint16_t IOGetCurrentPlayer(void) { return m_player; }

void IODoPreviousPlayer(void) {
  if (!m_player) { m_player = MAX_PLAYERS - 1; }
  else --m_player;
  pd->system->logToConsole("Now player %i - got to unplayed hole", (int)m_player+1);
  IODoGoToNextUnplayedLevel();
}

void IODoNextPlayer(void) {
  m_player = (m_player + 1) % MAX_PLAYERS;
  pd->system->logToConsole("Now player %i - got to unplayed hole", (int)m_player+1);
  IODoGoToNextUnplayedLevel();
}

void IOResetPlayerSave(const uint16_t player) {
  for (int l = 0; l < MAX_LEVELS; ++l) {
    for (int h = 0; h < MAX_HOLES; ++h) {
      m_persistent_data[player][l][h] = 0; 
    }
  }
}

bool IOGetIsTutorial(void) { return (m_level == 0 && m_hole == 0); }

uint16_t IOGetCurrentLevel(void) { return m_level; }

uint16_t IOGetPreviousLevel(void) {
  int16_t level = m_level;
  while (true) {
    if (--level < 0) { level += MAX_LEVELS; }
    if (m_hole_par[level][0]) return level;
  }
  return 0;
}

uint16_t IOGetNextLevel(void) {
  int16_t level = m_level;
  while (true) {
    level = (level + 1) % MAX_LEVELS;
    if (m_hole_par[level][0]) return level;
  }
  return 0;
}

uint16_t IOGetPreviousHole(void) {
  int16_t hole = m_hole;
  while (true) {
    if (--hole < 0) { hole += MAX_HOLES; }
    if (m_hole_par[m_level][hole]) return hole;
  }
  return 0;
}

uint16_t IOGetNextHole(void) {
  int16_t hole = (m_hole + 1) % MAX_HOLES;
  if (m_hole_par[m_level][hole]) return hole;
  return 0;
}

void IODoPreviousLevel(void) { 
  m_level = IOGetPreviousLevel();
  pd->system->logToConsole("Pre level called (player %i, level %i, hole %i)", (int)m_player+1, (int)m_level+1, (int)m_hole+1);
}

void IODoNextLevel(void) {
  m_level = IOGetNextLevel();
  pd->system->logToConsole("Next level called (player %i, level %i, hole %i)", (int)m_player+1, (int)m_level+1, (int)m_hole+1);
}

void IODoPreviousHole(void) { 
  m_hole = IOGetPreviousHole();
  pd->system->logToConsole("Prev hole called (player %i, level %i, hole %i)", (int)m_player+1, (int)m_level+1, (int)m_hole+1);
}

void IODoNextHole(void) {
  m_hole = IOGetNextHole();
  pd->system->logToConsole("Next hole called (player %i, level %i, hole %i)", (int)m_player+1, (int)m_level+1, (int)m_hole+1);
}

void IODoNextHoleWithLevelWrap(void) {
  IODoNextHole();
  if (m_hole == 0) { 
    IODoNextLevel();
    pd->system->logToConsole("Note: level wrap just activated");
  }
}

void IOSetLevelHole(uint16_t level, uint16_t hole) {
  m_level = level;
  m_hole = hole;
  pd->system->logToConsole("Set-level-hole called (player %i, level %i, hole %i)", (int)m_player+1, (int)m_level+1, (int)m_hole+1);
}

void IOSetCurrentHoleScore(const uint16_t score) {
  if (!score) { return; }
  if (!m_persistent_data[m_player][m_level][m_hole] || score < m_persistent_data[m_player][m_level][m_hole]) {
    m_persistent_data[m_player][m_level][m_hole] = score;
  }
}

void IODoGoToNextUnplayedLevel(void) {
  bool found = false;
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
  } else {
    m_level = 0;
    m_hole = 0;
    pd->system->logToConsole("! No levels played - set to level 1 hole 1");
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
  pd->system->logToConsole("decode error line %i: %s", linenum, error);
  FSMDo(kTitlesFSM_DisplayTitles);
}

///

int IOShouldDecodeScan(json_decoder* jd, const char* key) {
  return (! strcmp(key, "body") == 0); // Decode everything in the header
}

void IODidDecodeScan(json_decoder* jd, const char* key, json_value value) {
  // pd->system->logToConsole("IODidDecodeScan deode %s", key);
  if (strcmp(key, "par") == 0) {
    m_hole_par[m_level][m_hole] = json_intValue(value);
    // pd->system->logToConsole("m_hole_par[%i][%i] = %i", m_level, m_hole, m_hole_par[m_level][m_hole]);
  } else if (strcmp(key, "foreground") == 0) {
    m_hole_foreground[m_level][m_hole] = json_intValue(value);
    // pd->system->logToConsole("m_hole_foreground[%i][%i] = %i", m_level, m_hole, m_hole_foreground[m_level][m_hole]);
  } else if (strcmp(key, "background") == 0) {
    m_hole_background[m_level][m_hole] = json_intValue(value);
    // pd->system->logToConsole("m_hole_background[%i][%i] = %i", m_level, m_hole, m_hole_background[m_level][m_hole]);
  } else if (strcmp(key, "special") == 0) {
    m_hole_special[m_level][m_hole] = (enum PegSpecial_t) json_intValue(value);
  } else if (strcmp(key, "height") == 0) {
    m_hole_height[m_level][m_hole] = json_intValue(value);
  } else if (strcmp(key, "name") == 0) {
    strcpy(m_hole_name[m_level][m_hole], json_stringValue(value)); 
    // pd->system->logToConsole("decoded name %s", m_hole_name[m_level][m_hole]);
  } else if (strcmp(key, "author") == 0) {
    strcpy(m_hole_author[m_level][m_hole], json_stringValue(value)); 
  } else if (strcmp(key, "level") == 0 && json_intValue(value)-1 != m_level) {
    pd->system->error("IODidDecodeScan LEVEL MISSMATCH %i %i", json_intValue(value)-1, (int)m_level);
  } else if (strcmp(key, "hole") == 0 && json_intValue(value)-1 != m_hole) {
    pd->system->error("IODidDecodeScan HOLE MISSMATCH %i %i", json_intValue(value)-1, (int)m_hole);
  }
}

void IODoScanLevels() {
  char filePath[128];
  for (int32_t l = 0; l < MAX_LEVELS; ++l) {
    m_level = l;
    for (int32_t h = 0; h < MAX_HOLES; ++h) {
      m_hole = h;
      // snprintf(filePath, 128, "levels/fall_%i_hole_%i.json", 1, 1);
      snprintf(filePath, 128, "holes/level_%i_hole_%i.json", (int)l+1, (int)h+1);
      SDFile* file = pd->file->open(filePath, kFileRead);
      if (!file) {
        // Look for user-supplied levels instead
        snprintf(filePath, 128, "level_%i_hole_%i.json", (int)l+1, (int)h+1);
        SDFile* file = pd->file->open(filePath, kFileReadData);
      }
      if (!file) {
        break;
      }
      static json_decoder jd = {
        .decodeError = IODecodeError,
        .didDecodeTableValue = IODidDecodeScan,
        .shouldDecodeTableValueForKey = IOShouldDecodeScan
      };
      pd->system->logToConsole("decoding header for level %i hole %i", m_level, m_hole);
      pd->json->decode(&jd, (json_reader){ .read = IODoRead, .userdata = file }, NULL);
      int status = pd->file->close(file);
    }
  }
  m_level = 0;
  m_hole = 0;

  // Check for save-game file
  snprintf(filePath, 128, SAVE_FORMAT_1_NAME);
  SDFile* file = pd->file->open(filePath, kFileReadData);
  if (file) { // Load save data
    int result = pd->file->read(file, m_persistent_data, SAVE_SIZE_V1);
    pd->system->logToConsole("Reading %i bytes of save data, result %i", SAVE_SIZE_V1, result);
  } else {
    pd->system->logToConsole("No save-game data at %s", SAVE_FORMAT_1_NAME);
  }


  pd->system->logToConsole("TESTING setting player 2 to have finished l=1 h=1 with 5 shots");
  m_persistent_data[1][0][0] = 5;

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

  if (strcmp(key, "shape") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.shape = (enum PegShape_t) json_intValue(value); return;
      case kDecodeElliptic: m_elliptic.shape = (enum PegShape_t) json_intValue(value); return;
      case kDecodeLinear: m_linear.shape = (enum PegShape_t) json_intValue(value); return;
    }
  }

  if (strcmp(key, "shape_override") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: return;
      case kDecodeElliptic: m_elliptic.shapeOverride[m_pegContainerID] = json_intValue(value); return;
      case kDecodeLinear: m_linear.shapeOverride[m_pegContainerID] = json_intValue(value); return;
    }
  }

  if (strcmp(key, "n_pegs") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: return;
      case kDecodeElliptic: m_elliptic.nPegs = json_intValue(value); return;
      case kDecodeLinear: m_linear.nPegs = json_intValue(value); return;
    }
  }

  if (strcmp(key, "n_lines") == 0) {
    m_linear.nLines = json_intValue(value); return;
  }

  if (strcmp(key, "x") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.x = json_intValue(value); return;
      case kDecodeElliptic: m_elliptic.x = json_intValue(value); return;
      case kDecodeLinear: m_linear.x = json_intValue(value); return;
    }
  }

  if (strcmp(key, "y") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.y = json_intValue(value); return;
      case kDecodeElliptic: m_elliptic.y = json_intValue(value); return;
      case kDecodeLinear: m_linear.y = json_intValue(value); return;
    }
  }

  if (strcmp(key, "lc_x") == 0) {
    m_linear.pathX[m_lineContainerID] = json_intValue(value); return;
  }

  if (strcmp(key, "lc_y") == 0) {
    m_linear.pathY[m_lineContainerID] = json_intValue(value); return;
  }

  if (strcmp(key, "a") == 0) {
    m_elliptic.a = json_intValue(value); return;
  }

  if (strcmp(key, "b") == 0) {
    m_elliptic.b = json_intValue(value); return;
  }

  if (strcmp(key, "angle") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.angle = degToRad(json_intValue(value)); return;
      case kDecodeElliptic: m_elliptic.angle = degToRad(json_intValue(value)); return;
      case kDecodeLinear: m_linear.angle = degToRad(json_intValue(value)); return;
    }
  }

  if (strcmp(key, "arc") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: return;
      case kDecodeElliptic: m_elliptic.maxAngle = degToRad(json_intValue(value)); return;
      case kDecodeLinear: m_linear.maxAngle = degToRad(json_intValue(value)); return;
    }
  }

  if (strcmp(key, "use_arc") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: return;
      case kDecodeElliptic: m_elliptic.useArc = json_intValue(value); return;
      case kDecodeLinear: m_linear.useArc = json_intValue(value); return;
    }
  }

  if (strcmp(key, "speed") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: return;
      case kDecodeElliptic: m_elliptic.speed = json_floatValue(value); return;
      case kDecodeLinear: m_linear.speed = json_floatValue(value); return;
    }
  }

  if (strcmp(key, "size") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.size = json_intValue(value); return;
      case kDecodeElliptic: m_elliptic.size = json_intValue(value); return;
      case kDecodeLinear: m_linear.size = json_intValue(value); return;
    }
  }

  if (strcmp(key, "size_override") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: ; return;
      case kDecodeElliptic: m_elliptic.sizeOverride[m_pegContainerID] = json_intValue(value); return;
      case kDecodeLinear: m_linear.sizeOverride[m_pegContainerID] = json_intValue(value); return;
    }
  }

  if (strcmp(key, "type") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.type = (enum PegType_t) json_intValue(value); return;
      case kDecodeElliptic: m_elliptic.types[m_pegContainerID] = (enum PegType_t) json_intValue(value); return;
      case kDecodeLinear: m_linear.types[m_pegContainerID] = (enum PegType_t) json_intValue(value); return;
    }
  }

  if (strcmp(key, "easing") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: return;
      case kDecodeElliptic: m_elliptic.easing = (enum EasingFunction_t)json_intValue(value); return;
      case kDecodeLinear: m_linear.easing = (enum EasingFunction_t)json_intValue(value); return;
    }
  }

}


void* IOFinishDecodeLevel(json_decoder* jd, const char* key, json_value_type type) {

  if (strcmp(key, m_nameStatic) == 0) {
    boardDoAddStatic(&m_static);
    // pd->system->logToConsole("IOFinishDecodeLevel %s", m_nameStatic);
    m_staticID++;
    snprintf(m_nameStatic, 128, "StaticControl%i", (int)m_staticID+1);
    return NULL;
  }

  if (strcmp(key, m_nameElliptic) == 0) {
    boardDoAddWheel(&m_elliptic);
    // pd->system->logToConsole("IOFinishDecodeLevel %s", m_nameElliptic);
    m_ellipticID++;
    snprintf(m_nameElliptic, 128, "EllipticControl%i", (int)m_ellipticID+1);
    return NULL;
  }

  if (strcmp(key, m_nameLinear) == 0) {
    boardDoAddLinear(&m_linear);
    // pd->system->logToConsole("IOFinishDecodeLevel %s", m_nameLinear);
    m_linearID++;
    snprintf(m_nameLinear, 128, "LinearControl%i", (int)m_linearID+1);
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
  SDFile* file = pd->file->open(filePath, kFileRead);
  if (!file) {
    // Look for user-supplied levels instead
    snprintf(filePath, 128, "level_%i_hole_%i.json", (int)m_level+1, (int)m_hole+1);
    SDFile* file = pd->file->open(filePath, kFileReadData);
  }
  if (!file) {
    pd->system->error("IODoLoadCurrentHole CANNOT FIND LEVEL %i %i", (int)m_level+1, (int)m_hole+1);
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
  m_ellipticID = 0;
  snprintf(m_nameElliptic, 128, "EllipticControl%i", (int)m_ellipticID+1);
  m_linearID = 0;
  snprintf(m_nameLinear, 128, "LinearControl%i", (int)m_linearID+1);

  pd->json->decode(&jd, (json_reader){ .read = IODoRead, .userdata = file }, NULL);
  int status = pd->file->close(file);

  // Check playable
  if (boardGetRequiredPegsInPlay() == 0 || boardGetNPegs() == 0) {
    boardDoClear();
    boardDoTestLevel();
  }

}

///

void IODoSave() {
  char filePath[128];
  snprintf(filePath, 128, SAVE_FORMAT_1_NAME);
  SDFile* file = pd->file->open(filePath, kFileWrite);
  const int wrote = pd->file->write(file, m_persistent_data, SAVE_SIZE_V1);
  pd->system->logToConsole("SAVE wrote %i bytes, expected to write %i bytes", wrote, SAVE_SIZE_V1);
  pd->file->close(file);
}

///