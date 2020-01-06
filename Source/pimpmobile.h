/*

pimpmobile.h -- The interface of Pimpmobile, a module playback library
targeting the Nintendo GameBoy Advance


Copyright (c) 2005 J�rn Nystad and Erik Faye-Lund

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use
of this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim
   that you wrote the original software. If you use this software in a product,
   an acknowledgment in the product documentation would be appreciated but is
   not required.

2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/

#ifndef PIMPMOBILE_H
#define PIMPMOBILE_H

#ifdef __cplusplus
extern "C"
{
#endif

void CpuFastSet( const void *src, void *dst, u32 mode)
{
	asm (
		"mov r0, %[src]  \n"
		"mov r1, %[dst]  \n"
		"mov r2, %[mode] \n"
#ifdef __thumb__
		"swi 0xC         \n"
#else
		"swi 0xC << 16   \n"
#endif
		: /* no output */
		: /* inputs */
			[src]  "r"(src),
			[dst]  "r"(dst),
			[mode] "r"(mode)
		: "r0", "r1", "r2", "r3" /* clobbers */
	);
}

void pimp_init(const void *module, const void *sample_bank);
void pimp_close();

void pimp_vblank(); /* call this on the beginning of each vsync */
void pimp_frame(); /* call once each frame. doesn't need to be called in precious vblank time */

/* get information about playback */
int pimp_get_row();
int pimp_get_order();
void pimp_set_pos(int row, int order);

enum pimp_callback_type
{
	PIMP_CALLBACK_SYNC = 0,
	PIMP_CALLBACK_NOTE = 1,
	PIMP_CALLBACK_UNSUPPORTED_EFFECT = 2,
	PIMP_CALLBACK_UNSUPPORTED_VOLUME_EFFECT = 3
};

/* callback system (for music sync) */
typedef void (*pimp_callback)(int, int);
void pimp_set_callback(pimp_callback callback);

#ifdef __cplusplus
}
#endif

#endif /* PIMPMOBILE_H */
