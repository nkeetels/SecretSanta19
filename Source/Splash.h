// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: Splash.h
// Description: Simply shows the splash screen, an indexed color 8-bit image. 
// 
// Note: The fading of the image is handled in Main.c

#ifndef __SPLASH_H
#define __SPLASH_H

#include "KDTypes.h"

void Splash_Init();
void Splash_Destroy();
void Splash_Update(uint time);


#endif//__SPLASH_H
