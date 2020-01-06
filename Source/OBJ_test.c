// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: OBJ-test.c
// Description: this effect simply renders a textured-bumpmapped cube. 
//
// Note: the displayed sprites (picture + text) are actually one large 64 x 192 tiled image with 224 colors reserved 
//       for the picture and 32 colors reserved for the text.

#include "KDEngine.h"
#include "OBJ_test.h"

#define LUTSIZE 255
extern const s16 SinLUT[];

extern const unsigned short metalBitmap[8192];
extern const unsigned short metalPal[256];

extern const unsigned short nielsTiles[4096];
extern const unsigned short nielsPal[256];
extern const unsigned short nielssprTiles[2048];

extern unsigned char bump_circles[16384];

#define MAX_VERTICES 768

void OBJ_Test_Init()
{
  u16 *top = (u16*)(nielssprTiles + 2048);
  u16 *bottom = (u16*)(nielssprTiles + 0);

  OAM_Clear();

	OAM_SetSprite(0, 0, 0 + 0 + 30, 64, 64, 0, bottom, nielsPal);
	OAM_SetSprite(1, 0, 0 + 80 + 30, 64, 64, 0, top, nielsPal);
  OAM_SetSprite(2, 240-64-16, 160 - 44, 64, 0, 0, nielsTiles, nielsPal);

  OAM_CopySprites();  

  //LoadModel(&currentModel, currentFaces, bevel_vertices, bevel_normals, (s16*)bevel_texcoords, MAX_VERTICES, 5000, 0);
  make_cube(&currentModel, currentFaces, 65, 1);
  SetTexture((u8*)metalBitmap, 0);
  SetBumpMap(bump_circles);

  for (int i = 0; i < 255; i++)
  {
    u8 r = (metalPal[i] & 0x7C00) >> 10;
    u8 g = (metalPal[i] & 0x3E0) >> 5;
    u8 b = (metalPal[i] & 0x1F);

    ((unsigned short*)0x5000000)[i] = (b << 10) | (g << 5) | r;     
  }  
  FastSet16(g_currentBuffer, (254<<8)|(254), (SCREENWIDTH*SCREENHEIGHT)>>1);   
}

void OBJ_Test_Destroy()
{
}

void OBJ_Test_Update(uint time)
{
  FastSet16(g_currentBuffer, (254<<8)|(254), (SCREENWIDTH*SCREENHEIGHT)>>1); 
  currentModel.m_position.x = 58;
  currentModel.m_position.y = -20;  
  currentModel.m_rotation.x = -time>>1;
  currentModel.m_rotation.y = -time;
  currentModel.m_rotation.z = -time>>1;

  Render(&currentModel);   
}
