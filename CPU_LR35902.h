#ifndef _CPU
#define _CPU
#include "global.h"






int performInstruction(gameBoy *gb);
int performExtendedOp(gameBoy *gb, BYTE op);
int performOp(gameBoy *gb, BYTE op);






#endif
