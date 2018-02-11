#include <stdio.h>
#include <stdbool.h>
#include "global.h"
#include "CPU_LR35902.h"
#include "OPCODES.h"
#include "helper.h"


WORD getDoubleRegister(BYTE reg1, BYTE reg2) {
  return ( (reg1 << 8) | reg2 );
}



int performInstruction(gameBoy *gb) {
  int res;
  BYTE op = readMemory(gb, gb->CPU.PC);
  if (op == 0xCB)
    res = performExtendedOp(gb, op);
  else
    res = performOp(gb, op);

  return res;

}


int performOp(gameBoy *gb, BYTE op) {

  switch (op) {
    //NOP
    case 0x00: gb->CPU.PC += 1; return 4;
    //8-BIT LOAD
    //put value n into register
    case 0x06:  eightBitLoad(&(gb->CPU.regB), readMemory(gb, gb->CPU.PC + 1)); gb->CPU.PC += 2; return 8;
    case 0x0E:  eightBitLoad(&(gb->CPU.regC), readMemory(gb, gb->CPU.PC + 1)); gb->CPU.PC += 2; return 8;
    case 0x16:  eightBitLoad(&(gb->CPU.regD), readMemory(gb, gb->CPU.PC + 1)); gb->CPU.PC += 2; return 8;
    case 0x1E:  eightBitLoad(&(gb->CPU.regE), readMemory(gb, gb->CPU.PC + 1)); gb->CPU.PC += 2; return 8;
    case 0x26:  eightBitLoad(&(gb->CPU.regH), readMemory(gb, gb->CPU.PC + 1)); gb->CPU.PC += 2; return 8;
    case 0x2E:  eightBitLoad(&(gb->CPU.regL), readMemory(gb, gb->CPU.PC + 1)); gb->CPU.PC += 2; return 8;
    //copy value from register to A
    case 0x7F:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regA); gb->CPU.PC += 1; return 4;
    case 0x78:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regB); gb->CPU.PC += 1; return 4;
    case 0x79:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regC); gb->CPU.PC += 1; return 4;
    case 0x7A:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regD); gb->CPU.PC += 1; return 4;
    case 0x7B:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regE); gb->CPU.PC += 1; return 4;
    case 0x7C:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regH); gb->CPU.PC += 1; return 4;
    case 0x7D:  eightBitLoad(&(gb->CPU.regA), gb->CPU.regL); gb->CPU.PC += 1; return 4;
    case 0x7E:  eightBitLoad(&(gb->CPU.regA), readMemory(gb, getDoubleRegister(gb->CPU.regH, gb->CPU.regL))); gb->CPU.PC += 1; return 8;
    //copy value from register to B
    case 0x40:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regB); gb->CPU.PC += 1; return 4;
    case 0x41:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regC); gb->CPU.PC += 1; return 4;
    case 0x42:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regD); gb->CPU.PC += 1; return 4;
    case 0x43:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regE); gb->CPU.PC += 1; return 4;
    case 0x44:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regH); gb->CPU.PC += 1; return 4;
    case 0x45:  eightBitLoad(&(gb->CPU.regB), gb->CPU.regL); gb->CPU.PC += 1; return 4;
    case 0x46:  eightBitLoad(&(gb->CPU.regB), readMemory(gb, getDoubleRegister(gb->CPU.regH, gb->CPU.regL))); gb->CPU.PC += 1; return 8;
    //copy value from register to C
    case 0x48:  eightBitLoad(&(gb->CPU.regC), gb->CPU.regB); gb->CPU.PC += 1; return 4;
    case 0x49:  eightBitLoad(&(gb->CPU.regC), gb->CPU.regC); gb->CPU.PC += 1; return 4;
    case 0x4A:  eightBitLoad(&(gb->CPU.regC), gb->CPU.regD); gb->CPU.PC += 1; return 4;
    case 0x4B:  eightBitLoad(&(gb->CPU.regC), gb->CPU.regE); gb->CPU.PC += 1; return 4;
    case 0x4C:  eightBitLoad(&(gb->CPU.regC), gb->CPU.regH); gb->CPU.PC += 1; return 4;
    case 0x4D:  eightBitLoad(&(gb->CPU.regC), gb->CPU.regL); gb->CPU.PC += 1; return 4;
    case 0x4E:  eightBitLoad(&(gb->CPU.regC), readMemory(gb, getDoubleRegister(gb->CPU.regH, gb->CPU.regL))); gb->CPU.PC += 1; return 8;
    //copy value from register to D
    case 0x50:  eightBitLoad(&(gb->CPU.regD), gb->CPU.regB); gb->CPU.PC += 1; return 4;
    case 0x51:  eightBitLoad(&(gb->CPU.regD), gb->CPU.regC); gb->CPU.PC += 1; return 4;
    case 0x52:  eightBitLoad(&(gb->CPU.regD), gb->CPU.regD); gb->CPU.PC += 1; return 4;
    case 0x53:  eightBitLoad(&(gb->CPU.regD), gb->CPU.regE); gb->CPU.PC += 1; return 4;
    case 0x54:  eightBitLoad(&(gb->CPU.regD), gb->CPU.regH); gb->CPU.PC += 1; return 4;
    case 0x55:  eightBitLoad(&(gb->CPU.regD), gb->CPU.regL); gb->CPU.PC += 1; return 4;
    case 0x56:  eightBitLoad(&(gb->CPU.regD), readMemory(gb, getDoubleRegister(gb->CPU.regH, gb->CPU.regL))); gb->CPU.PC += 1; return 8;
    //copy value from register to E
    case 0x58:  eightBitLoad(&(gb->CPU.regE), gb->CPU.regB); gb->CPU.PC += 1; return 4;
    case 0x59:  eightBitLoad(&(gb->CPU.regE), gb->CPU.regC); gb->CPU.PC += 1; return 4;
    case 0x5A:  eightBitLoad(&(gb->CPU.regE), gb->CPU.regD); gb->CPU.PC += 1; return 4;
    case 0x5B:  eightBitLoad(&(gb->CPU.regE), gb->CPU.regE); gb->CPU.PC += 1; return 4;
    case 0x5C:  eightBitLoad(&(gb->CPU.regE), gb->CPU.regH); gb->CPU.PC += 1; return 4;
    case 0x5D:  eightBitLoad(&(gb->CPU.regE), gb->CPU.regL); gb->CPU.PC += 1; return 4;
    case 0x5E:  eightBitLoad(&(gb->CPU.regE), readMemory(gb, getDoubleRegister(gb->CPU.regH, gb->CPU.regL))); gb->CPU.PC += 1; return 8;
    //copy value from register to H
    case 0x60:  eightBitLoad(&(gb->CPU.regH), gb->CPU.regB); gb->CPU.PC += 1; return 4;
    case 0x61:  eightBitLoad(&(gb->CPU.regH), gb->CPU.regC); gb->CPU.PC += 1; return 4;
    case 0x62:  eightBitLoad(&(gb->CPU.regH), gb->CPU.regD); gb->CPU.PC += 1; return 4;
    case 0x63:  eightBitLoad(&(gb->CPU.regH), gb->CPU.regE); gb->CPU.PC += 1; return 4;
    case 0x64:  eightBitLoad(&(gb->CPU.regH), gb->CPU.regH); gb->CPU.PC += 1; return 4;
    case 0x65:  eightBitLoad(&(gb->CPU.regH), gb->CPU.regL); gb->CPU.PC += 1; return 4;
    case 0x66:  eightBitLoad(&(gb->CPU.regH), readMemory(gb, getDoubleRegister(gb->CPU.regH, gb->CPU.regL))); gb->CPU.PC += 1; return 8;
    //copy value from register to L
    case 0x68:  eightBitLoad(&(gb->CPU.regL), gb->CPU.regB); gb->CPU.PC += 1; return 4;
    case 0x69:  eightBitLoad(&(gb->CPU.regL), gb->CPU.regC); gb->CPU.PC += 1; return 4;
    case 0x6A:  eightBitLoad(&(gb->CPU.regL), gb->CPU.regD); gb->CPU.PC += 1; return 4;
    case 0x6B:  eightBitLoad(&(gb->CPU.regL), gb->CPU.regE); gb->CPU.PC += 1; return 4;
    case 0x6C:  eightBitLoad(&(gb->CPU.regL), gb->CPU.regH); gb->CPU.PC += 1; return 4;
    case 0x6D:  eightBitLoad(&(gb->CPU.regL), gb->CPU.regL); gb->CPU.PC += 1; return 4;
    case 0x6E:  eightBitLoad(&(gb->CPU.regL), readMemory(gb, getDoubleRegister(gb->CPU.regH, gb->CPU.regL))); gb->CPU.PC += 1; return 8;
    //copy value from register to (HL)
    case 0x70:  writeMemory(getDoubleRegister(gb->CPU.regH, gb->CPU.regL), gb->CPU.regB, gb); gb->CPU.PC += 1; return 8;
    case 0x71:  writeMemory(getDoubleRegister(gb->CPU.regH, gb->CPU.regL), gb->CPU.regC, gb); gb->CPU.PC += 1; return 8;
    case 0x72:  writeMemory(getDoubleRegister(gb->CPU.regH, gb->CPU.regL), gb->CPU.regD, gb); gb->CPU.PC += 1; return 8;
    case 0x73:  writeMemory(getDoubleRegister(gb->CPU.regH, gb->CPU.regL), gb->CPU.regE, gb); gb->CPU.PC += 1; return 8;
    case 0x74:  writeMemory(getDoubleRegister(gb->CPU.regH, gb->CPU.regL), gb->CPU.regH, gb); gb->CPU.PC += 1; return 8;
    case 0x75:  writeMemory(getDoubleRegister(gb->CPU.regH, gb->CPU.regL), gb->CPU.regL, gb); gb->CPU.PC += 1; return 8;
    case 0x36:  writeMemory(getDoubleRegister(gb->CPU.regH, gb->CPU.regL), readMemory(gb, gb->CPU.PC + 1), gb); gb->CPU.PC += 1; return 8;
    default: printf("%02X Not implemented\n", gb->m_rom[gb->CPU.PC]);gb->CPU.PC += 1; return 4;
  }
}

int performExtendedOp(gameBoy *gb, BYTE op) {

}
