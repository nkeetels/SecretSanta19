// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: Zoomblur.h
// Description: textured plasma with light beams.
// 
// Note: the intention was to create a zoom blur (sometimes called radial blur) using additive feedback, hence the 
//       colors are sorted according to intensity. Would would be sped up by sampling intensities and writing them
//       to a grid (shading table) and subsequently interpolating those intensities. 

#ifndef __ZOOMBLUR_H
#define __ZOOMBLUR_H

#include "KDTypes.h"

void ZoomBlur_Init();
void ZoomBlur_Destroy();
void ZoomBlur_Update(uint time);
void ZoomBlur_VBlank();

#endif//__ZOOMBLUR_H
