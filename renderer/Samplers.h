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
#ifndef SAMPLERS_H
#define SAMPLERS_H

#include "common/Mat4.h"
#include "common/Vector4.h"

class Texture;

class TextureSampler
{
public:
	TextureSampler() : _texture(NULL), _scaleX(0), _scaleY(0), _mipmapCount(0) {}

	void bindTexture(const Texture* texture);
	void unbindTexture();
	bool isBound() const { return _texture != NULL; }

	void setScale(double new_scalex, double new_scaley);

	/* main sampling functions */
	Color sample(double x, double y) const;
	Color sampleBiLinear(double x, double y, int mipNumber = 0) const;
	Color sampleBiLinearMipmapped(double x, double y, double x_step, double y_step) const;

private:
	const Texture* _texture;
	int _mipmapCount;
	double _scaleX;
	double _scaleY;
};

class ShadowSampler
{
public:
	ShadowSampler();
	void bindTexture(const DepthTexture* texture);

	double sample(double x, double y, double z) const;
	double samplePCF(double x, double y, double z, int taps) const;
	double samplePoison(double x, double y, double z) const;
	double samplePoisonPCF(double x, double y, double z, int taps) const;

	bool isBound() const { return _depthbuffer != NULL; }
public:
	const DepthTexture* getBuffer()  { return _depthbuffer;}
	void clear();
private:
	const DepthTexture *_depthbuffer;
	int _width;
	int _height;
};

class ShadowCubemapSampler
{
public:
	ShadowCubemapSampler() : _isBound(false) {}
	void bindTextures(const DepthTexture** textures);

	double sample(int face, double x, double y, double z) const;
	double samplePCF(int face, double x, double y, double z, int taps) const;
	double samplePoison(int face, double x, double y, double z) const;
	double samplePoisonPCF(int face, double x, double y, double z, int taps) const;

	bool isBound() const { return _isBound; }
	int selectFace(const Vector3 &dir) const;
private:
	ShadowSampler _faceSamplers[6];
	bool _isBound;
};

#endif
