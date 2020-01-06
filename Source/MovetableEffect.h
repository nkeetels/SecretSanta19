// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: MovetableEffect.h
// Description: the movetable effect is essentially a texture-coordinate rotation effect. Inputs are a 128 x 128 
//              8-bit indexed color texture, a pre-generated 320 x 256 (tunnel) lookup table, offsets for scrolling 
//              through the lookup table and a (time) index that gets added to the texture-coordinates in order to 
//              create the appearance of a moving camera.

#ifndef __MOVETABLEEFFECT_H
#define __MOVETABLEEFFECT_H

#include "KDTypes.h"

void MovetableEffect_Init();
void MovetableEffect_Destroy();
void MovetableEffect_Update(uint time);

void IWRAM_CODE MoveTable(u8 *texture, s16 *table, int xoffset, int yoffset, int t);

#endif//__MOVETABLEEFFECT_H
