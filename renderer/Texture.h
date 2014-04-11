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
#ifndef TEXTURE_H
#define TEXTURE_H

#include "common/Vector4.h"
#include "Renderer.h"
#include <string>
#include <map>

//////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class TextureBase 
{
public:
	TextureBase(int width, int height) 
	{
		_allocated = false;
		allocate(width, height);
	}

	TextureBase(T* data, int width, int height) 
	{
		_allocated = false;
		_width = width;
		_height = height;
		_data = data;

	}

	TextureBase() 
	{
		_allocated = false;
		_width =  0;
		_height = 0;
		_data = NULL;
	}

	void allocate(int width, int height) 
	{
		 if (_allocated) 
		 {
			 if (_width == width && _height == height)
				 return;
			 delete [] _data;
		 }

		 _data = new T[width*height];
		 _width = width;
		 _height = height;
		 _allocated = true;
	}

	virtual ~TextureBase() 
	{
		if (_allocated)
			delete [] _data;

	}

	void clear(T value)
	{
		for (int y = 0 ; y < _height ; y++)
			for (int x = 0 ; x < _width ; x++)
				setPixelValue(x,y, value);
	}

	void setPixelValue(int x, int y, T value) const
	{
		assert(x < _width && y < _height);
		_data[y*_width+x] = value;
	}

	T getPixelValue(int x, int y)  const
	{
		assert(x < _width && y < _height);
		return _data[y*_width+x];
	}

	int getWidth() const { return _width; }
	int getHeight() const { return _height; }

	virtual Color debugGetPixel(int x, int y) const  { return Color(0,0,0);  }

	bool saveToFile(const char* file) const;

	T* getPointer() { return _data; }

private:
	TextureBase(const TextureBase &other);
	TextureBase& operator=(const TextureBase &other);
protected:
	int _width;
	int _height;
	T *_data;
	bool _allocated;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////

class Texture : public TextureBase<DEVICE_PIXEL>
{
public:
	/* allocate new writable texture wrapping existing data */
	Texture(DEVICE_PIXEL *data, int width, int height);

	/* allocate a new writable texture*/
	Texture(int width, int height);


	// load a texture
	static Texture* load(const char* name, bool mipmaps = false);

	/* Static texture loading functions*/
	static const Texture* loadCached(const char* name, bool mipmaps);
	static void unloadCached(const Texture*t);

	/* texture information */
	const char* getFilename()  const { return filename.c_str(); }

	/* main sampling functions */
	Color sample(int x, int y) const;
	~Texture();

	virtual Color debugGetPixel(int x, int y) const  { return sample(x,y);  }

	int getMipmapCount() const { assert(_mipmapCount) ; return _mipmapCount;}
private:
	/* properties */
	std::string filename;
	int _mipmapCount;

	/* to be a good citizen */
	Texture(const Texture &other);
	Texture& operator=(const Texture &other);

	static Texture* load_failback(const char* name, bool mipmaps);

	/* RO texture cache */
	mutable int refcount;

	Texture() { _mipmapCount = 0 ; _allocated = false ; refcount = 1;}
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////

class DepthTexture : public TextureBase<double> 
{
public:
	DepthTexture(int width, int height) : TextureBase(width, height) {}

	void clear() 
	{ 
		TextureBase::clear(std::numeric_limits<float>::infinity());
	}

	bool zTest(int x, int y, double d)
	{
		if (d >= getPixelValue(x,y))
			return false;
		setPixelValue(x,y,d);
		return true;
	}

	Color debugGetPixel(int x, int y) const override
	{

		if (getPixelValue(x,y) == std::numeric_limits<double>::infinity())
			return Color(0,0,1);

		double depth = 1.0-getPixelValue(x,y);

		if (depth > 1)
			return Color(1, 0,0);
		else if (depth < 0)
			return Color(0, 1,0);
		else
			return Color(depth, depth, depth);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

class IntegerTexture : public TextureBase<unsigned int> 
{
public:
	IntegerTexture(int width, int height) : TextureBase(width, height) {}
	void clear() { TextureBase::clear(0);}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
