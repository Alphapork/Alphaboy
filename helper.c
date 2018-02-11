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
  gb->MBC1 = false;
  gb->MBC2 = false;
  gb->enableRAM = false; //TODO
  gb->ROMbanking = false; //TODO
  gb->currentRomBank = 1;
  gb->currentRAMBank = 0;
  gb->CPU.regA = 0x01;
  gb->CPU.regF = 0xB0;
  gb->CPU.regB = 0x00;
  gb->CPU.regC = 0x13;
  gb->CPU.regD = 0x00;
  gb->CPU.regE = 0xD8;
  gb->CPU.regH = 0x01;
  gb->CPU.regL = 0x4D;
  gb->CPU.PC = 0x100;
  gb->CPU.SP = 0xFFFE;
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

void doChangeROMRAMMode(gameBoy *gb, BYTE data) {
  BYTE newData = data & 0x1;
  gb->ROMbanking = (newData == 0)?true:false;
  if (gb->ROMbanking) {
    gb->currentRAMBank = 0;
  }
}

void doRAMBankChange(gameBoy *gb, BYTE data) {
  gb->currentRAMBank = data & 0x3;
}

void doChangeHiROMBank(gameBoy *gb, BYTE data) {
  //turn off the upper 3 bits of the current rom
  gb->currentRomBank &= 31;

  //turn of the lower 5 bits of the data
  data &= 224;
  gb->currentRomBank |= data;
  if (gb->currentRomBank == 0) gb->currentRomBank++;
}

void doChangeLoROMBank(gameBoy *gb, BYTE data) {
  if(gb->MBC2) {
    gb->currentRomBank = data & 0xF;
    if (gb->currentRomBank == 0) gb->currentRomBank++;
    return;
  }

  BYTE lower5 = data & 31;
  gb->currentRomBank &= 224; // turn of the lower 5
  gb->currentRomBank |= lower5;
  if (gb->currentRomBank == 0) gb->currentRomBank++;
}

void doRamBankEnable(gameBoy *gb, WORD address, BYTE data) {
  if (gb->MBC2) {
    if (testBit(address, 4) == 1) return;
  }

  BYTE testData = data & 0xF;
  if (testData == 0xA) {
    gb->enableRAM = true;
  }
  else if (testData == 0x0) {
    gb->enableRAM = false;
  }
}

void handleBanking(gameBoy *gb, WORD address, BYTE data) {
  //do RAM enabling
  if (address < 0x2000) {
    if (gb->MBC1 || gb->MBC2) {
      doRamBankEnable(gb, address, data);
    }
  }

  //do ROM bank change
  else if ((address >= 0x200) && (address < 0x4000)) {
    if (gb->MBC1 || gb->MBC2) {
      doChangeLoROMBank(gb, data);
    }
  }

  //do ROM or RAM bank change
  else if ((address >= 0x4000) && (address < 0x6000)) {
    //there is no rambank in mbc2 so always use rambank 0
    if (gb->MBC1) {
      if (gb->ROMbanking) {
        doChangeHiROMBank(gb, data);
      }
      else {
        doRAMBankChange(gb, data); //TODO
      }
    }
  }

  //this will change whether we are doing ROM banking
  //or RAM banking with the above if statement
  else if ((address >= 0x6000) && (address < 0x8000)) {
    if (gb->MBC1) {
      doChangeROMRAMMode(gb, data); //TODO
    }
  }
}

void writeMemory(WORD address, BYTE data, gameBoy *gb) {
  //read-only
  if ( address < 0x8000 ) {
    handleBanking(gb, address, data); //TODO
  }

  else if ( (address >= 0xA000) && (address < 0xC000)) {
    if (gb->enableRAM) {
      WORD newAddress = address - 0xA000;
      gb->m_RAMBanks[newAddress + (gb->currentRAMBank * 0x2000)] = data;
    }
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




bool testBit(WORD wordToTest, int bitNumber){
  return((wordToTest >> bitNumber) & 0x01 == 0x01);
}

BYTE readMemory(gameBoy *gb, WORD address) {

  if ((address >= 0x4000) && (address <= 0x7FFF)) {
    WORD newAddress = address - 0x4000;
    return gb->m_cartridge[newAddress + (gb->currentRomBank * 0x4000)];
  }

  else if ((address >= 0xA000) && (address <= 0xBFFF)) {
    WORD newAddress = address - 0xA000;
    return gb->m_RAMBanks[newAddress + (gb->currentRAMBank * 0x2000)];
  }
  return gb->m_rom[address];
}

bool isClockEnabled(gameBoy *gb) {
  return testBit(readMemory(gb, TMC), 2);
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

      if (readMemory(gb, TIMA) == 0xFF) {
        writeMemory(TIMA, readMemory(gb, TMA), gb);
        requestInterupt(2, gb);
      }

      else {
        writeMemory(TIMA, readMemory(gb, TIMA) + 0x01, gb);
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
  BYTE request = readMemory(gb, 0xFF0F);
  request = bitSet(request, idBit);
  writeMemory(0xFF0F,request, gb);
}


void serviceInterupt(int interupt, gameBoy *gb) {
  gb->masterAllowInterupt = false;
  BYTE request = readMemory(gb, 0xFF0F);
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
    BYTE request = readMemory(gb, 0xFF0F);
    BYTE enabled = readMemory(gb, 0xFFFF);

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
  return testBit(readMemory(gb, 0xFF40), 7);
}


void setLCDStatus(gameBoy *gb) {

  BYTE status = readMemory(gb, 0xFF41);
  if (!isLCDEnabled(gb)) {



    //set the mode to 1 during lcd disabled and reset scanlineCounter
    gb->scanlineCounter = 456;
    gb->m_rom[0xFF44] = 0;
    status &= 252; //reset bit 0 and 1
    status = bitSet(status, 0);
    writeMemory(0xFF41, status, gb);
    return;

  }


  BYTE currentline = readMemory(gb, 0xFF44);
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

  if (readMemory(gb, 0xFF44) == readMemory(gb, 0xFF45)) {
    status = bitSet(status, 2);

    if ( testBit(status, 6) )
      requestInterupt(1,gb);

  } else {
    status = bitReset(status,2);
  }

  writeMemory(0xFF41,status,gb);

}

int bitGetVal(BYTE targetByte, int targetBit) {
  if (testBit(targetByte, targetBit))
    return 1;
  else
    return 0;
}



void doDMATransfer(BYTE data, gameBoy *gb) {
  WORD address = data << 8;
  for (int i = 0; i < 0xA0; i++) {
    writeMemory(0xFE00 + i, readMemory(gb, address + i), gb);
  }
}


void checkMBC(gameBoy *gb) {
  switch (gb->m_cartridge[0x147]) {
    case 1: gb->MBC1 = true; break;
    case 2: gb->MBC1 = true; break;
    case 3: gb->MBC1 = true; break;
    case 5: gb->MBC2 = true; break;
    case 6: gb->MBC2 = true; break;
    default: break;
  }
}
