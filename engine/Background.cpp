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

#include "renderer/Renderer.h"
#include "Shaders.h"
#include "Engine.h"
#include <assert.h>

bool Engine::loadBackgroundImage( const char* file )
{

	resetBackground();
	_backgroundTexture = Texture::loadCached(file, true);
	return true;
}

void Engine::resetBackground()
{
	if (_backgroundTexture) {
		Texture::unloadCached(_backgroundTexture);
		_backgroundTexture = NULL;
	}
	_backGroundColor = Color(100,100,100);
}

void Engine::setTileBackground( bool enable )
{
	_tile_background = enable;
}

void Engine::renderBackground()
{
	double scaleX, scaleY;

	if (_backgroundTexture) 
	{
		if (_tile_background) {
			scaleX = _outputSizeX, scaleY = _outputSizeY;
		} else  {
			scaleX = _backgroundTexture->getWidth(), scaleY = _backgroundTexture->getHeight();
		}

		_renderer->fillBackgroundTexture(*_backgroundTexture, scaleX, scaleY);
		return;
	}
	
	_renderer->fillBackground(_backGroundColor / 255);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////

FogParams Engine::getFogParams() 
{ 
	return _fogParams; 
}

void Engine::setFogParams(const FogParams &params) 
{
	_fogParams = params;
}

void Engine::resetFog() {
	_fogParams.reset();
	_fogParams.color = _backGroundColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::setupFogShaderData() 
{
	ShaderFogData &fp = _shaderData.fogParams;

	fp.enabled = _fogParams.enabled;

	if (fp.enabled) {

		fp.color = _fogParams.color / 255;
		fp.linear = _fogParams.type == FOG_LINEAR;
		fp.exp2 = _fogParams.type == FOG_EXPONETIAL2;

		fp.start = _fogParams.startPoint;
		fp.end = _fogParams.endPoint;
		fp.scale = 1.0 / (_fogParams.endPoint - _fogParams.startPoint);

		fp.density = _fogParams.density;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

Color applyFog(const UniformBuffer* u, double depth, const Color &color)
{
	double coof;
	const ShaderFogData &fp = u->fogParams;

	depth = max(0.0, depth);

	if (fp.linear)
		coof = (fp.end -depth) *  fp.scale;
	else {

		double expparam = fp.density * depth;
		if (fp.exp2)
			expparam = expparam * expparam;

		coof = exp(-expparam);
	}

	coof = clamp(coof, 0.0, 1.0);
	return (color * coof) + (fp.color * (1.0-coof));
}
