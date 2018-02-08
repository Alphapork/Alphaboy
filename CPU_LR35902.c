#include <stdio.h>
#include <stdbool.h>
#include "global.h"
#include "CPU_LR35902.h"
#include "OPCODES.h"


WORD getDoubleRegister(BYTE reg1, BYTE reg2) {
  return ( (reg1 << 8) | reg2 );
}



int performInstruction(gameBoy *gb) {
  int res;

  if (gb->m_rom[gb->CPU.PC] == 0xCB)
    res = performExtendedOp(gb);
  else
    res = performOp(gb);

  return res;

}


int performOp(gameBoy *gb) {

  switch (gb->m_rom[gb->CPU.PC]) {
    //NOP
    case 0x00: gb->CPU.PC += 1; return 4;
    //8-BIT LOAD
    //put value n into register
    case 0x06:  eightBitLoad(&(gb->CPU.regB), gb->m_rom[gb->CPU.PC + 1]); gb->CPU.PC += 2; return 8;
    case 0x0E:  eightBitLoad(&(gb->CPU.regC), gb->m_rom[gb->CPU.PC + 1]); gb->CPU.PC += 2; return 8;
    case 0x16:  eightBitLoad(&(gb->CPU.regD), gb->m_rom[gb->CPU.PC + 1]); gb->CPU.PC += 2; return 8;
    case 0x1E:  eightBitLoad(&(gb->CPU.regE), gb->m_rom[gb->CPU.PC + 1]); gb->CPU.PC += 2; return 8;
    case 0x26:  eightBitLoad(&(gb->CPU.regH), gb->m_rom[gb->CPU.PC + 1]); gb->CPU.PC += 2; return 8;
    case 0x2E:  eightBitLoad(&(gb->CPU.regL), gb->m_rom[gb->CPU.PC + 1]); gb->CPU.PC += 2; return 8;
    //copy value from register to A
    case 0x7F:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regA); gb->CPU.PC += 1; return 4;
    case 0x78:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regB); gb->CPU.PC += 1; return 4;
    case 0x79:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regC); gb->CPU.PC += 1; return 4;
    case 0x7A:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regD); gb->CPU.PC += 1; return 4;
    case 0x7B:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regE); gb->CPU.PC += 1; return 4;
    case 0x7C:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regH); gb->CPU.PC += 1; return 4;
    case 0x7D:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regL); gb->CPU.PC += 1; return 4;
    case 0x7E:  eightBitLoad(&(gb->CPU.regA), gb->m_rom[getDoubleRegister(gb->CPU.regH, gb->CPU.regL)]); gb->CPU.PC += 1; return 8;
    //copy value from register to B
    case 0x40:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regB); gb->CPU.PC += 1; return 4;
    case 0x41:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regC); gb->CPU.PC += 1; return 4;
    case 0x42:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regD); gb->CPU.PC += 1; return 4;
    case 0x43:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regE); gb->CPU.PC += 1; return 4;
    case 0x44:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regH); gb->CPU.PC += 1; return 4;
    case 0x45:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regL); gb->CPU.PC += 1; return 4;
    case 0x46:  eightBitLoad(&(gb->CPU.regB), gb->m_rom[getDoubleRegister(gb->CPU.regH, gb->CPU.regL)]); gb->CPU.PC += 1; return 8;

    default: printf("%02X Not implemented\n", gb->m_rom[gb->CPU.PC]);gb->CPU.PC += 1; return 4;
  }
}

int performExtendedOp(gameBoy *gb) {

}
