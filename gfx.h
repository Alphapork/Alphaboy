#ifndef GFX_HEADER
#define GFX_HEADER
#include "global.h"
#include "Alphaboy.h"
#include "helper.h"

COLOUR getColour(BYTE colourNum, WORD address, gameBoy *gb);

void renderTiles(gameBoy *gb);

void renderSprites(gameBoy *gb);

void drawScanline(gameBoy *gb);

void updateGfx(int cycles, gameBoy *gb);



#endif
