#include "board.h"

struct Peg_t m_pegs[MAX_PEGS];

uint16_t m_nPegs = 0;

void initBoard(void) {
  memset(&m_pegs, 0, sizeof(struct Peg_t) * MAX_PEGS);
}

void randomiseBoard(void) {
  clearBoard();
  for (int i = 0; i < N_OBST; ++i) {
    const uint16_t x = rand() % WFALL_PIX_X;
    const uint16_t y = (rand() % WFALL_PIX_Y) + (2*UI_OFFSET_TOP);
    const float a = ((2*M_PIf) / 256.0f) * (rand() % 256);
    const uint16_t iPeg = m_nPegs++;
    const enum PegShape_t s = (rand() % 2 ? kPegShapeBall : kPegShapeRect);
    const enum PegMotion_t m = kPegMotionStatic;
    const enum PegType_t t = kPegTypeNormal;
    initPeg(&m_pegs[iPeg], s, m, t, x, y, a);
  }
}

void updateBoard(void) {
  for (int i = 0; i < m_nPegs; ++i) {
    updatePeg(&m_pegs[i]);
  }
}

void clearBoard(void) {
  for (int i = 0; i < m_nPegs; ++i) {
    clearPeg(&m_pegs[i]);
  }
  m_nPegs = 0;
}

void renderBoard(void) {
  for (int i = 0; i < m_nPegs; ++i) {
    pd->graphics->drawBitmap(m_pegs[i].m_bitmap, m_pegs[i].m_xBitmap, m_pegs[i].m_yBitmap, kBitmapUnflipped);
  }
}