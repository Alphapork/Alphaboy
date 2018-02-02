#ifndef HELPER_HEADER
#define HELPER_HEADER
#include "global.h"
#include "Alphaboy.h"


void readROM(char const *file_name, BYTE *m_cartridge);

void initGameBoy(gameBoy *gb);

void writeMemory(WORD address, BYTE data, gameBoy *gb);

bool testBit(BYTE byteToTest, int bitNumber);

BYTE readMemory(BYTE *memoryToReadFrom, WORD addressToRead);

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

void doDMATransfer(BYTE date, gameBoy *gb);

void drawScanline(gameBoy *gb);

void updateGfx(int cycles, gameBoy *gb);



#endif
