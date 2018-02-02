#ifndef ALPHABOY_HEADER
#define ALPHABOY_HEADER
#include "CPU_LR35902.h"


typedef struct gameBoy {
  BYTE *m_rom;
  gbCPU CPU;
  bool gbRunning;
  bool masterAllowInterupt;
  BYTE screenData[160][144][3];
  int timerCount;
  int timerStartValue;
  int dividerRegister;
  int scanlineCounter;
} gameBoy;













#endif
