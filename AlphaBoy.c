#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "global.h"
#include "AlphaBoy.h"
#include "helper.h"
#include "CPU_LR35902.h"
#include "gfx.h"










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
  
	 // int opCycles = performInstruction(&alphaBoy);
  
	 // cyclesThisFrame += opCycles;
         //checkTimers(opCycles,&alphaBoy);
         //updateGfx(opCycles, &alphaBoy);
         //doInterupts(&alphaBoy);
       }
  
  
  
   }

  free(m_cartridge);
  free(alphaBoy.m_rom);
  return 0;
}
