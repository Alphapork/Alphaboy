#ifndef _CPU
#define _CPU
#include "AlphaBoy.h"

typedef struct gbCPU {

  BYTE regA;
  BYTE regF;
  BYTE regB;
  BYTE regC;
  BYTE regD;
  BYTE regE;
  BYTE regH;
  BYTE regL;



  BYTE addSubFlag;
  BYTE zeroFlag;
  BYTE halfCarryFlag;
  BYTE carryFlag;

  WORD SP;
  WORD PC;


} gbCPU;

int performInstruction(gameBoy *gb);
int performExtendedOp(gameBoy *gb);
int performOp(gameBoy *gb);






#endif
