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
#include <assert.h>
#include <limits>
#include <cmath>

#include "Renderer.h"
#include "Texture.h"
#include "Samplers.h"

#include "common/Mat4.h"
#include "common/Vector4.h"
#include "common/Math.h"


const static double poissonDisk[4][2] = 
{
	{ -0.94201624, -0.39906216 },
	{ 0.94558609, -0.76890725 },
	{ -0.094184101, -0.92938870 },
	{ 0.34495938, 0.29387760 }
};


void TextureSampler::bindTexture( const Texture* texture )
{
	unbindTexture();

	if (!texture)
		return;

	_texture = texture;
	_mipmapCount = texture->getMipmapCount();
	_scaleX = texture->getWidth();
	_scaleY = texture->getHeight();
}

void TextureSampler::unbindTexture()
{
	_texture = NULL;
	_mipmapCount = 0;
	_scaleX = _scaleY = 0;
}

void TextureSampler::setScale( double new_scalex, double new_scaley )
{
	_scaleX = new_scalex;
	_scaleY = new_scaley;
}

Color TextureSampler::sample( double x, double y ) const
{
	x = std::abs(x) * _scaleX;
	y = std::abs(1-y) * _scaleY;

	int tx = (int)x %  _texture->getWidth();
	int ty = (int)y %  _texture->getHeight();
	return _texture->sample(tx,ty);
}

Color TextureSampler::sampleBiLinear( double x, double y, int mipNumber /*= 0*/ ) const
{
	const Texture &t = _texture[mipNumber];

	// scale the the input coordinates by current scale.
	x = std::abs(x) * _scaleX / (1 << mipNumber);
	y = std::abs(1-y) * _scaleY / (1 << mipNumber);

	// find texel coordinates
	int tx = (int)x % t.getWidth();
	int ty = (int)y %  t.getHeight();

	// find weights of the four pixels
	double wx = frac(x), wy = frac(y);

	Color c1 = t.sample(tx, ty)   * (1.0 - wx) + t.sample(tx+1, ty) * wx;
	Color c2 = t.sample(tx, ty+1) * (1.0 - wx) + t.sample(tx+1, ty+1) * wx;
	return (c1 * (1.0-wy)) + (c2 * (wy));
}

Color TextureSampler::sampleBiLinearMipmapped( double x, double y, double dx, double dy ) const
{
	dx *= _texture->getWidth();
	dy *= _texture->getHeight();

	double lod = log2(dx*dx+dy*dy) / 2;
	int level = clamp((int)lod, 0, _mipmapCount -1);
	return sampleBiLinear(x, y, level);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

ShadowSampler::ShadowSampler() : _depthbuffer(NULL) {}

void ShadowSampler::bindTexture( const DepthTexture* texture )
{
	_depthbuffer = texture;

	if (_depthbuffer) {
		_width = texture->getWidth();
		_height = texture->getHeight();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////

double ShadowSampler::sample( double x, double y, double z ) const
{
	// regular simple sample
	int x_int = clamp((int)(x * _width +0.5), 0, _width - 1);
	int y_int = clamp((int)(y *_height +0.5), 0, _height - 1);
	return _depthbuffer->getPixelValue(x_int,y_int) >= z ? 1.0 : 0;
}


double ShadowSampler::samplePCF( double x, double y, double z, int kernelSize ) const
{
	// sample with PCF filtering
	int x_int = (int)(x * _width +0.5);
	int y_int = (int)(y * _height +0.5);

	int x1 = clamp(x_int - kernelSize / 2, 0, _width - kernelSize);
	int y1 = clamp(y_int - kernelSize / 2, 0, _height - kernelSize);

	double result = 0;

	for (int x = x1 ; x < x1 + kernelSize ; x++) 
		for (int y = y1 ; y < y1 + kernelSize ; y++)
			result  += ((_depthbuffer->getPixelValue(x,y) >= z) ? 1.0 : 0);

	return result / (kernelSize*kernelSize);
}

double ShadowSampler::samplePoison( double x, double y, double z ) const 
{
	double result = 0;
	for (int i = 0 ; i < 4 ; i++)
		result += sample(x + poissonDisk[i][0] / 700.0, y + poissonDisk[i][1] / 700.0, z);
	return result / 4;
}

double ShadowSampler::samplePoisonPCF( double x, double y, double z, int taps ) const
{
	double result = 0;
	for (int i = 0 ; i < 4 ; i++)
		result += samplePCF(x + poissonDisk[i][0] / 700.0, y + poissonDisk[i][1] / 700.0, z, taps);
	return result / 4;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void ShadowCubemapSampler::bindTextures( const DepthTexture** textures )
{
	_isBound = (textures != NULL);

	if (_isBound) {
		for (int i=0 ; i < 6 ;i++)
			_faceSamplers[i].bindTexture(textures[i]);
	} else {
		for (int i=0 ; i < 6 ;i++)
			_faceSamplers[i].bindTexture(NULL);
	}
}

int ShadowCubemapSampler::selectFace(const Vector3 &dir) const
{
	const double x = -dir.x();
	const double y = -dir.y();
	const double z = -dir.z();

	if (std::abs(z) >= std::abs(x) && std::abs(z) >= std::abs(y))
		return  z > 0 ? 4 : 5;
	else if (std::abs(y) >= std::abs(x))
		return y > 0 ? 2 : 3;
	else
		return x > 0 ? 0 : 1;
}

double ShadowCubemapSampler::sample( int face, double x, double y, double z ) const
{
	return _faceSamplers[face].sample(x,y,z);
}

double ShadowCubemapSampler::samplePCF( int face, double x, double y, double z, int taps ) const
{
	return _faceSamplers[face].samplePCF(x,y,z,taps);
}

double ShadowCubemapSampler::samplePoison( int face, double x, double y, double z ) const
{
	return _faceSamplers[face].samplePoison(x,y,z);
}

double ShadowCubemapSampler::samplePoisonPCF( int face, double x, double y, double z, int taps ) const
{
	return _faceSamplers[face].samplePoisonPCF(x,y,z,taps);
}
