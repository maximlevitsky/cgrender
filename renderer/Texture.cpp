/*
	This file is part of CG4.

	Copyright (c) Inbar Donag and Maxim Levitsky

    CG4 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    CG4 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CG4.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Texture.h"
#include "common/Vector4.h"
#include <cmath>
#include "model/PngLoader.h"

static std::map<std::string, const Texture*> textureCache;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

Texture::Texture(DEVICE_PIXEL *data, int width, int height) : TextureBase(data,width,height)
{
	refcount = 1;
	_mipmapCount = 0;
}

Texture::Texture(int width, int height) : TextureBase(width, height)
{
	refcount = 1; _mipmapCount = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

Color Texture::sample( int x, int y ) const
{
	x = min(_width-1, x);
	y = min(_height-1,y);
	const DEVICE_PIXEL & p = getPixelValue(x,y);
	return Color((double)p.Red/255, (double)p.Green/255, (double)p.Blue/255);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

const Texture* Texture::loadCached( const char* name, bool mipmaps )
{
	if (strlen(name) == 0)
		return NULL;

	auto iter = textureCache.find(name);
	if (iter != textureCache.end()) 
	{
		iter->second->refcount++;
		return iter->second;
	}

	Texture* result = load(name, mipmaps);

	if (!result)
		return NULL;

	textureCache[name] = result;
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Texture::unloadCached(const Texture*t)
{
	Texture* tex = const_cast<Texture*>(t);
	if (tex->refcount > 1)
		tex->refcount--;
	else {
		textureCache.erase(tex->filename);
		delete [] tex;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

Texture* Texture::load(const char* name, bool mipmaps)
{
	Texture * t = NULL;

	PngLoader p;
	p.SetFileName(name);

	if (!p.ReadPng())
		return NULL;

	int mip_count = mipmaps ? (int)log2(min(p.GetWidth(), p.GetHeight())) : 1;
	t = new Texture[mip_count];
	t->_mipmapCount = mip_count;
	t->allocate(p.GetWidth(), p.GetHeight());

	for (int y = 0 ; y < t->_height ; y++)
		for (int x = 0 ; x < t->_width ; x++) 
		{
			int value = p.GetValue(x,y);
			t->setPixelValue(x,y, DEVICE_PIXEL((uint8_t)GET_R(value), (uint8_t)GET_G(value), (uint8_t)GET_B(value)));
		}

	p.ClosePng();

	t->filename = name;

	Texture *prev_level = t;
	for (int level = 1 ; level < t->_mipmapCount ; level++) 
	{
		Texture *next_level = prev_level+1;
		next_level->allocate(prev_level->getWidth() / 2, prev_level->getHeight() /2);

		for (int x = 0 ; x < next_level->_width ; x++)
			for (int y = 0 ; y < next_level->_height ; y++) 
			{
				Color c = prev_level->sample(x*2, y*2);
				c += prev_level->sample(x*2+1, y*2);
				c +=  prev_level->sample(x*2, y*2+1);
				c +=  prev_level->sample(x*2+1, y*2+1);
				c /= 4;
				next_level->setPixelValue(x,y, DEVICE_PIXEL(
						(unsigned char)(c.x()*255),
						(unsigned char)(c.y()*255),
						(unsigned char)(c.z()*255)));
			}

			prev_level = next_level;
	}

	
	return t;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

Texture::~Texture()
{
	refcount--;
	assert(refcount == 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
bool TextureBase<T>::saveToFile(const char* file) const
{
	PngLoader o(file, _width, _height);
	if (!o.InitWritePng())
		return false;

	for (int y = 0; y < _height; y++) {
		for (int x = 0; x < _width; x++) {
			Color pixel = debugGetPixel(x, y);
			o.SetValue(x, _height - y,
					SET_RGB((int) ((pixel.x() * 255)),
							(int) ((pixel.y() * 255)),
							(int) ((pixel.z() * 255))));
		}
	}
	bool retval = o.WritePng();
	o.ClosePng();
	return retval;
}

template<>
bool TextureBase<DEVICE_PIXEL>::saveToFile(const char* file) const;

template<>
bool TextureBase<double>::saveToFile(const char* file) const;

template<>
bool TextureBase<int>::saveToFile(const char* file) const;
