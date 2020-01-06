// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: Zoomblur.c
// Description: textured plasma with light beams.
// 
// Note: the intention was to create a zoom blur (sometimes called radial blur) using additive feedback, hence the 
//       colors are sorted according to intensity. Would would be sped up by sampling intensities and writing them
//       to a grid (shading table) and subsequently interpolating those intensities. 

#include "KDEngine.h"
#include "Zoomblur.h"

extern u8 shadingtable[20][15];

extern const unsigned short spr_creditsTiles[4096];
extern const unsigned short spr_creditsPal[256];
extern const unsigned short tex_logoBitmap[2048];

void ZoomBlur_Init()
{
  REG_DISPCNT = 0x1444;

  int i, j;
  for (i = 0; i < 256; i++)
  { 
    ((unsigned short*)0x5000000)[i] = RGB15(i, i>>1, i>>1);
  }
  unsigned short *creditsTiles2 = (unsigned short*)(spr_creditsTiles + 2048);

  OAM_Clear();
	OAM_SetSprite(0, 120-64+12, 80-32, 64, 0, 0, spr_creditsTiles, spr_creditsPal);
	OAM_SetSprite(1, 128-64+12+64-8, 80-32, 64, 0, 0, creditsTiles2, spr_creditsPal);
	OAM_SetSprite(2, 512, 512, 64, 0, 0, creditsTiles2, spr_creditsPal);

  OAM_SetBlendMode(ALPHA_BLEND, BLEND(16,0));
  OAM_CopySprites();  

  for (i = 0; i < 15; i++)
  {
    shadingtable[0][i] = 0;
    shadingtable[19][i] = 0;
  }

  for (j = 0; j < 20; j++)
  {
    shadingtable[j][0] = 0;
    shadingtable[j][14] = 0;
  }
}

void ZoomBlur_Destroy()
{
  FastSet16(((unsigned short*)0x5000000), 0, 255>>2); 
  REG_BLDCNT = 0x00ff;
  REG_BLDY = 16; 
}

void IWRAM_CODE ZoomBlur_Update(uint time)
{
  u16 *target = g_currentBuffer;

  int a = time >> 2;
  int b = time >> 1;
  int x, y,u,v,c,d,c1,c2;

  for(y=0; y<160; y++)
  {
    for(x=0; x<120; x++)
    {    
       c = SinLUT[(y + a) & LUTSIZE] >> 1; 
       d = SinLUT[(y - a) & LUTSIZE] >> 1; 
       c1 = SinLUT[(x + c) & LUTSIZE] >> 3; 
       c2 = SinLUT[(y + d) & LUTSIZE] >> 3; 
       u = (c1) & 63;
       v = (c1 + c2) & 63;
       u16 pixel = (tex_logoBitmap[((v)<<6)+u] << 8) | tex_logoBitmap[((v)<<6)+u];

       *g_currentBuffer++ = pixel;
    }
  }

  int w = 15;
  int h = 20;
  int rad, xx, yy;
  int offset = 0;

  for (x = 0; x < 5; x++)
  {
    rad = 255 / 5;
    xx = 8 + (SinLUT[(64 + x * rad + time*2)&LUTSIZE]>>6);
    yy = 10 + (SinLUT[(x * rad + time*2)&LUTSIZE]>>6);
    shadingtable[yy][xx] = 255;
    shadingtable[yy][xx+1] = 128;    
    shadingtable[yy+1][xx] = 255;    
    shadingtable[yy+1][xx+1] = 128;        
  }

  offset = 0;
  int sa, ca;
  int i, j;
  int color;
  int offsetx = 0;
  int offsety = 0;
  u16 index = 0;

  int filter[8];
  u8 halfxres = (16>>1);
  u8 halfyres = (20>>1);

  // Top-left
  for(y = 1; y < halfyres+1; y++)
  for(x = 1; x < halfxres+1; x++)
  {
    filter[0] = mod(y, h) + sa; 
    filter[1] = mod((y + 1), h) + sa;
    filter[2] = mod((y + 1), h) + sa;
    filter[3] = mod(y, h) + sa;
    filter[4] = mod(x, w) + ca;
    filter[5] = mod(x, w) + ca;
    filter[6] = mod(x + 1, w) + ca;
    filter[7] = mod(x + 1, w) + ca; 
    
    shadingtable[y][x] =
        _div((
          shadingtable[filter[0]][filter[4]]
        + shadingtable[filter[1]][filter[5]]
        + shadingtable[filter[2]][filter[6]]
        + shadingtable[filter[3]][filter[7]])
        * 64, 300);
  } 

  // Top-right
  for(y = 1; y < halfyres+1; y++)
  for(x = halfxres-2; x < 15; x++)
  {
    filter[0] = mod(y, h) + sa; 
    filter[1] = mod((y + 1), h) + sa;
    filter[2] = mod((y + 1), h) + sa;
    filter[3] = mod(y, h) + sa;
    filter[4] = mod(x, w) + ca;
    filter[5] = mod(x, w) + ca;
    filter[6] = mod(x - 1, w) + ca;
    filter[7] = mod(x - 1, w) + ca; 
    
    shadingtable[y][x] =
        _div((
          shadingtable[filter[0]][filter[4]]
        + shadingtable[filter[1]][filter[5]]
        + shadingtable[filter[2]][filter[6]]
        + shadingtable[filter[3]][filter[7]])
        * 64, 300);
  } 

  // Bottom-left
  for(y = halfyres-1; y < 20-1; y++)
  for(x = 1; x < halfxres+1; x++)
  {
    filter[0] = mod(y, h) + sa; 
    filter[1] = mod((y - 1), h) + sa;
    filter[2] = mod((y - 1), h) + sa;
    filter[3] = mod(y, h) + sa;
    filter[4] = mod(x, w) + ca;
    filter[5] = mod(x, w) + ca;
    filter[6] = mod(x + 1, w) + ca;
    filter[7] = mod(x + 1, w) + ca; 
    
    shadingtable[y][x] =
        _div((
          shadingtable[filter[0]][filter[4]]
        + shadingtable[filter[1]][filter[5]] 
        + shadingtable[filter[2]][filter[6]] * 2
        + shadingtable[filter[3]][filter[7]])
        * 58, 300);
  } 

  // Bottom-right
  for(y = halfyres-1; y < 20; y++)
  for(x = halfxres-2; x < 15; x++)
  {
    filter[0] = mod(y, h) + sa; 
    filter[1] = mod((y - 1), h) + sa;
    filter[2] = mod((y - 1), h) + sa;
    filter[3] = mod(y, h) + sa;
    filter[4] = mod(x, w) + ca;
    filter[5] = mod(x, w) + ca;
    filter[6] = mod(x - 1, w) + ca;
    filter[7] = mod(x - 1, w) + ca; 
    
    shadingtable[y][x] =
        _div((
          shadingtable[filter[0]][filter[4]]
        + shadingtable[filter[1]][filter[5]]
        + shadingtable[filter[2]][filter[6]] * 2
        + shadingtable[filter[3]][filter[7]])
        * 58, 300);
  } 

    const int DARKEN = 0;
    int u1, u2, u3, u4;
    int s1,s2;
    const int tab = 15;
    offset = 0, x = 0, y = 0;
    int tablepos = 0;
    int ymul = 0;
    int du1, du2, eu1;
    int tempu1, tempu2, tempv1, tempv2;

    for (j = 0; j < 20-1; j++)
    {
        for (i = 0; i < 15; i++)
        {  
            u1 = shadingtable[j][i];
            u2 = shadingtable[j][i+1];
            u3 = shadingtable[j+1][i];
            u4 = shadingtable[j+1][i+1];
            du1 = (((u2 - u1) << 8) >> 3);
            du2 = (((u4 - u3) << 8) >> 3);

            ymul = (j << 3) * 120 + (i << 3);

            for (y = 0; y < 8; y+=2)
            {
                s1 = u1;
                s2 = u3;
                tempu1 = tempu2 = 0;
                {                           
                    #define EVEN(add) \
                    { \  
                    tempu1 += du1; tempu2 += du2; s1 = u1 + (tempu1 >> 8); s2 = u3 + (tempu2 >> 8); \
                    eu1 = (((s2 - s1) << 8) >> 3); \
                    u = (s1 + ((eu1 * y) >> 8)); \
                    u >>= DARKEN; \
                    offset = ymul + add; \   
                    color = ((u << 8) | u); target[offset] += color; \          
                    target[offset + 120] += color; \          
                    }    

                    EVEN(0)
                    EVEN(1)
                    EVEN(2)
                    EVEN(3)
                    EVEN(4)
                    EVEN(5)
                    EVEN(6)
                    EVEN(7)
                }
                ymul += 240;
            } 
        }
        tablepos += tab;
    }   
}
