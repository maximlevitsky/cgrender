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
#include "Shaders.h"
#include "Engine.h"
#include <assert.h>
#include "model/Material.h"

void Engine::resetMaterials()
{
	for (unsigned int i = 0 ; i < _itemCount ; i++) 
		_sceneItems[i]._material.reset();
	_globalObjectMaterial.reset();
}

void Engine::reloadTextures()
{
	for (unsigned int i = 0 ; i < _itemCount ; i++) 
	{
		SceneItem &item = _sceneItems[i];
		std::string file = item._material.getObjectTexture();

		if (!item.texture || file != item.texture->getFilename()) 
		{
			if (item.texture) Texture::unloadCached(item.texture);
			item.texture = NULL;
			item.texture = Texture::loadCached(file.c_str(), true);
			item.texScaleX = item._material.getscaleX();
			item.texScaleY = item._material.getscaleY();
		}
	}
}

MaterialParams& Engine::getMatrialParams()
{
	if (_drawSeparateObjects && _selectedObject != -1)
		return _sceneItems[_selectedObject]._material;
	return _globalObjectMaterial;
}



void Engine::setupMaterialsShaderData( int objectID )
{
	SceneItem &currentItem = _sceneItems[objectID];

	// setup which material to use
	MaterialParams *material = &currentItem._material;

	/* setup misc settings*/
	_shaderData.textureSampler.bindTexture(currentItem.texture);
	_shaderData.shineness = material->getShineness();
	_shaderData.lightBackfaces = _lightBackfaces;
	_shaderData.kA =  (_ambientLight.color / 255) * material->getAmbient();
	_shaderData.objectColor =  material->getObjectColor() / 255;

	if (_shaderData.textureSampler.isBound()) 
	{
		_shaderData.textureSampler.setScale(
			currentItem.texture->getWidth() * currentItem.texScaleX, 
			currentItem.texture->getHeight() * currentItem.texScaleY
		);
	}

	_shaderData.sampleMode = _texSampleMode;
	_shaderData.facesReversed = translateFaceType(FACE_FRONT) == FACE_BACK && translateFaceType(FACE_BACK) == FACE_FRONT;
	_shaderData.forceFrontFaces = translateFaceType(FACE_BACK) == FACE_FRONT && translateFaceType(FACE_FRONT) == FACE_FRONT;
}
