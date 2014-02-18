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
#include "common/Mat4.h"
#include "common/Vector4.h"

#include "renderer/Renderer.h"
#include "Shaders.h"
#include "Engine.h"
#include <assert.h>

#include "model/Model.h"
#include "IO/ObjLoader.h"

Engine::Engine( void ) : 

	// settings
	_drawWireFrame(false), 
	_drawPolygonNormals(false), 
	_drawVertexNormals(false), 
	_drawBoundingBox(false), 
	_drawSeparateObjects(false),
	_drawAxes(false),
	_shadingMode(SHADING_PHONG),
	_backfaceCulling(false),
	_depthRendering(false),
	_perspectiveCorrect(true),
	_invertedVertexNormals(false),
	_invertedPolygonNormals(false),
	_tile_background(true),
	_lightBackfaces(false),
	_backgroundTexture(NULL),
	_texSampleMode(TMS_BILINEAR_MIPMAPS),
	_force_all_front_faces(false),
	_draw_light_sources(false),

	// models
	_itemCount(0), _sceneItems(NULL),
	_selectedObject(-1),
	
	// box and axes generic model
	_sceneBoxModel(NULL),
	_axesModel(NULL),

	// outputs
	_outputSizeX(0),
	_outputSizeY(0),
	_outputTexture(NULL),
	_outputZBuffer(NULL),
	_outputSelBuffer(NULL),
	_shadowMapsValid(false),
	_normalsScale (0.06)
{
	resetLighting();
	resetMaterials();
	resetBackground();
	resetFog();
	createLighSourcesModels();
	_projectionTransform.setPerspectiveEnabled(true);

	for (int i = 0 ; i < MAX_LIGHT*6 ; i++)
		_shadowMaps[i] = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::setRenderer(Renderer *renderer)
{
	_renderer = renderer;
}

void Engine::setOutput(Texture* output, int width, int height)
{

	_outputTexture = output;

	// reallocate Z buffer
	if (!_outputZBuffer || width > _outputZBuffer->getWidth() || height > _outputZBuffer->getHeight())
	{
		if (_outputZBuffer)
			delete _outputZBuffer;
		_outputZBuffer = new DepthTexture(width, height);
		/* no need to clear the Z buffer as it will not be used by anyone but the render portion where it will be cleared */
	}

	if (!_drawSeparateObjects) 
	{
		// no need for selection buffer for normal mode
		if (_outputSelBuffer)
			delete _outputSelBuffer;
		_outputSelBuffer = NULL;

	} else {

		// allocate or reallocate the selection buffer if needed
		if (!_outputSelBuffer || width > _outputSelBuffer->getWidth() || height > _outputSelBuffer->getHeight())
		{
			if (_outputSelBuffer)
				delete _outputSelBuffer;
			_outputSelBuffer = new IntegerTexture(width, height);
			_outputSelBuffer->clear();
		}
	}

	_outputSizeX = width;
	_outputSizeY = height;
	updateOutputs();
}



void Engine::updateOutputs()
{
	_renderer->setViewport(_outputSizeX, _outputSizeY);
	_renderer->setZBuffer(_outputZBuffer);
	_renderer->setSelBuffer(_outputSelBuffer);
	_renderer->setOutputTexture(_outputTexture);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::~Engine( void )
{
	resetScene();
	delete _light_dir_model;
	delete _light_point_model;
	delete _light_spot_model;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::loadDebugScene()
{
	resetScene();
	_sceneItems = new SceneItem[10];

	// one triangle 
	Vector3 v4(-1,0.5,0), v5(1,0.5,0.1), v6(0, -1, 0);
	Model* m = Model::createTriangleModel(v4, v5, v6);
	_sceneItems[_itemCount++]._mainModel = m;

	m->_defaultMaterial.setObjectColor(Color(0,255,0));

	m->vertices[0].texCoord = Vector3(0,0,0);
	m->vertices[1].texCoord = Vector3(1,0,0);
	m->vertices[2].texCoord = Vector3(1,1,0);


	processScene();
	resetTransformations();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Engine::loadSceneFromOBJ( const char* file)
{
	// clear the existing model if already loaded once
	resetScene();

	ObjLoader loader;
	loader.load(file);


	// initialize our model array
	_itemCount = loader.models.size();
	_sceneItems = new SceneItem[_itemCount];

	// copy pointers to models to our array and calculate the box of the model
	int i = 0;
	for (auto it = loader.models.begin() ; it !=  loader.models.end() ; it++, i++)
		_sceneItems[i]._mainModel = *it;


#if 0
	/* load lights from IRIT
	*/

	if (iritLights.size()) {

		resetLighting();

		int i = 0;
		for (auto iter = iritLights.begin() ; iter != iritLights.end() ; iter++, i++) {
			_lightParams[i] = *iter;
			_lightParams[i].enabled = true;
		}

		iritLights.clear();
	}
#endif

	processScene();
	resetTransformations();
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::processScene() 
{
	// calculate per model bounding boxes and overall bounding box
	for (unsigned int i = 0 ; i < _itemCount ; i++) 
	{
		_sceneItems[i]._modelBox = _sceneItems[i]._mainModel->getBoundingBox();

		if (i > 0)
			_initialsceneBox += _sceneItems[i]._modelBox;
		else
			_initialsceneBox = _sceneItems[i]._modelBox;
	}

	// now move all objects to their boxes and create their bounding boxes
	for (unsigned int i = 0 ; i < _itemCount ; i++) 
	{
		SceneItem &item = _sceneItems[i];

		item._material.setBase(&_globalObjectMaterial);
		item._material.setDefault(&item._mainModel->_defaultMaterial);


		// calculate the position of the model in the world, so it would appear in same place
		// as it was before
		item._position =  item._modelBox.getCenter() - _initialsceneBox.getCenter();

		// move model and the bounding box to the origin
		item._mainModel->moveTo(item._modelBox.getCenter());
		item._modelBox.moveTo(item._modelBox.getCenter());

		// and now create box model for the box.
		item._boxModel = WireFrameModel::createBoxModel(item._modelBox, Color(0,0,1));
	}

	// load textures
	reloadTextures();


	Vector3 center = _initialsceneBox.getCenter();
	_initialsceneBox.moveTo(center);
	_sceneBox = _initialsceneBox;
	_sceneBoxModel = WireFrameModel::createBoxModel(_sceneBox, Color(0,0,1));
	_axesModel = WireFrameModel::createAxisModel(0.5 / calculateInitialScaleFactor());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::resetScene()
{
	if (_itemCount) {
		_itemCount = 0;
		delete [] _sceneItems;
	}

	_sceneItems = NULL;

	// and global scene model
	if (_sceneBoxModel)
		delete _sceneBoxModel;

	if (_axesModel)
		delete _axesModel;

	_sceneBoxModel = NULL;
	_axesModel = NULL;

	_invertedPolygonNormals = false;
	_invertedVertexNormals = false;

	invalidateShadowMaps();
	resetBackground();
	freeShadowMaps();

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::setDrawSeperateObjects( bool enable )
{
	if (enable == _drawSeparateObjects)
		return;

	_drawSeparateObjects = enable;
	if (_drawSeparateObjects)
		_selectedObject = -1;

	// update output buffers
	setOutput(_outputTexture, _outputSizeX, _outputSizeY);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Engine::selectObject( int mouseCX, int mouseCY )
{
	/* given X,Y in screen space, attempt to select an object by casting a ray.
		returns true if object was selected and its a different one (e.g redraw is needed
	*/

	if (!_itemCount)
		return false;

	/* the current candidate fro selection */
	assert(_outputSelBuffer);

	int candidateObject = _outputSelBuffer->getPixelValue(mouseCX,mouseCY) - 1;

	// didn't select anything or selected the same object
	if ((_selectedObject == candidateObject) || (candidateObject != -1 && candidateObject >= (int)_itemCount))
		return false;

	_selectedObject = candidateObject;
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::invertVertexNormals( bool enable )
{
	if (enable == _invertedVertexNormals)
		return;

	for (unsigned int i = 0 ; i < _itemCount ; i++)
		_sceneItems[i]._mainModel->invertVertexNormals();

	_invertedVertexNormals = enable;

	invalidateNormalModels();
}

void Engine::invertPolygonNormals( bool enable )
{
	if (enable == _invertedPolygonNormals)
		return;

	for (unsigned int i = 0 ; i < _itemCount ; i++)
		_sceneItems[i]._mainModel->invertPolygonNormals();

	_invertedPolygonNormals = enable;
	invalidateNormalModels();
}

void Engine::invalidateNormalModels()
{
	for (unsigned int i = 0 ; i < _itemCount ; i++) 
	{
		SceneItem &item = _sceneItems[i];

		if (item._polygonNormalModel)
			delete item._polygonNormalModel;
		if (item._vertexNormalModel)
			delete item._vertexNormalModel;

		item._polygonNormalModel = NULL;
		item._vertexNormalModel = NULL;
	}
}

void Engine::createNormalModels()
{
	double scalefactor = _normalsScale / calculateInitialScaleFactor();

	for (unsigned int i = 0 ; i < _itemCount ; i++) 
	{
		SceneItem &item = _sceneItems[i];

		Mat4 normalScale = _globalObjectTransform.getInvScaleMatrix() * item._modelTransform.getInvScaleMatrix();

		try {
			if (_drawVertexNormals && !item._vertexNormalModel) 
				item._vertexNormalModel = WireFrameModel::createVertexNormalModel(item._mainModel,scalefactor,normalScale);
		} catch(...) {
			item._vertexNormalModel = NULL;
			invalidateNormalModels();
		}

		try {
			if (_drawPolygonNormals && !item._polygonNormalModel) 
				item._polygonNormalModel = WireFrameModel::createPolygonNormalModel(item._mainModel, scalefactor,normalScale);
		} catch(...) {
			item._polygonNormalModel = NULL;
			invalidateNormalModels();

		}
	}
}

