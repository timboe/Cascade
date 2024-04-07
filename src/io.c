#include "io.h"
#include "ui.h"
#include "render.h"
#include "input.h"
#include "sound.h"
#include "peg.h"

enum DecodeType_t {
  kDecodeStatic,
  kDecodeElliptic,
  kDecodeLinear
};

enum DecodeType_t m_decodeType;

struct StaticLoader_t {
  enum PegShape_t shape;
  float x;
  float y;
  uint16_t angle; // in degrees
  uint8_t size;
  enum PegType_t type;
};

char m_nameStatic[128];
uint16_t m_staticID = 0;
struct StaticLoader_t m_static = {0};

uint16_t m_player = 0;
uint16_t m_level = 0;
uint16_t m_hole = 0;

const uint16_t SAVE_SIZE_V1 = sizeof(uint32_t) * SAVE_FORMAT_1_MAX_PLAYERS * SAVE_FORMAT_1_MAX_LEVELS * SAVE_FORMAT_1_MAX_HOLES;
uint32_t m_player_score[SAVE_FORMAT_1_MAX_PLAYERS][SAVE_FORMAT_1_MAX_LEVELS][SAVE_FORMAT_1_MAX_HOLES] = {0};

uint16_t m_level_par[MAX_LEVELS][MAX_HOLES] = {0};
uint16_t m_level_foreground[MAX_LEVELS][MAX_HOLES] = {0};
uint16_t m_level_background[MAX_LEVELS][MAX_HOLES] = {0};

int doRead(void* userdata, uint8_t* buf, int bufsize);

void doWrite(void* userdata, const char* str, int len);

void decodeError(json_decoder* jd, const char* error, int linenum);


/// ///

uint16_t getWaterfallBackground(uint16_t level, uint16_t hole) { return m_level_background[level][hole];}

uint16_t getWaterfallForeground(uint16_t level, uint16_t hole) { return m_level_foreground[level][hole]; }

uint16_t getCurrentPlayer(void) { return m_player; }

void doPreviousPlayer(void) {
  if (!m_player) { m_player = MAX_PLAYERS - 1; }
  else --m_player;
  pd->system->logToConsole("Now player %i", (int)m_player);
}

void doNextPlayer(void) {
  m_player = (m_player + 1) % MAX_PLAYERS;
  pd->system->logToConsole("Now player %i", (int)m_player);
}

void resetPlayerSave(uint16_t player) {
  for (int l = 0; l < MAX_LEVELS; ++l) {
    for (int h = 0; h < MAX_HOLES; ++h) {
      m_player_score[player][l][h] = 0; 
    }
  }
}

uint16_t getCurrentLevel(void) { return m_level; }

uint16_t getPreviousLevel(void) {
  int16_t level = m_level;
  while (true) {
    if (--level < 0) { level += MAX_LEVELS; }
    if (m_level_par[level][0]) return level;
  }
  return 0;
}

uint16_t getNextLevel(void) {
  int16_t level = m_level;
  while (true) {
    level = (level + 1) % MAX_LEVELS;
    if (m_level_par[level][0]) return level;
  }
  return 0;
}

uint16_t getPreviousHole(void) {
  int16_t hole = m_hole;
  while (true) {
    if (--hole < 0) { hole += MAX_HOLES; }
    if (m_level_par[m_level][hole]) return hole;
  }
  return 0;
}

uint16_t getNextHole(void) {
  int16_t hole = (m_hole + 1) % MAX_HOLES;
  if (m_level_par[m_level][hole]) return hole;
  return 0;
}

void doPreviousLevel(void) { 
  m_level = getPreviousLevel();
  pd->system->logToConsole("Now level %i", (int)m_level);
}

void doNextLevel(void) {
  m_level = getNextLevel();
  pd->system->logToConsole("Now level %i", (int)m_level);
}

void doPreviousHole(void) { 
  m_hole = getPreviousHole();
  pd->system->logToConsole("Now hole %i", (int)m_hole);
}

void doNextHole(void) {
  m_hole = getNextHole();
  pd->system->logToConsole("Now hole %i", (int)m_hole);
}

void setHoleScore(uint16_t score) {
  if (score && score < m_player_score[m_player][m_level][m_hole]) {
    m_player_score[m_player][m_level][m_hole] = score;
  }
}

void goToNextUnplayedLevel(void) {
  bool found = false;
  // Find highest played level number
  for (int l = MAX_LEVELS-1; l >= 0; --l) {
    for (int h = MAX_HOLES-1; h >= 0; --h) {
      if (m_player_score[m_player][l][h] && m_level_par[l][h]) {
        m_level = l;
        m_hole = h;
        found = true;
        break;
      }
    }
    if (found) { break; }
  }
  if (found) {
    doNextHole();
    if (m_hole == 0) {
      doNextLevel();
    }
  } else {
    m_level = 0;
    m_hole = 0;
    pd->system->logToConsole("Now hole 0");
    pd->system->logToConsole("Now level 0");
  }
}

void getLevelStatistics(uint16_t level, uint16_t* score, uint16_t* par) {
  for (int h = 0; h < MAX_HOLES; ++h) {
    if (m_level_par[level][h] == 0) { // h-1 was the last hole in this level
      return;
    }
    if (m_player_score[m_player][level][h] == 0) { // the player hasn't finished all holes in the level
      return;
      *score = 0;
      *par = 0;
    }
    *score += m_player_score[m_player][level][h];
    *par += m_level_par[level][h];
  }
}

void getHoleStatistics(uint16_t level, uint16_t hole, uint16_t* score, uint16_t* par) {
  *par = m_level_par[level][hole];
  *score = m_player_score[m_player][level][hole];
}

uint16_t getCurrentHole(void) { return m_hole; }

uint16_t getPar(uint16_t level, uint16_t hole) { return m_level_par[level][hole]; }

uint16_t getScore(uint16_t level, uint16_t hole) { return m_player_score[m_player][level][hole]; }

uint16_t getCurrentHolePar(void) { return getPar(m_level, m_hole); }

uint16_t getCurrentHoleScore(void) { return getScore(m_level, m_hole); }


int doRead(void* userdata, uint8_t* buf, int bufsize) {
  return pd->file->read((SDFile*)userdata, buf, bufsize);
}

void doWrite(void* userdata, const char* str, int len) {
  pd->file->write((SDFile*)userdata, str, len);
}

void decodeError(json_decoder* jd, const char* error, int linenum) {
  pd->system->logToConsole("decode error line %i: %s", linenum, error);
}


///

int shouldDecodeScan(json_decoder* jd, const char* key) {
  return (! strcmp(key, "body") == 0); // Decode everything in the header
}

void didDecodeScan(json_decoder* jd, const char* key, json_value value) {
  pd->system->logToConsole("didDecodeScan deode %s", key);
  if (strcmp(key, "par") == 0) {
    m_level_par[m_level][m_hole] = json_intValue(value);
    pd->system->logToConsole("m_level_par[%i][%i] = %i", m_level, m_hole, m_level_par[m_level][m_hole]);
  } else if (strcmp(key, "foreground") == 0) {
    m_level_foreground[m_level][m_hole] = json_intValue(value);
    pd->system->logToConsole("m_level_foreground[%i][%i] = %i", m_level, m_hole, m_level_foreground[m_level][m_hole]);
  } else if (strcmp(key, "background") == 0) {
    m_level_background[m_level][m_hole] = json_intValue(value);
    pd->system->logToConsole("m_level_background[%i][%i] = %i", m_level, m_hole, m_level_background[m_level][m_hole]);
  } else if (strcmp(key, "level") == 0 && json_intValue(value)-1 != m_level) {
    pd->system->error("didDecodeScan LEVEL MISSMATCH %i %i", json_intValue(value)-1, (int)m_level);
  } else if (strcmp(key, "hole") == 0 && json_intValue(value)-1 != m_hole) {
    pd->system->error("didDecodeScan HOLE MISSMATCH %i %i", json_intValue(value)-1, (int)m_hole);
  }
}

void scanLevels() {
  char filePath[128];
  for (int32_t l = 0; l < MAX_LEVELS; ++l) {
    m_level = l;
    // bool abort = false;
    for (int32_t h = 0; h < MAX_HOLES; ++h) {
      m_hole = h;
      // snprintf(filePath, 128, "levels/fall_%i_hole_%i.json", 1, 1);
      snprintf(filePath, 128, "levels/level_%i_hole_%i.json", (int)l+1, (int)h+1);
      SDFile* file = pd->file->open(filePath, kFileRead);
      if (!file && m_level >= 10) {
        // Look for user-supplied levels instead
        snprintf(filePath, 128, "level_%i_hole_%i.json", (int)l+1, (int)h+1);
        SDFile* file = pd->file->open(filePath, kFileReadData);
      }
      if (!file) {
        // if (!h) { abort = true; } // If missing the first fall of this level then stop looking here
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
    // if (abort) { break; }
  }
  m_level = 0;
  m_hole = 0;

  // Check for save-game file
  snprintf(filePath, 128, SAVE_FORMAT_1_NAME);
  SDFile* file = pd->file->open(filePath, kFileRead);
  if (file) { // Load save data
    int result = pd->file->read(file, m_player_score, SAVE_SIZE_V1);
    pd->system->logToConsole("Reading %i bytes of save data, result %i", SAVE_SIZE_V1, result);
  } else {
    pd->system->logToConsole("No save-game data at %s", SAVE_FORMAT_1_NAME);
  }


  pd->system->logToConsole("TESTING setting player 2 to have finished l=1 h=1 with 5 shots");
  m_player_score[1][0][0] = 5;

}

////

void willDecode(json_decoder* jd, const char* key, json_value_type type) {

  if (strcmp(key, m_nameStatic) == 0) {
    pd->system->logToConsole("willDecode Decoding %s", m_nameStatic);
    m_decodeType = kDecodeStatic;
    return;
  }

}


void didDecode(json_decoder* jd, const char* key, json_value value) {

  if (strcmp(key, "x") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.x = json_intValue(value); return;
    }
  }

  if (strcmp(key, "y") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.y = json_intValue(value); return;
    }
  }

  if (strcmp(key, "angle") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.angle = json_intValue(value); return;
    }
  }

  if (strcmp(key, "size") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.size = json_intValue(value); return;
    }
  }

  if (strcmp(key, "type") == 0) {
    switch (m_decodeType) {
      case kDecodeStatic: m_static.type = (enum PegType_t)json_intValue(value); return;
    }
  }

}


void* finishDecode(json_decoder* jd, const char* key, json_value_type type) {

  if (strcmp(key, m_nameStatic) == 0) {
    pd->system->logToConsole("finishDecode %s", m_nameStatic);
    m_staticID++;
    snprintf(m_nameStatic, 128, "StaticControl%i", (int)m_staticID+1);
    return NULL;
  }

  return NULL;
}

void loadCurrentHole() {
  char filePath[128];
  snprintf(filePath, 128, "levels/level_%i_hole_%i.json", (int)m_level+1, (int)m_hole+1);
  SDFile* file = pd->file->open(filePath, kFileRead);
  if (!file && m_level >= 10) {
    // Look for user-supplied levels instead
    snprintf(filePath, 128, "level_%i_hole_%i.json", (int)m_level+1, (int)m_hole+1);
    SDFile* file = pd->file->open(filePath, kFileReadData);
  }
  if (!file) {
    pd->system->error("loadCurrentHole CANNOT FIND LEVEL %i %i", (int)m_level+1, (int)m_hole+1);
    doFSM(kTitlesFSM_DisplayTitles);
    return;
  }
  static json_decoder jd = {
    .decodeError = decodeError,
    .willDecodeSublist = willDecode,
    .didDecodeTableValue = didDecode,
    .didDecodeSublist = finishDecode
  };



  m_staticID = 0;
  snprintf(m_nameStatic, 128, "StaticControl%i", (int)m_staticID+1);

  pd->json->decode(&jd, (json_reader){ .read = doRead, .userdata = file }, NULL);
  int status = pd->file->close(file);

}

///

void doSave() {
  char filePath[128];
  snprintf(filePath, 128, SAVE_FORMAT_1_NAME);
  SDFile* file = pd->file->open(filePath, kFileWrite);
  const int wrote = pd->file->write(file, m_player_score, SAVE_SIZE_V1);
  pd->system->logToConsole("SAVE wrote %i bytes, expected to write %i bytes", wrote, SAVE_SIZE_V1);
  pd->file->close(file);
}

///

