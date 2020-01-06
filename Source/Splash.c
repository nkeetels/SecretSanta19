// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: Splash.c
// Description: Simply shows the splash screen, an indexed color 8-bit image. 
// 
// Note: The fading of the image is handled in Main.c

#include "Splash.h"
#include "KDEngine.h"

extern const unsigned short gba_labelBitmap[19200];
extern const unsigned short gba_labelPal[256];

void Splash_Init()
{
  REG_DISPCNT = 0x1444;

  FastCopy16(((unsigned short*)0x5000000),gba_labelPal, 256);
}

void Splash_Destroy()
{ 
}

void Splash_Update(uint time)
{
  Memcpy32(g_currentBuffer, gba_labelBitmap, 120*160*2);
}

