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

void Engine::resetBackground()
{
	if (_backgroundTexture)
	{
		Texture::unloadCached(_backgroundTexture);
		_backgroundTexture = NULL;
	}

	_backgroundSettings.color = Color(0.4,0.4,0.4);
	_backgroundSettings.mode = BackgroundParams::COLOR;
	_backgroundSettings.textureScalingMode = BackgroundParams::STRETCH;
	_backgroundSettings.textureFile = "";
}


void Engine::setBackGroundSettings(const BackgroundParams newSettings)
{
	bool textureUpdate =
			newSettings.mode != _backgroundSettings.mode ||
			newSettings.textureFile != _backgroundSettings.textureFile;


	if (textureUpdate) {

		if (_backgroundTexture)
		{
			Texture::unloadCached(_backgroundTexture);
			_backgroundTexture = NULL;
		}

		if (newSettings.mode == BackgroundParams::TEXTURE) {
			_backgroundTexture = Texture::loadCached(newSettings.textureFile.c_str(), false);
		}
	}

	_backgroundSettings = newSettings;
}

void Engine::renderBackground()
{
	double scaleX, scaleY;

	if (_backgroundSettings.mode ==  BackgroundParams::COLOR || !_backgroundTexture)
	{
		_renderer->renderBackgroundColor(_backgroundSettings.color);
		return;
	}

	switch(_backgroundSettings.textureScalingMode)
	{
	case BackgroundParams::TILE:
		scaleX = _outputSizeX, scaleY = _outputSizeY;
		_renderer->renderBackground(*_backgroundTexture, scaleX, scaleY);
		break;
	case BackgroundParams::STRETCH:
		scaleX = _backgroundTexture->getWidth(), scaleY = _backgroundTexture->getHeight();
		_renderer->renderBackground(*_backgroundTexture, scaleX, scaleY);
		break;
	default:
		assert(0);
	}
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
	_fogParams.color = _backgroundSettings.color;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::setupFogShaderData() 
{
	ShaderFogData &fp = _shaderData.fogParams;

	fp.enabled = _fogParams.enabled;

	if (fp.enabled) {

		fp.color = _fogParams.color;
		fp.linear = _fogParams.type == FogParams::FOG_LINEAR;
		fp.exp2 = _fogParams.type == FogParams::FOG_EXPONETIAL2;

		fp.start = _fogParams.startPoint;
		fp.end = _fogParams.endPoint;
		fp.scale = 1.0 / (_fogParams.endPoint - _fogParams.startPoint);

		fp.density = _fogParams.density;
	}
}

