// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: Freedir.c
// Description: Raytracing the intersection with a plane, otherwise known as free-direction planes in the demoscene. 
//              Instead of interpolating between colors this effect interpolates between texture-coordinates, which 
//              results in a sharp image with unique pixel color lookups. Essentially this means that only 15 x 20 
//              grid points have to be raytraced instead of a full-screen image. 
//              
//
// Note: In addition to saving texture-coordinates to a UV table I also save the depth to a shading table so that 
//       I can shift the palette to a darker shade when the distance increases. However, the interpolation reveals
//       artifacts when the delta between values becomes large (e.g. when overshooting max-UV). That's why the 
//       camera (ray-direction) rotates but the ray-origin doesn't translate. 
//
// Note: The shading is honestly a bit of a hack. I created a tool in Python that quantizes an indexed color 8-bit
//       bitmap image to 32 colors and create 7 shades of that 32-color palette. This is why g_paletteOffset exists. 
//
// Note: this effect can also be used as a rotozoomer when directly facing a plane (see ROTOZOOM_BOUNCE flag)
//
// Note: the displayed sprites (picture + text) are actually one large 64 x 192 tiled image with 224 colors reserved 
//       for the picture and 32 colors reserved for the text.

#include "Freedir.h"
#include "KDEngine.h"

#define PLANES

int u, v, z;

extern const unsigned short mike2Tiles[2048];
extern const unsigned short mike2Pal[256];
extern const unsigned short mike2sprTiles[2048];

extern const unsigned short tiling32Bitmap[8192];
extern const unsigned short tiling32Pal[256];

u16 EWRAM_DATA uvtable[20 * 15];
u8 EWRAM_DATA shadingtable[20 * 15];

void Freedir_Init()
{
  u16 *top = (u16*)(mike2sprTiles + 2048);
  u16 *bottom = (u16*)(mike2sprTiles + 0);

  OAM_Clear();

	OAM_SetSprite(0, 0, 0 + 0 + 16, 64, 64, 0, bottom, mike2Pal);
	OAM_SetSprite(1, 0, 0 + 80 + 16, 64, 64, 0, top, mike2Pal);
  OAM_SetSprite(2, 240-64-16, 16, 64, 0, 0, mike2Tiles, mike2Pal);

  OAM_CopySprites();  

  FastCopy16(((unsigned short*)0x5000000),tiling32Pal, 256); 

  const u8 palEntry = 255;
  const u16 bgColor = (palEntry << 8) | palEntry;
  FastSet16(g_currentBuffer, bgColor, (SCREENWIDTH*SCREENHEIGHT)>>1);  
}

void Freedir_Destroy()
{
}

int sgn(int x)
{
    return (x > 0) - (x < 0);
}

#define ROTOZOOM_BOUNCE 1 << 1

void IWRAM_CODE Freedir_Update(uint time)
{
    const u8 palEntry = 255;
    const u16 bgColor = (palEntry << 8) | palEntry;
    FastSet16((u16*)g_currentBuffer, bgColor, 160*120);  

    u8 flags = ROTOZOOM_BOUNCE;
    int bounce = 0;
    Matrix3x3 m;

    if (time > 225)
    {
      flags = 0;
    }

    if (flags & ROTOZOOM_BOUNCE)
    {
      Rotate(m, time>>2, 64, time>>3);
      bounce = abs(SinLUT[(time>>1)&LUTSIZE]) >> 3;
    }
    else
    {
      Rotate(m, time>>1, 64 + ((time-300))>>1, time>>2);
    }
 
    u16 *target32 = g_currentBuffer;

    int x, y;
    int i, j;
    int uv, offset = 0, voffset = 0;
    int ox, oy, oz;
    int dx, dy, dz;
    int nx, ny, nz;

    int ix, iy, iz, t, ilen;
    u8 color;

    u8 *tex = (u8*)tiling32Bitmap;
    const int tab = 15;

    int max_t = 0;

    ox = 0;
    oy = 0;
    oz = 0;                

    j = 0;
    //for (j = 0; j < 160; j+=8)
    do 
    {
        i = 0;
        //for (i = 0; i < 120; i+=8)
        do
        {                                        
            dx = (((i<<1) - 120) << 8) >> 7;
            dy = ((j - 80) << 8) >> 7;
            dz = 1 << 8;

            nx = (m[0][0] * dx + m[0][1] * dy + m[0][2] * dz) >> 8;
            ny = (m[1][0] * dx + m[1][1] * dy + m[1][2] * dz) >> 8;
            nz = (m[2][0] * dx + m[2][1] * dy + m[2][2] * dz) >> 8;

            ilen = _sqrt(nx * nx + ny * ny + nz * nz);

            nx = _div(nx << 8, ilen);
            ny = _div(ny << 8, ilen);
            nz = _div(nz << 8, ilen);

            dx = (nx * ilen) >> 8;
            dy = (ny * ilen) >> 8;
            dz = (nz * ilen) >> 8;

            if (dy == 0)
                t = 65535;
            else        
                t = _div((sgn(dy) << 14) - oy, dy) + bounce;

            ix = ox + ((int)(dx * t) >> 8);
            iz = oz + ((int)(dz * t) >> 8);

            u = (int)(abs(ix)<<0);
            v = (int)(abs(iz)<<0);

            uv = ((v << 8) + u); // remove sanity check when building
            
            t >>= 7;  
            t = min(t, 6);

            shadingtable[offset] = (t << 5);            

            u16 uvt = uv;
            uvtable[offset] = uvt;

            offset++;
            voffset++;
            i += 8;
        } while (i < 120);
        voffset+=120;
        j += 8;
    } while (j < 160);

    int u1, v1, u2, v2, u3, v3, u4, v4;
    int s1, t1, s2, t2, texofs = 0;

    int tablepos = 0;
    int ymul = 0;
    int du1, du2, dv1, dv2, eu1, eu2, d1, d2, d3, d4, zdu1, zdu2, zdv1, zdv2;
    int tempu1, tempu2, tempv1, tempv2, tempz1, tempz2 ,z1, z2, z, vz1;

    int scroll_u = 0;
    int scroll_v = 0;

    if (flags & ROTOZOOM_BOUNCE)
    {
      scroll_u -= time >> 1;
      scroll_v -= time >> 1;
    }

   for (j = 1; j < 20-1; j++)
    {
        for (i = 1; i < 15-1; i++)
        {   
            uv = uvtable[tablepos + i];
            u1 = (uv >> 8) & 0xFF;
            v1 = uv & 0xFF;
            d1 = shadingtable[tablepos + i];

            uv = uvtable[tablepos + i + 1];
            u2 = (uv >> 8) & 0xFF;
            v2 = uv & 0xFF;
            d2 = shadingtable[tablepos + i + 1];

            uv = uvtable[tablepos + i + tab];
            u3 = (uv >> 8) & 0xFF;
            v3 = uv & 0xFF;
            d3 = shadingtable[tablepos + i + tab];

            uv = uvtable[tablepos + i + tab + 1];  
            u4 = (uv >> 8) & 0xFF;
            v4 = uv & 0xFF;
            d4 = shadingtable[tablepos + i + tab + 1];

            du1 = (((u2 - u1) << 16) >> 3);
            du2 = (((u4 - u3) << 16) >> 3);
            dv1 = (((v2 - v1) << 16) >> 3);
            dv2 = (((v4 - v3) << 16) >> 3);

            zdu1 = (((d2 - d1) << 16) >> 3);
            zdu2 = (((d4 - d3) << 16) >> 3);

            ymul = (j << 3) * 120 + (i << 3);
            for (y = 0; y < 8; y+=2)
            {
                tempu1 = tempu2 = tempv1 = tempv2 = 0;
                tempz1 = tempz2 = 0;
                {                      
                    #define PLOT(add) \
                    { \  
                      tempu1 += du1; tempu2 += du2; s1 = u1 + (tempu1 >> 16); s2 = u3 + (tempu2 >> 16); \
                      tempv1 += dv1; tempv2 += dv2; t1 = v1 + (tempv1 >> 16); t2 = v3 + (tempv2 >> 16); \
                      tempz1 += zdu1; tempz2 += zdu2; z1 = d1 + (tempz1 >> 16); z2 = d3 + (tempz2 >> 16);\
                      vz1 = (((z2 - z1) << 16) >> 3); \
                      eu1 = (((s2 - s1) << 16) >> 3); eu2 = (((t2 - t1) << 16) >> 3); \
                      u = (s1 + ((eu1 * y) >> 16)); v = (t1 + ((eu2 * y) >> 16)); z = z1 + ((vz1 * y) >> 16) >> 5; \  
                      u += scroll_u; v += scroll_v; \    
                      offset = ymul + add; \   
                      texofs = ((v << 7) + u) & 0x3FFF; color = tex[texofs] - 224 + (z << 5); target32[offset] = (color << 8) | color; \       
                      target32[offset + 120] = (color << 8) | color; \          
                    }                                                               
                   
                    PLOT(0)
                    PLOT(1)
                    PLOT(2)
                    PLOT(3)
                    PLOT(4)
                    PLOT(5)
                    PLOT(6)
                    PLOT(7)
                }
                ymul += 240;
            } 
        }
        tablepos += tab;
    }   

    // Draw border
    u16 c = (26 << 8) | 26;
    HLine(16, 237-15, 8, c);
    HLine(16, 237-15, 159-8, c);    
}
