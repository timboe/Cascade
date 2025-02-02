#include "sshot.h"
#include "render.h"
#include "io.h"

#ifdef TAKE_SCREENSHOTS

SDFile* m_imageFile;
LCDBitmap* m_imageBitmap;

int32_t m_yOffsetSS = 0;
bool m_ssInProgress = false;
bool m_doRender = true;

int32_t m_pixIndex;
int m_pixWidth, m_pixHeight, m_pixRowBytes;
uint8_t* m_pixData;

// Simple BMP class by u/Daeke
typedef struct {
  char magic[2];
  int32_t fileSize;
  int32_t reserved;
  int32_t contentPosition;
} BitmapFileHeader;

typedef struct {
  int32_t headerLength;
  int16_t width;
  int16_t height;
  int16_t planes;
  int16_t bitsPerPixel;
} BitmapCoreHeader;

void saveLCDBitmapHeader(SDFile* _file, LCDBitmap* _bitmap);

bool saveLCDBitmapToFile(SDFile* _file);

void ssRender(void);
void ssEncode(void) ;
void ssWrite(void);

/// ///

bool screenShotGetInProgress(void) { return m_ssInProgress; }

void screenShotDo() {
  if (!m_ssInProgress) {
    pd->system->error("called screenShotDo with no screen shot in progress");
    return;
  }
  if (m_doRender) { 
    ssRender();
  } else { 
    ssEncode();
    if (++m_yOffsetSS == WF_MAX_HEIGHT) { 
      ssWrite();
    }
  }
}

void screenShotInit(void) {
  char filePath[128];
  snprintf(filePath, 128, "round_%i_hole_%i.bmp", IOGetCurrentLevel()+1, IOGetCurrentHole()+1);
  m_imageFile = pd->file->open(filePath, kFileWrite);
 
  m_imageBitmap = pd->graphics->newBitmap(DEVICE_PIX_X, IOGetCurrentHoleHeight(), kColorWhite);

  m_pixIndex = 0;
  m_pixWidth = 0;
  m_pixHeight = 0;
  m_pixRowBytes = 0;
  m_pixData = NULL;

  m_yOffsetSS = 0;
  m_ssInProgress = true;
  m_doRender = true;

  saveLCDBitmapHeader(m_imageFile, m_imageBitmap);
}

void ssRender(void) {
  gameSetYOffset(DEVICE_PIX_Y * m_yOffsetSS, true);
  pd->graphics->setDrawOffset(0, -DEVICE_PIX_Y * m_yOffsetSS);
  renderDo(gameGetFrameCount(), FSMGet(), FSMGetGameMode());
  m_doRender = false;
}

void ssEncode(void) {
  LCDBitmap* frame = pd->graphics->getDisplayBufferBitmap(); // Not owned
  pd->graphics->pushContext(m_imageBitmap);
  pd->graphics->drawBitmap(frame, 0, DEVICE_PIX_Y * m_yOffsetSS, kBitmapUnflipped);
  pd->graphics->popContext();
  m_doRender = true;
}

void ssWrite(void) {
  pd->graphics->pushContext(m_imageBitmap);
  pd->graphics->drawLine(0, 0, DEVICE_PIX_X, 0, 4, kColorBlack);
  pd->graphics->drawLine(0, IOGetCurrentHoleHeight(), DEVICE_PIX_X, IOGetCurrentHoleHeight(), 4, kColorBlack);
  pd->graphics->popContext();
  bool finished = false;
  while (!finished) { finished = saveLCDBitmapToFile(m_imageFile); }
  pd->file->close(m_imageFile);
  m_imageFile = NULL;
  pd->graphics->freeBitmap(m_imageBitmap);
  m_imageBitmap = NULL;
  m_ssInProgress = false;
}

void saveLCDBitmapHeader(SDFile* _file, LCDBitmap* _bitmap) {
  const int32_t fileHeaderLength = 14;
  const int32_t coreHeaderLength = 12;
  const int32_t bitsInAByte = 8;
  const int32_t bytesPerColors = 3;

  int width, height, rowBytes;
  uint8_t *data;
  pd->graphics->getBitmapData(_bitmap, &width, &height, &rowBytes, NULL, &data);

  BitmapFileHeader fileHeader = (BitmapFileHeader) {
    .magic = {'B', 'M'},
    .fileSize = fileHeaderLength + coreHeaderLength + width * height * bytesPerColors,
    .contentPosition = fileHeaderLength + coreHeaderLength
  };
  BitmapCoreHeader coreHeader = (BitmapCoreHeader) {
    .headerLength = coreHeaderLength,
    .width = width,
    .height = height,
    .planes = 1,
    .bitsPerPixel = bitsInAByte * bytesPerColors
  };
  pd->file->write(_file, &fileHeader.magic, sizeof(char[2]));
  pd->file->write(_file, &fileHeader.fileSize, sizeof(int32_t));
  pd->file->write(_file, &fileHeader.reserved, sizeof(int32_t));
  pd->file->write(_file, &fileHeader.contentPosition, sizeof(int32_t));
  pd->file->write(_file, &coreHeader.headerLength, sizeof(int32_t));
  pd->file->write(_file, &coreHeader.width, sizeof(int16_t));
  pd->file->write(_file, &coreHeader.height, sizeof(int16_t));
  pd->file->write(_file, &coreHeader.planes, sizeof(int16_t));
  pd->file->write(_file, &coreHeader.bitsPerPixel, sizeof(int16_t));

  pd->graphics->getBitmapData(_bitmap, &m_pixWidth, &m_pixHeight, &m_pixRowBytes, NULL, &m_pixData);
}


bool saveLCDBitmapToFile(SDFile* _file) {
  const int32_t bitsInAByte = 8;

  const int32_t count = m_pixWidth * m_pixHeight;
  int32_t localCount = 0;

  while (true) {
    const int32_t x = m_pixIndex % m_pixWidth;
    const int32_t y = m_pixHeight - (m_pixIndex / m_pixWidth) - 1;

    const int32_t byteIndex = x / bitsInAByte + y * m_pixRowBytes;
    const int bitIndex = (1 << (bitsInAByte - 1)) >> (x % bitsInAByte);

    const uint8_t color = m_pixData[byteIndex] & bitIndex ? 0xFF : 0x00;
    pd->file->write(_file, &color, sizeof(uint8_t)); // Red
    pd->file->write(_file, &color, sizeof(uint8_t)); // Green
    pd->file->write(_file, &color, sizeof(uint8_t)); // Blue

    if (++m_pixIndex == count) return true; 
    // if (++localCount == SSHOT_PIXELS_PER_FRAME) return false;
  }

  return false;
}

#endif