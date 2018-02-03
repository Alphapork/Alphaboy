#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "global.h"
#include "AlphaBoy.h"
#include "helper.h"
#include "CPU_LR35902.h"
#include "gfx.h"
#include "./intel_8080/8080_core.h"









int main(int argc, char const *argv[]) {

  BYTE *m_cartridge;
  m_cartridge = calloc(0x200000, sizeof(BYTE));
  if (m_cartridge == NULL) {
    printf("ERROR\n");
    return -1;
  }

  readROM(argv[1], m_cartridge);


  gameBoy alphaBoy;
  initGameBoy(&alphaBoy);
  if (alphaBoy.m_rom == NULL) {
    printf("ERROR\n");
    return -1;
  } else {
    printf("Succes!\n");
  }


   while(alphaBoy.gbRunning) {
  
     int cyclesThisFrame = 0;
  
       while ( cyclesThisFrame < MAXCYCLES) {
  
	 int opCycles = e8080_perform_instruction(&alphaBoy.CPU, alphaBoy.m_rom, 1);
	 printf("%04X	A: %02X  B: %02X  C: %02X  D: %02X  E: %02X  H: %02X  L: %02X\n",alphaBoy.CPU.PC, alphaBoy.CPU.reg_A, alphaBoy.CPU.reg_B, alphaBoy.CPU.reg_C, alphaBoy.CPU.reg_D, alphaBoy.CPU.reg_E, alphaBoy.CPU.reg_H, alphaBoy.CPU.reg_L);
        
	 cyclesThisFrame += opCycles;
         checkTimers(opCycles,&alphaBoy);
         updateGfx(opCycles, &alphaBoy);
         doInterupts(&alphaBoy);
	 
       }
       
  
  
   }

  free(m_cartridge);
  free(alphaBoy.m_rom);
  return 0;
}
