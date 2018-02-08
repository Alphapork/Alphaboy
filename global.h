#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER
#include <stdbool.h>

#define CLOCKSPEED 4194304
#define TIMA 0xFF05
#define TMA 0xFF06
#define TMC 0xFF07
#define  MAXCYCLES 69905

typedef unsigned char BYTE ;
typedef char SIGNED_BYTE ;
typedef unsigned short WORD;
typedef short SIGNED_WORD;

typedef enum COLOUR { WHITE,LIGHT,DARK,BLACK } COLOUR;

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

typedef struct gameBoy {

  gbCPU CPU;
  bool gbRunning;
  bool masterAllowInterupt;
  bool MBC1;
  bool MBC2;
  BYTE currentRomBank;
  BYTE screenData[160][144][3];
  int timerCount;
  int timerStartValue;
  int dividerRegister;
  int scanlineCounter;
  BYTE *m_rom;
  BYTE *m_cartridge;
} gameBoy;







#endif
