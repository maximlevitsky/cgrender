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
#ifndef PNGLOADER_H
#define PNGLOADER_H

#include <stdio.h>
#include "png.h"//should reside where you can see it


/*
	This class was derived from libpng example.c file
	it is intended to supply simple read and write 
	routines into the png library.

	It only support a small subset of png options 
	which means that it will be able to read most of the 
	png files available but will only write the files 
	in 24bpp format with default compression and no interlace 

	please send bugs to schein@cs.technion.ac.il
*/


#define SET_RGB(r,g,b) ((r)<<24|(g)<<16|(b)<<8|0)
#define SET_RGBA(r,g,b,a) ((r)<<24|(g)<<16|(b)<<8|a)
#define GET_R(x) (((x)&0xff000000)>>24)
#define GET_G(x) (((x)&0x00ff0000)>>16)
#define GET_B(x) (((x)&0x0000ff00)>>8)
#define GET_A(x) ((x)&0x000000ff)


class PngLoader
{
public:
	PngLoader(const char * name,int width =0,int height =0);
	PngLoader();
	~PngLoader();
	void SetFileName(const char * name);
	void SetWidth(int width);
	void SetHeight(int height);
	int GetWidth();
	int GetHeight();
	void SetValue(unsigned int x,unsigned int y,unsigned int value);
	int GetValue(unsigned int x,unsigned int y) const;
	int GetNumChannels();
	int GetBPP();
	bool ReadPng();
	bool InitWritePng();
	bool WritePng();

	void ClosePng();
private:
	bool IsPngFile();

private:
	int m_width;
	int m_height;
	char * m_fileName;
	FILE *m_fp;
	png_structp m_png_ptr;
	png_infop m_info_ptr;
};

#endif
