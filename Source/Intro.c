// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: Intro.c
// Description: Simply shows an indexed color 8-bit bitmap that is distorted using hardware registers.
//
// Note: The image warping is actually handled in Main.c. Next time I should add interrupt support for effects in the
//       scripting system. 

#include "KDEngine.h"
#include "Intro.h"

extern const unsigned short introschermBitmap[19200];
extern const unsigned short introschermPal[256];

void Intro_Init()
{
  FastCopy16(((unsigned short*)0x5000000),introschermPal, 256);
}

void Intro_Destroy()
{
}

void Intro_Update(uint time)
{
  FastCopy16(g_currentBuffer, introschermBitmap, 160*120);
}
