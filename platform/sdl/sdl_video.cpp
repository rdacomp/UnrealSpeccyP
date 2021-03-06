/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2016 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../platform.h"

#ifdef USE_SDL
#ifndef SDL_UNUSE_VIDEO

#include <SDL.h>
#include "../../ui/ui.h"

namespace xPlatform
{

static SDL_Surface* screen = NULL;
static SDL_Surface* offscreen = NULL;

static struct eCachedColors
{
	inline dword RGBX(byte r, byte g, byte b) const { return (b << 16)|(g << 8)|r; }
	void Init(SDL_PixelFormat* pf)
	{
		const byte brightness = 200;
		const byte bright_intensity = 55;
		for(int c = 0; c < 16; ++c)
		{
			byte i = c&8 ? brightness + bright_intensity : brightness;
			byte b = c&1 ? i : 0;
			byte r = c&2 ? i : 0;
			byte g = c&4 ? i : 0;
			items[c] = SDL_MapRGB(pf, r, g, b);
			items_rgbx[c] = RGBX(r, g, b);
		}
	}
	dword items[16];
	dword items_rgbx[16];
}
color_cache;

bool InitVideo()
{
#ifdef SDL_NO_OFFSCREEN
	screen = SDL_SetVideoMode(320, 240, 32, SDL_SWSURFACE);
	if(!screen)
		return false;
#else//SDL_NO_OFFSCREEN
	screen = SDL_SetVideoMode(320, 240, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
	if(!screen)
		return false;
	offscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 32,
		screen->format->Rmask,
		screen->format->Gmask,
		screen->format->Bmask,
		screen->format->Amask);
	if(!offscreen)
		return false;
#endif//SDL_NO_OFFSCREEN
	color_cache.Init(screen->format);
	return true;
}
void DoneVideo()
{
	if(offscreen)
		SDL_FreeSurface(offscreen);
	if(screen)
		SDL_FreeSurface(screen);
}

void UpdateScreen()
{
#ifdef SDL_NO_OFFSCREEN
	SDL_Surface* out = screen;
#else//SDL_NO_OFFSCREEN
	SDL_Surface* out = offscreen;
#endif//SDL_NO_OFFSCREEN
	if(SDL_MUSTLOCK(out))
		SDL_LockSurface(out);
	byte* data = (byte*)Handler()->VideoData();
	dword* scr = (dword*)out->pixels;
#ifdef USE_UI
	byte* data_ui = (byte*)Handler()->VideoDataUI();
	if(data_ui)
	{
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320; ++x)
			{
				xUi::eRGBAColor c_ui = xUi::palette[*data_ui++];
				xUi::eRGBAColor c = color_cache.items_rgbx[*data++];
				*scr++ = SDL_MapRGB(out->format, (c.r >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.b >> c_ui.a) + c_ui.b);
			}
			scr += out->pitch - 320*sizeof(*scr);
		}
	}
	else
#endif//USE_UI
	{
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320/4; ++x)
			{
				*scr++ = color_cache.items[*data++];
				*scr++ = color_cache.items[*data++];
				*scr++ = color_cache.items[*data++];
				*scr++ = color_cache.items[*data++];
			}
			scr += out->pitch - 320*sizeof(*scr);
		}
	}
	if(SDL_MUSTLOCK(out))
		SDL_UnlockSurface(out);
#ifndef SDL_NO_OFFSCREEN
	SDL_BlitSurface(offscreen, NULL, screen, NULL);
#endif//SDL_NO_OFFSCREEN
	SDL_Flip(screen);
}

}
//namespace xPlatform

#endif//SDL_UNUSE_VIDEO
#endif//USE_SDL
