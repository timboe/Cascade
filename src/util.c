#include "util.h"

/// ///

char* ftos(const float value, const int16_t size, char* dest) {
  const char* tmpSign = (value < 0) ? "-" : "";
  const float tmpVal = (value < 0) ? -value : value;

  const int16_t tmpInt1 = tmpVal;
  const float tmpFrac = tmpVal - tmpInt1;
  const int16_t tmpInt2 = trunc(tmpFrac * 10000);

  char truncStr[8];
  snprintf (dest, size, "%02d", tmpInt2);
  snprintf (truncStr, 8, "%.2s", dest);

  snprintf (dest, size, "%s%d.%s", tmpSign, tmpInt1, truncStr);
  return dest;
}

void snprintf_c(char* buf, const uint8_t bufSize, const int n) {
  if (n < 1000) {
    snprintf(buf+strlen(buf), bufSize, "%d", n);
    return;
  }
  snprintf_c(buf, bufSize, n / 1000);
  snprintf(buf+strlen(buf), bufSize, ",%03d", n %1000);
}

uint8_t radToByte(const float rad) { return (rad / M_2PIf) * 256.0f; }

uint8_t angToByte(const float ang) { return (ang / 360.0f) * 256.0f; }

float degToRad(const float ang) { return ang * (M_PIf / 180.0f); }

float len(const float x1, const float x2, const float y1, const float y2) {
  return sqrtf( len2(x1, x2, y1, y2) );
}

float len2(const float x1, const float x2, const float y1, const float y2) {
  return powf(x1 - x2, 2) + powf(y1 - y2, 2);
}