#ifndef ALPHABOY_HEADER
#define ALPHABOY_HEADER
#include "CPU_LR35902.h"
#include <stdint.h>
#include <stdbool.h>
#include "./intel_8080/8080_core.h"

typedef struct gameBoy {
  BYTE *m_rom;
  //gbCPU CPU;
  e8080_cpu CPU;
  bool gbRunning;
  bool masterAllowInterupt;
  BYTE screenData[160][144][3];
  int timerCount;
  int timerStartValue;
  int dividerRegister;
  int scanlineCounter;
} gameBoy;













#endif
