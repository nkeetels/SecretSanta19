// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: Intro.h
// Description: Simply shows an indexed color 8-bit bitmap that is distorted using hardware registers.
//
// Note: The image warping is actually handled in Main.c. Next time I should add interrupt support for effects in the
//       scripting system. 

#ifndef __INTRO_H
#define __INTRO_H

#include "KDTypes.h"

void Intro_Init();
void Intro_Destroy();
void Intro_Update(uint time);


#endif//__INTRO_H
