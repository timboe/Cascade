#include "game.h"

// Float to string
char* ftos(const float value, const int16_t size, char* dest);

void snprintf_c(char* buf, const uint8_t bufSize, const int n);

uint8_t radToByte(const float rad);

uint8_t angToByte(const float ang);

float angToRad(const float ang);

float len(const float x1, const float x2, const float y1, const float y2);

float len2(const float x1, const float x2, const float y1, const float y2);