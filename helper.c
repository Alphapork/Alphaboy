#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "helper.h"
#include "global.h"

void readROM(char const *file_name, BYTE *m_cartridge) {
  FILE *gameFile;
  gameFile = fopen(file_name, "rb");
  fread(m_cartridge, 1, 0x200000, gameFile);
  fclose(gameFile);
}

void initGameBoy(gameBoy *gb) {

  gb->m_rom = calloc (0x10000, sizeof(BYTE));
  gb->gbRunning = true;
  gb->masterAllowInterupt = true;
  gb->CPU.regA = 0x01;
  gb->CPU.regF = 0xB0;
  gb->CPU.regB = 0x00;
  gb->CPU.regC = 0x13;
  gb->CPU.regD = 0x00;
  gb->CPU.regE = 0xD8;
  gb->CPU.regH = 0x01;
  gb->CPU.regL = 0x4D;
  gb->m_rom[0xFF05] = 0x00;
  gb->m_rom[0xFF06] = 0x00;
  gb->m_rom[0xFF07] = 0x00;
  gb->m_rom[0xFF10] = 0x80;
  gb->m_rom[0xFF11] = 0xBF;
  gb->m_rom[0xFF12] = 0xF3;
  gb->m_rom[0xFF14] = 0xBF;
  gb->m_rom[0xFF16] = 0x3F;
  gb->m_rom[0xFF17] = 0x00;
  gb->m_rom[0xFF19] = 0xBF;
  gb->m_rom[0xFF1A] = 0x7F;
  gb->m_rom[0xFF1B] = 0xFF;
  gb->m_rom[0xFF1C] = 0x9F;
  gb->m_rom[0xFF1E] = 0xBF;
  gb->m_rom[0xFF20] = 0xFF;
  gb->m_rom[0xFF21] = 0x00;
  gb->m_rom[0xFF22] = 0x00;
  gb->m_rom[0xFF23] = 0xBF;
  gb->m_rom[0xFF24] = 0x77;
  gb->m_rom[0xFF25] = 0xF3;
  gb->m_rom[0xFF26] = 0xF1;
  gb->m_rom[0xFF40] = 0x91;
  gb->m_rom[0xFF42] = 0x00;
  gb->m_rom[0xFF43] = 0x00;
  gb->m_rom[0xFF45] = 0x00;
  gb->m_rom[0xFF47] = 0xFC;
  gb->m_rom[0xFF48] = 0xFF;
  gb->m_rom[0xFF49] = 0xFF;
  gb->m_rom[0xFF4A] = 0x00;
  gb->m_rom[0xFF4B] = 0x00;
  gb->m_rom[0xFFFF] = 0x00;
  gb->timerCount = 1024;
  gb->timerStartValue = 1024;
  gb->dividerRegister = 0;
  gb->scanlineCounter = 456;


}



void writeMemory(WORD address, BYTE data, gameBoy *gb) {
  //read-only
  if ( address < 0x8000 ) {

  }
  //mirror to RAM
  else if ( ( address >= 0xE000 ) && ( address < 0xFE00)) {
    gb->m_rom[address] = data;
    writeMemory(address - 0x2000, data, gb);
  }
  //read-only
  else if ( (address >= 0xFEA0) && (address < 0xFEFF)) {

  }

  else if (address == TMC) {
    BYTE currentFreq = gb->m_rom[TMC] & 0x03;
    gb->m_rom[TMC] = data;
    BYTE newFreq = gb->m_rom[TMC] & 0x03;

    if (currentFreq != newFreq) {
      switch(newFreq) {
        case 0x00: gb->timerStartValue = 1024; //4096hz
        case 0x01: gb->timerStartValue = 16; //262144hz
        case 0x02: gb->timerStartValue = 64; //65536hz
        case 0x03: gb->timerStartValue = 256;//16382hz
      }
      gb->timerCount = gb->timerStartValue;
    }
  }

  else if (address == 0xFF04) {
    gb->m_rom[0xFF04] = 0;
  }

  else if (address == 0xFF44) {
    gb->m_rom[0xFF44] = 0;
  }

  else if (address == 0xFF46) {
    doDMATransfer(data, gb);
  }

  else {
    gb->m_rom[address] = data;
  }
}




bool testBit(BYTE byteToTest, int bitNumber){
  return((byteToTest >> bitNumber) & 0x01 == 0x01);
}

BYTE readMemory(BYTE *memoryToReadFrom, WORD addressToRead) {
  return memoryToReadFrom[addressToRead];
}

bool isClockEnabled(gameBoy *gb) {
  return testBit(readMemory(gb->m_rom, TMC), 2);
}

void checkTimers(int cycles, gameBoy *gb) {

  gb->dividerRegister += cycles;
  if (gb->dividerRegister >= 0xFF) {
    gb->dividerRegister = 0;
    gb->m_rom[0xFF04]++;
  }





  if (isClockEnabled) {

    gb->timerCount -= cycles;

    if (gb->timerCount <= 0) {
      gb->timerCount = gb->timerStartValue;

      if (readMemory(gb->m_rom, TIMA) == 0xFF) {
        writeMemory(TIMA, readMemory(gb->m_rom, TMA), gb);
        requestInterupt(2, gb);
      }

      else {
        writeMemory(TIMA, readMemory(gb->m_rom, TIMA) + 0x01, gb);
      }
    }

  }
}

void pushWordOnStack(gameBoy *gb) {
  printf("TODO pushWordOnStack\n");
}

BYTE bitSet(BYTE targetByte, int targetBit) {
  return (targetByte | (0x01 << targetBit));
}

BYTE bitReset(BYTE targetByte, int targetBit) {
  return (targetByte & (0xFF^(0x01 << targetBit)));
}

void requestInterupt(int idBit, gameBoy *gb) {
  BYTE request = readMemory(gb->m_rom, 0xFF0F);
  request = bitSet(request, idBit);
  writeMemory(0xFF0F,request, gb);
}


void serviceInterupt(int interupt, gameBoy *gb) {
  gb->masterAllowInterupt = false;
  BYTE request = readMemory(gb->m_rom, 0xFF0F);
  request = bitReset(request, interupt);
  writeMemory(0xFF0F,request, gb);

  pushWordOnStack(gb);

  switch (interupt) {
    case 0: gb->CPU.PC = 0x40; break;
    case 1: gb->CPU.PC = 0x48; break;
    case 2: gb->CPU.PC = 0x50; break;
    case 4: gb->CPU.PC = 0x60; break;
  }
}





void doInterupts(gameBoy *gb) {
  if (gb->masterAllowInterupt){
    BYTE request = readMemory(gb->m_rom, 0xFF0F);
    BYTE enabled = readMemory(gb->m_rom, 0xFFFF);

    if( request > 0) {
      for (int i = 0; i < 5; i++) {
        if (testBit(request, i)) {
          if (testBit(enabled, i)) {
            serviceInterupt(i, gb);
          }
        }
      }
    }
  }
}


bool isLCDEnabled(gameBoy *gb) {
  return testBit(readMemory(gb->m_rom, 0xFF40), 7);
}


void setLCDStatus(gameBoy *gb) {
  
  BYTE status = readMemory(gb->m_rom, 0xFF41);
  if (!isLCDEnabled(gb)) {
    
    
    
    //set the mode to 1 during lcd disabled and reset scanlineCounter
    gb->scanlineCounter = 456;
    gb->m_rom[0xFF44] = 0;
    status &= 252; //reset bit 0 and 1
    status = bitSet(status, 0);
    writeMemory(0xFF41, status, gb);
    return;
    
  }

  
  BYTE currentline = readMemory(gb->m_rom, 0xFF44);
  BYTE currentmode = status & 0x3;
  
  BYTE mode = 0;
  bool reqInterupt = false;

  //if vblank so set mode to 1
  if (currentline >= 144){
      mode = 1;
      status = bitSet(status,0);
      status = bitReset(status,1);
      reqInterupt = testBit(status,4);
  }
  else {

    int mode2bounds = 456 - 80;
    int mode3bounds = mode2bounds - 172;


    //mode 2
    if (gb->scanlineCounter >= mode2bounds) {
      mode = 2;
      status = bitSet(status,1);
      status = bitReset(status,0);
      reqInterupt = testBit(status,5);

    }
    //mode 3
    else if (gb->scanlineCounter >= mode2bounds) {
      mode = 3;
      status = bitSet(status,1);
      status = bitSet(status,0);

    }

    //mode 0
    else {
      mode = 0;
      status = bitReset(status,1);
      status = bitReset(status,0);
      reqInterupt = testBit(status,3);

    }

  }


  //just entered a new mode so request interupt

  if (requestInterupt && (mode !=currentmode)) {
    requestInterupt(1,gb);
  }

  //check coincident flag

  if (readMemory(gb->m_rom, 0xFF44) == readMemory(gb->m_rom, 0xFF45)) {
    status = bitSet(status, 2);

    if ( testBit(status, 6) ) 
      requestInterupt(1,gb);

  } else {
    status = bitReset(status,2);
  }

  writeMemory(0xFF41,status,gb);
  
}




void doDMATransfer(BYTE data, gameBoy *gb) {
  WORD address = data << 8;
  for (int i = 0; i < 0xA0; i++) {
    writeMemory(0xFE00 + i, readMemory(gb->m_rom, address + i), gb);
  }
}

void renderTiles(gameBoy *gb) {
  WORD tileData = 0;
  WORD backgroundMemory = 0;
  bool unsig = true;

  //where to draw the visual area and the window
  BYTE scrollY = readMemory(gb->m_rom, 0xFF42);
  BYTE scrollX = readMemory(gb->m_rom, 0xFF43);
  BYTE windowY = readMemory(gb->m_rom, 0xFF4A);
  BYTE windowX = readMemory(gb->m_rom, 0xFF4B) - 7;
  BYTE lcdControl = readMemory(gb->m_rom, 0xFF40);
  bool usingWindow = false;

  //is the window enabled?
  if (testBit(lcdControl,4) {
      //is the current scanline we're drawing
      //within the windows Y pos?
      if (windowY <= readMemory(gb->m_rom, 0xFF44) ) {	
	usingWindow = true;
      }

    }

    //which tile data are we using?
    if (testBit(lcdControl,4)) {
      tileData = 0x8000;
    }

    else {
      //IMPORTANT: this memory region uses signed
      //bytes as tile identifiers
      tileData = 0x8800;
      unsig = false;
    }

    //which background mem?
    if (usingWindow == false) {
      if (testBit(lcdControl,3)) {
	backgroundMemory = 0x9C00;
      }
      else {
	backgroundMemory = 0x9800;
      }
    }

    else {
      //which window memory?
      if (testBit(lcdControl,6)) {
	backgroundMemory = 0x9C00;
      }
      else {
	backgroundMemory = 0x9800;
      }
    }

    BYTE yPos = 0;

    //yPos is used to calculate which of 32 vertical tiles the
    //current scanline is drawing

    if (!usingWindow)
      yPos = scrollY + readMemory(gb->m_rom, 0xFF44);
    else
      yPos = readMemory(gb->m_rom, 0xFF44) - windowY;


    //which of the 8 vertical pixels of the current
    //tile is the scanline on?
    WORD tileRow = (((BYTE)(yPos/8)) *32);//TODO

    
}

void drawScanline(gameBoy *gb) {
  BYTE control = readMemory(gb->m_rom, 0xFF40);
  if (testBit(control,0)) {
    //renderTiles(gb);  //TODO
  }

  if (testBit(control,1)) {
    //renderSprites(gb);  //TODO
  }
}




void updateGfx(int cycles, gameBoy *gb) {
  setLCDStatus(gb);

  if (isLCDEnabled(gb)) { 
    gb->scanlineCounter -= cycles;
  } else return;

  if (gb->scanlineCounter <= 0) {
    gb->m_rom[0xFF44]++;
    BYTE currentScanline = readMemory(gb->m_rom, 0xFF44);

    gb->scanlineCounter = 456;

    if (currentScanline == 144) {
      requestInterupt(0, gb);
    }

    else if (currentScanline > 153) {
      gb->m_rom[0xFF44] = 0x00;
    }

    else if (currentScanline < 144) {
      drawScanline(gb);
    }
  }
}
