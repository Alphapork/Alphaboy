#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "helper.h"
#include "global.h"


COLOUR getColour(BYTE colourNum, WORD address, gameBoy *gb) {
  COLOUR res = WHITE;
  BYTE palette = readMemory(gb->m_rom, address);
  int hi = 0;
  int lo = 0;

  //which bits of the colour palette does the colour id map to?
  switch (colourNum) {
  case 0: hi = 1; lo = 0; break;
  case 1: hi = 3; lo = 2; break;
  case 2: hi = 5; lo = 4; break;
  case 3: hi = 7; lo = 6; break;
  }

  //use the palette to get the colour
  int colour = 0;
  colour = bitGetVal(palette, hi) << 1;
  colour |= bitGetVal(palette, lo);

  //convert the game colour to emultor colour
  switch (colour) {
  case 0: res = WHITE; break;
  case 1: res = LIGHT; break;
  case 2: res = DARK; break;
  case 3: res = BLACK; break;
  }

  return res;
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
  if (testBit(lcdControl,5)) {
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
    WORD tileRow = (((BYTE)(yPos/8)) *32);

    //time to start drawing the 160 horizontal pixels
    //for this scanline
    for (int pixel = 0; pixel < 160; pixel++) {
      BYTE xPos = pixel + scrollX;

      //translate the current x pos to window space if necessary
      if (usingWindow) {
	if (pixel >= windowX) {
	  xPos = pixel -windowX;
	}
      }

      //which of the 32 horizontal tiles does this xPos fall within?
      WORD tileCol = (xPos/8);
      SIGNED_WORD tileNum;

      //get the tile identity number. Remember it can be signed
      //or unsigned
      WORD tileAddress = backgroundMemory + tileRow + tileCol;
      if (unsig)
	tileNum = (BYTE)readMemory(gb->m_rom,tileAddress);
      else
	tileNum = (SIGNED_BYTE)readMemory(gb->m_rom,tileAddress);

      //deduce where this tile identifier is in memory.
      WORD tileLocation = tileData;

      if (unsig)
	tileLocation += (tileNum *16);
      else
	tileLocation += ((tileNum + 128) *16);

      //find the correct vertical line we're on of the
      //tile to get the tile data
      //from memory
      BYTE line = yPos % 8;
      line *= 2; //each vertical line takes up two bytes in memory
      BYTE data1 = readMemory(gb->m_rom, tileLocation + line);
      BYTE data2 = readMemory(gb->m_rom, tileLocation + line + 1);

      //pixel 0 in the tile is bit 7 of data1 and data2
      //Pixel 1 is bit 6 etc
      int colourBit = xPos % 8;
      colourBit -= 7;
      colourBit *= -1;

      //combine data 2 and data 1 to get colour id for this pixel
      //in the tile
      int colourNum = bitGetVal(data2, colourBit);
      colourNum <<= 1;
      colourNum |= bitGetVal(data1, colourBit); //TODO

      //now we have the colour id get the actual
      //colour from palette 0xFF47
      COLOUR col = getColour(colourNum, 0xFF47, gb); //TODO
      int red = 0;
      int green = 0;
      int blue = 0;

      //setup RGB values
      switch(col){
      case WHITE: red = 255; green = 255; blue = 255; break;
      case LIGHT: red = 0xCC; green = 0xCC; blue = 0xCC; break;
      case DARK: red = 0x77; green = 0x77; blue = 0x77; break;
      }

      int finaly = readMemory(gb->m_rom,0xFF44);

      //safety check to make sure what im about
      //to set is in the 160x144 bounds
      if ((finaly < 0) || (finaly >143) || (pixel < 0) || (pixel > 159)) {
	continue;
      }

      gb->screenData[pixel][finaly][0] = red;
      gb->screenData[pixel][finaly][1] = green;
      gb->screenData[pixel][finaly][2] = blue;
      
    }

    
}


void renderSprites(gameBoy *gb) {
  BYTE lcdControl = readMemory(gb->m_rom, 0xFF40); //TODO lcdControll -> gameBoy struct
  bool use8x16 = false;
  if (testBit(lcdControl,2)) {
    use8x16 = true;
  }
  
  for (int sprite = 0; sprite < 40; sprite++) {
    //sprite occupies 4 bytes in the sprite attributes table
    BYTE index = sprite * 4;
    BYTE yPos = readMemory(gb->m_rom, 0xFE00 + index) - 16;
    BYTE xPos = readMemory(gb->m_rom, 0xFE00 + index + 1) - 8;
    BYTE tileLocation = readMemory(gb->m_rom, 0xFE00 + index + 2);
    BYTE attributes  = readMemory(gb->m_rom, 0xFE00 + index + 3);
    
    bool yFlip = testBit(attributes,6);
    bool xFlip = testBit(attributes,5);
    
    int scanline = readMemory(gb->m_rom, 0xFF44);
    
    int ysize = 8;
    if (use8x16)
      ysize = 16;
    
    //does this sprite intercept with the scanline?
    if ((scanline >= yPos) && (scanline < (yPos +ysize))) {
	int line = scanline - yPos;
	
	//read the sprite in backwards in the y axis
	if (yFlip) {
	  line -= ysize;
	  line *= -1;
	}
	
	line *= 2; //same as for tiles
	WORD dataAddress = (0x8000 + (tileLocation * 16)) + line;
	BYTE data1 = readMemory(gb->m_rom, dataAddress);
	BYTE data2 = readMemory(gb->m_rom, dataAddress + 1);
	
	//its easier to read in from right to left as pixel 0 is
	//bit 7 in the colour data, pixel 1 is bit 6 etc
	for (int tilePixel = 7; tilePixel >= 0; tilePixel --) {
	  int colourbit = tilePixel;
	  // read the sprite in backwards for the x axis
	  if (xFlip) {
	    colourbit -= 7;
	    colourbit *= -1;
	  }
	  
	  //the rest is the same as for tiles
	  int colourNum = bitGetVal(data2, colourbit);
	  colourNum <<= 1;
	  colourNum |=bitGetVal(data1, colourbit);
	  
	  WORD colourAddress = testBit(attributes,4) ? 0xFF49:0xFF48;
	  COLOUR col = getColour(colourNum, colourAddress, gb);
	  
	  //white is transparent for sprites.
	  if (col == WHITE)
	    continue;
	  
	  
	  int red = 0;
	  int green = 0;
	  int blue = 0;
	  
	  switch(col){
	  case WHITE: red = 255; green = 255; blue = 255; break;
	  case LIGHT: red = 0xCC; green = 0xCC; blue = 0xCC; break;
	  case DARK: red = 0x77; green = 0x77; blue = 0x77; break;
	  }
	  
	  int xPix = 0 - tilePixel;
	  xPix += 7;
	  
	  int pixel = xPos +xPix;
	  
	  //sanity check
	  if ((scanline < 0) || (scanline >143) || (pixel < 0) || (pixel > 159)) {
	    continue;
	  }
	  
	  gb->screenData[pixel][scanline][0] = red;
	  gb->screenData[pixel][scanline][1] = green;
	  gb->screenData[pixel][scanline][2] = blue;	  
	  
	}
      }
      }
  }
  

void drawScanline(gameBoy *gb) {
  BYTE control = readMemory(gb->m_rom, 0xFF40);
  if (testBit(control,0)) {
    renderTiles(gb);
  }

  if (testBit(control,1)) {
    renderSprites(gb);  //TODO
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
