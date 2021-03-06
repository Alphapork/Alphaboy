#ifndef HELPER_HEADER
#define HELPER_HEADER
#include "global.h"
#include "Alphaboy.h"


void readROM(char const *file_name, BYTE *m_cartridge);

void initGameBoy(gameBoy *gb);

void doChangeROMRAMMode(gameBoy *gb, BYTE data);

void doRAMBankChange(gameBoy *gb, BYTE data);

void doChangeHiROMBank(gameBoy *gb, BYTE data);

void doChangeLoROMBank(gameBoy *gb, BYTE data);

void doRamBankEnable(gameBoy *gb, WORD address, BYTE data);

void handleBanking(gameBoy *gb, WORD address, BYTE data);

void writeMemory(WORD address, BYTE data, gameBoy *gb);

bool testBit(WORD wordToTest, int bitNumber);

BYTE readMemory(gameBoy *gb, WORD addressToRead);

bool isClockEnabled(gameBoy *gb);

void checkTimers(int cycles, gameBoy *gb);

void pushWordOnStack(gameBoy *gb);

BYTE bitSet(BYTE targetByte, int targetBit);

BYTE bitReset(BYTE targetByte, int targetBit);

void requestInterupt(int idBit, gameBoy *gb);

void serviceInterupt(int interupt, gameBoy *gb);

void doInterupts(gameBoy *gb);

bool isLCDEnabled();

void setLCDStatus();

int bitGetVal(BYTE targetByte, int targetBit);

COLOUR getColour(BYTE colourNum, WORD address, gameBoy *gb);

void doDMATransfer(BYTE data, gameBoy *gb);

void renderTiles(gameBoy *gb);

void renderSprites(gameBoy *gb);

void drawScanline(gameBoy *gb);

void updateGfx(int cycles, gameBoy *gb);



#endif
