/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

/*
	PSP VSH 24bpp text bliter
*/
#include "common.h"

//#define ALPHA_BLEND 1

extern unsigned char msx[];

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int pwidth;
int pheight, bufferwidth, pixelformat;
unsigned int* vram32;

u32 fcolor = 0x00ffffff;
u32 bcolor = 0xff000000;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static u32 adjust_alpha(u32 col)
{
	u32 alpha = col>>24;
	u8 mul;
	u32 c1,c2;

	if(alpha==0)    return col;
	if(alpha==0xff) return col;

	c1 = col & 0x00ff00ff;
	c2 = col & 0x0000ff00;
	mul = (u8)(255-alpha);
	c1 = ((c1*mul)>>8)&0x00ff00ff;
	c2 = ((c2*mul)>>8)&0x0000ff00;
	return (alpha<<24)|c1|c2;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//int blit_setup(int sx,int sy,const char *msg,int fg_col,int bg_col)
int blit_setup(void)
{
	int unk;
	sceDisplayGetMode(&unk, &pwidth, &pheight);
	sceDisplayGetFrameBuf((void*)&vram32, &bufferwidth, &pixelformat, PSP_DISPLAY_SETBUF_NEXTFRAME);
	if( (bufferwidth==0) || (pixelformat!=3)) return -1;

	fcolor = 0x00ffffff;
	bcolor = 0xff000000;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// blit text
/////////////////////////////////////////////////////////////////////////////
void blit_set_color(int fg_col,int bg_col)
{
	fcolor = fg_col;
	bcolor = bg_col;
}

/////////////////////////////////////////////////////////////////////////////
// blit text
/////////////////////////////////////////////////////////////////////////////
int blit_string(int sx,int sy,const char *msg)
{
	int x,y,p;
	int offset;
	char code;
	u8 font;
	u32 fg_col,bg_col;

	u32 col,c1,c2;
	u32 alpha;

	fg_col = adjust_alpha(fcolor);
	bg_col = adjust_alpha(bcolor);


//Kprintf("MODE %d WIDTH %d\n",pixelformat,bufferwidth);
	if( (bufferwidth==0) || (pixelformat!=3)) return -1;

	for(x=0;msg[x] && x<(pwidth/8);x++)
	{
		code = msg[x] & 0x7f; // 7bit ANK
		for(y=0;y<8;y++)
		{
			offset = (sy+y)*bufferwidth + sx+x*8;
			font = y>=7 ? 0x00 : msx[ code*8 + y ];
			for(p=0;p<8;p++)
			{
				col = (font & 0x80) ? fg_col : bg_col;
				alpha = col>>24;
				if(alpha==0) vram32[offset] = col;
				else if(alpha!=0xff)
				{
					c2 = vram32[offset];
					c1 = c2 & 0x00ff00ff;
					c2 = c2 & 0x0000ff00;
					c1 = ((c1*alpha)>>8)&0x00ff00ff;
					c2 = ((c2*alpha)>>8)&0x0000ff00;
					vram32[offset] = (col&0xffffff) + c1 + c2;
				}

				font <<= 1;
				offset++;
			}
		}
	}
	return x;
}

int blit_string_ctr(int sy,const char *msg)
{
	int sx = 480/2;

#ifdef CONFIG_639
	if(psp_fw_version == FW_639)
		sx = 480/2-scePaf_strlen(msg)*(8/2);
#endif

#ifdef CONFIG_635
	if(psp_fw_version == FW_635)
		sx = 480/2-scePaf_strlen(msg)*(8/2);
#endif

#ifdef CONFIG_620
	if(psp_fw_version == FW_620)
		sx = 480/2-scePaf_strlen_620(msg)*(8/2);
#endif

	return blit_string(sx,sy,msg);
}
