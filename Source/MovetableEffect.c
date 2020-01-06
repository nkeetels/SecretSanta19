// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: MovetableEffect.c
// Description: the movetable effect is essentially a texture-coordinate rotation effect. Inputs are a 128 x 128 
//              8-bit indexed color texture, a pre-generated 320 x 256 (tunnel) lookup table, offsets for scrolling 
//              through the lookup table and a (time) index that gets added to the texture-coordinates in order to 
//              create the appearance of a moving camera.

#include "KDEngine.h"
#include "MovetableEffect.h"
#include "KDEngine.h"

extern unsigned char metal_image[16384];
extern unsigned char metal_palette[512];
extern signed short tunnellut[]; 

extern const unsigned short steefTiles[2048];
extern const unsigned short steefsprTiles[4096];
extern const unsigned short steefPal[256];

IWRAM_CODE void MoveTable(u8 *texture, s16 *table, int xoffset, int yoffset, int t)
{
	u32 *target = (u32*)(g_currentBuffer);
	s16 *uv = (yoffset * 320) + xoffset + (table - 1);
	int i = 0, j = 0;	

	#define PLOT \
	{ \
		*target++=(texture[((*(++uv))+t)&TEXSIZE]) | (texture[((*(++uv))+t)&TEXSIZE]<<8) | (texture[((*(++uv))+t)&TEXSIZE]<<16 | (texture[((*(++uv))+t)&TEXSIZE]<<24)); \     
		*target++=(texture[((*(++uv))+t)&TEXSIZE]) | (texture[((*(++uv))+t)&TEXSIZE]<<8) | (texture[((*(++uv))+t)&TEXSIZE]<<16 | (texture[((*(++uv))+t)&TEXSIZE]<<24)); \     
		*target++=(texture[((*(++uv))+t)&TEXSIZE]) | (texture[((*(++uv))+t)&TEXSIZE]<<8) | (texture[((*(++uv))+t)&TEXSIZE]<<16 | (texture[((*(++uv))+t)&TEXSIZE]<<24)); \     
		*target++=(texture[((*(++uv))+t)&TEXSIZE]) | (texture[((*(++uv))+t)&TEXSIZE]<<8) | (texture[((*(++uv))+t)&TEXSIZE]<<16 | (texture[((*(++uv))+t)&TEXSIZE]<<24)); \     
	}

	const int line = 320 - 240;
  do
	{
		i = 0;
    do
		{
			PLOT
			PLOT
			PLOT
		} while (++i < 5);
		uv += line;
    } while (++j < 160);
}

void MovetableEffect_Init()
{
  REG_DISPCNT = 0x1444;

  u16 *top = (u16*)(steefsprTiles + 2048);
  u16 *bottom = (u16*)(steefsprTiles + 0);

  OAM_Clear();

	OAM_SetSprite(0, 200-84, 0 + 0 + 0, 64, 64, 0, bottom, steefPal);
	OAM_SetSprite(1, 200-84, 0 + 80 + 0, 64, 64, 0, top, steefPal);
  OAM_SetSprite(2, 16, 160 - 44, 64, 0, 0, steefTiles, steefPal);

  OAM_CopySprites();  

  u16 *metalPal = (u16*)metal_palette;
  for (int i = 0; i < 255; i++)
  {
    u8 r = (metalPal[i] & 0x7C00) >> 10;
    u8 g = (metalPal[i] & 0x3E0) >> 5;
    u8 b = (metalPal[i] & 0x1F);   

    ((unsigned short*)0x5000000)[i] = (b << 10) | (g << 5) | r;     
  }

  const u8 palEntry = 139;
  const u16 bgColor = (palEntry << 8) | palEntry; 
  FastSet16(g_currentBuffer, bgColor, (SCREENWIDTH*SCREENHEIGHT)>>1);  
}

void MovetableEffect_Destroy()
{
}

extern const s16 SinLUT[];
#define LUTSIZE 255
void MovetableEffect_Update(uint time)
{
  int x = 40 + (SinLUT[((time >> 1) + 64) & LUTSIZE] >> 3);
  int y = 60 + (SinLUT[(time) & LUTSIZE] >> 3);

	int t = (time<<1) & LUTSIZE;

  MoveTable(metal_image, tunnellut, x, y, -t);
}

