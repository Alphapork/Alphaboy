#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "global.h"
#include "AlphaBoy.h"
#include "helper.h"
#include "CPU_LR35902.h"
#include "gfx.h"
#define TESTROM
#define DEBUG



int main(int argc, char const *argv[]) {

  gameBoy alphaBoy;
  alphaBoy.m_cartridge = calloc(0x200000, sizeof(BYTE));
  if (alphaBoy.m_cartridge == NULL) {
    printf("ERROR\n");
    return -1;
  }

  alphaBoy.m_RAMBanks = calloc(0x8000, sizeof(BYTE));
  if (alphaBoy.m_RAMBanks == NULL) {
    printf("ERROR\n");
    return -1;
  }

  readROM(argv[1], alphaBoy.m_cartridge);



  initGameBoy(&alphaBoy);
  if (alphaBoy.m_rom == NULL) {
    printf("ERROR\n");
    return -1;
  } else {
    printf("Succes!\n");
  }

  #ifdef TESTROM
  alphaBoy.CPU.PC = 0;
  for (int i = 0; i < 1000; i++) {
    alphaBoy.m_rom[i] = alphaBoy.m_cartridge[i];
  }
  #endif

  #ifndef TESTROM
  checkMBC(&alphaBoy);
  #endif

   while(alphaBoy.gbRunning) {

     int cyclesThisFrame = 0;

       while ( cyclesThisFrame < MAXCYCLES) {

	        int opCycles = performInstruction(&alphaBoy);
          #ifdef DEBUG
          printf("PC: %04X  SP: %04X  A: %04X  B: %04X  C: %04X  D: %04X  E: %04X  H: %04X  L: %04X\n", alphaBoy.CPU.PC, alphaBoy.CPU.SP, alphaBoy.CPU.regA, alphaBoy.CPU.regB
                  , alphaBoy.CPU.regC, alphaBoy.CPU.regD, alphaBoy.CPU.regE, alphaBoy.CPU.regH, alphaBoy.CPU.regL);
          getchar();

          #endif
	        cyclesThisFrame += opCycles;
          checkTimers(opCycles,&alphaBoy);
          updateGfx(opCycles, &alphaBoy);
          doInterupts(&alphaBoy);
       }



   }

  free(alphaBoy.m_cartridge);
  free(alphaBoy.m_rom);
  free(alphaBoy.m_RAMBanks);
  return 0;
}
