// Button Bashers Secret Santa 2019 Gameboy Advance demo source code
// 
// File: Main.c
// Description: the entry point of the demo, handles initialization of Pimp Mobile module player, sets up 
//              interrupts and features some interrupt-based post-processing effects and a very basic script 
//              system for running the effects.
// 
// Note: because of time pressure there are a couple of hacks in the main-loop of the script system to start / end
//       fades, and I would rather add interrupt support for effects next time so that I can move the code for Plasma
//       to the appropriate effect source file (Intro.c)

#pragma long_calls

#include "KDEngine.h"
#include "MovetableEffect.h"
#include "Freedir.h"
#include "Splash.h"
#include "Intro.h" 
#include "Zoomblur.h"
#include "OBJ_test.h"

#include <gba_base.h>
#include <gba_systemcalls.h>
#include <gba_dma.h>
#include <gba_sound.h>
#include <gba_timers.h>
#include <gba_interrupt.h>

#include "pimpmobile.h"

extern const unsigned char xmas_mod[];
extern const unsigned char xmas_sample_bank[];

static uint g_frame = 0;
static uint g_currentEffect = 0;
u8 curBlend = 0;
u8 fade =  16;
int scanlineCounter = 0;
int scanline_y;

const static void (*g_initializers[])() = 
{
  Splash_Init,
  Intro_Init,
  Freedir_Init,
  OBJ_Test_Init, 
  MovetableEffect_Init,     
  ZoomBlur_Init,      
};

const static void (*g_effects[])(uint) = 
{
  Splash_Update,
  Intro_Update,
  Freedir_Update,  
  OBJ_Test_Update, 
  MovetableEffect_Update,      
  ZoomBlur_Update,    
};

const static void (*g_destroyers[])() = 
{
  Splash_Destroy,
  Intro_Destroy,  
  Freedir_Destroy,    
  OBJ_Test_Destroy,
  MovetableEffect_Destroy,      
  ZoomBlur_Destroy,    
};

const static int transitions[] = 
{
  0, 225*2, 225*4, 225*8 + 255, 225*10 + 255, 225*12 + 255
};

void vblank()
{
	pimp_vblank();  
	pimp_frame();
  g_frame++;
}

void Plasma()
{
  if (g_frame >= transitions[1] && g_frame < transitions[2])
  {
    u16 scanline = REG_VCOUNT << 4;
    REG_BG2PA = 255+(SinLUT[(64 + ((scanline >> 3) + (scanlineCounter >> 6))) & 255] >> 4);
    REG_BG2PD = 255-(SinLUT[((-scanlineCounter >> 9) + ((160 * 2 - scanline) >> 4)) & 255]);
  }
  else if (g_frame >= transitions[2])
  {
    REG_BG2PA = 255;
    REG_BG2PD = 255;
  }
}

void Fade(int t)
{
  //t = clamp(t, 0, 16);

  REG_BLDCNT = 0x00ff | ((0x0010)&63);
  REG_BLDY = t;
}

void vcount()
{
  Plasma();

  scanlineCounter++;
}

int main()
{
  pimp_init(xmas_mod, xmas_sample_bank);
	irqInit();
	irqEnable(IRQ_VBLANK | IRQ_HBLANK);
	irqSet(IRQ_VBLANK, vblank);
  irqSet(IRQ_HBLANK, vcount);

  REG_DISPCNT = 0x1444;
  REG_BLDCNT = 0x009f; // start wit
  //REG_BLDCNT = 0x00ff; // start zwart
  REG_BLDY = 16;  

  int i = 0;
  while (1)
  {
    g_currentEffect = i;

    g_initializers[i]();
    while (g_frame < transitions[i+1]) 
    {    
      VBlankIntrWait(); 

      if (g_frame >= transitions[1]-32) // intro starts white
      {
        if (g_frame <= transitions[i] + 32)
          Fade(16 - ((g_frame - transitions[i] ) >> 1));

        if (g_frame >= transitions[i+1]-32)
          Fade(16 - ((transitions[i+1] - g_frame) >> 1));
      }

      // logo fade
      if (g_frame >= ((transitions[1]>>1)-64) && g_frame < transitions[1]-64)
      {
        REG_BLDCNT = 0x009f; // fade to white

        if (fade > 0)
        {
          fade -= (curBlend >> 6);
          curBlend++;
        } 
        REG_BLDY = fade;   
      }      

      g_effects[i](g_frame);

      if (REG_DISPCNT & 0x10)
      {
        REG_DISPCNT &= ~0x10;
        g_currentBuffer = BACKBUFFER;
      }
      else
      {
        REG_DISPCNT |= 0x10;
        g_currentBuffer = FRONTBUFFER;
      }  
    }
    g_destroyers[i]();
    
    REG_BLDCNT = 0x00ff | ((0x0010)&63);
    REG_BLDY = 16;   

    const u8 palEntry = 139;
    const u16 bgColor = (palEntry << 8) | palEntry; 
    u16 *target = (g_currentBuffer == BACKBUFFER) ? FRONTBUFFER : BACKBUFFER;
    FastSet16(target, bgColor, (SCREENWIDTH*SCREENHEIGHT)>>1);  

    i++;
  }
}