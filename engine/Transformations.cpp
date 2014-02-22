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
#include "Engine.h"


void Engine::setupTransformationShaderData( int objectID )
{
	// setup transform matrices for vertex shader
	// if objectID <0 then only take in account globlal transformations

	SceneItem &item = _sceneItems[objectID];
	Mat4 objectTransform = objectID >= 0 ? item._modelTransform.getMatrix() : Mat4::createUnit();
	Mat4 objectNormalTransform = objectID >= 0 ? item._modelTransform.getNormalTransformMatrix() : Mat4::createUnit();

	// setup transformations
	_shaderData.mat_objectToCameraSpace =
		objectTransform *  _globalObjectTransform.getMatrix() * _cameraTransform.getMatrix();


	_shaderData.mat_objectToClipSpaceTransform =  
		_shaderData.mat_objectToCameraSpace * _projectionTransform.getMatrix();

	_shaderData.mat_objectToCameraSpaceNormalTransform =  objectNormalTransform * 
		_globalObjectTransform.getNormalTransformMatrix() * _cameraTransform.getNormalTransformMatrix();

	_shaderData.mat_cameraToWorldSpace = _cameraTransform.getMatrix().inverse();
	_shaderData.mat_cameraToObjectSpace = (_shaderData.mat_cameraToWorldSpace * _globalObjectTransform.getMatrix()).inverse();

	//_shaderData.projNormaltransform = _shaderData.mat_objectToCameraSpaceNormalTransform * _projectionTransform.getMatrix().inverse().transpose();
}

void Engine::resetTransformations() 
{
	Vector3 boxSizes = _initialsceneBox.getSizes();

	// reset per object transformation and move objects to their positions
	for (unsigned int i = 0 ; i < _itemCount ; i++) {
		_sceneItems[i]._modelTransform.reset();
		_sceneItems[i]._modelTransform.setTranslation(_sceneItems[i]._position);
	}

	// reset global object transformation
	_globalObjectTransform.reset();

	// set camera params
	double distance = boxSizes.x(); // distance from front boundary of scene box and camera
	_projectionTransform.setFrontPlane(distance, distance/ (boxSizes.x() / boxSizes.y()));
	_projectionTransform.setDistance(distance);


	// set depth of the clip box to length of diagonal of the scene box
	double depth = boxSizes.z();
	_projectionTransform.setDepth(depth);
	_projectionTransform.setPerspectiveEnabled(true);

	// distance from center of scene box and camera
	_cameraTransform.reset();
	_cameraTransform.setTranslation(Vector3(0,0, boxSizes.x()+depth/2));

	recomputeBoundingBox();
	recomputeDepth();

	invalidateNormalModels();
	invalidateShadowMaps();
}

Transformations::AffineTransformation * Engine::getTransformationSettings()
{
	if (_drawSeparateObjects) {
		if (_selectedObject != -1)
			return &_sceneItems[_selectedObject]._modelTransform;
		return &_globalObjectTransform;
	} else
		return &_globalObjectTransform;
}

void Engine::rotateObject( int axis, double angleDelta )
{
	if(!_itemCount) return;

	Transformations::AffineTransformation *t = getTransformationSettings();
	if (!t) return; 

	/* account for inverted depth mode*/
	if (_cameraTransform.getInvert() && (axis == 0 || axis == 1))
		angleDelta *= -1;

	/* inverse object rotation so it appears a bit more friendly (more camera related)*/
	Vector3 rotation = t->getRotation();
	rotation[axis] -= (M_PI) * angleDelta / 180;
	t->setRotation(rotation);

	if (_drawSeparateObjects && _selectedObject != -1)
		recomputeBoundingBox();

	recomputeDepth();
	invalidateShadowMaps();
}

void Engine::scaleObject( int axis, double delta )
{
	if(!_itemCount) return;

	Transformations::AffineTransformation *t = getTransformationSettings();
	if (!t) return; 

	// take in account that scene might be already scaled, so need to keep sane scaling speed

	double factor = 1.0;
	if (_drawSeparateObjects) factor *= _globalObjectTransform.getScale()[axis];

	factor /= calculateInitialScaleFactor();

	Vector3 scale = t->getScale();
	scale[axis] += (delta / factor);

	// don't allow negative scale and limit scale a bit
	if (scale[axis] <= 0.001)
		scale[axis] = 0.001;

	t->setScale(scale);
	invalidateNormalModels();

	if (_drawSeparateObjects && _selectedObject != -1)
		recomputeBoundingBox();

	recomputeDepth();
	invalidateShadowMaps();

}

void Engine::moveObject( int axis, double delta )
{
	if(!_itemCount) return;

	Transformations::AffineTransformation *t = getTransformationSettings();
	if (!t) return; 

	// take in account that object might be scaled, so change step accordantly
	double factor = 1.0;
	if (_drawSeparateObjects) factor *= _globalObjectTransform.getScale()[axis];

	Vector3 move = t->getTranlation();
	move[axis] += (delta / factor);
	t->setTranslation(move);

	if (_drawSeparateObjects && _selectedObject != -1)
		recomputeBoundingBox();

	invalidateShadowMaps();
}

void Engine::rotateCamera( int axis, double angleDelta )
{
	Vector3 rotation = _cameraTransform.getRotation();
	rotation[axis] += (M_PI) * angleDelta / 180;
	_cameraTransform.setRotation(rotation);
}

void Engine::moveCamera( int axis, double delta )
{
	Vector3 move = _cameraTransform.getTranlation();
	move[axis] += delta;
	_cameraTransform.setTranslation(move);
}

void Engine::setPerspectiveD( double d )
{
	_projectionTransform.setDistance(d);
}

double Engine::calculateInitialScaleFactor() const 
{
	Vector3 sizes = _initialsceneBox.getSizes();
	double boxScalingFactor = max(sizes[0], max(sizes[1], sizes[2]));
	return (1 / boxScalingFactor) * 2;
}


void Engine::recomputeBoundingBox() 
{

	if (!_itemCount)
		return;

	BOUNDING_BOX box;

	for (unsigned int i = 0 ; i < _itemCount ; i++) 
	{
		SceneItem &item = _sceneItems[i];

		BOUNDING_BOX itemBox = item._modelBox * item._modelTransform.getMatrix();

		if (i > 0) 
			box += itemBox;
		else
			box = itemBox;
	}

	_sceneBox = box;

	if (_sceneBoxModel)
		delete _sceneBoxModel;

	_sceneBoxModel = WireFrameModel::createBoxModel(_sceneBox, Color(0,0,1));
}

void Engine::recomputeDepth() 
{
	//return;
	Vector3 sizes = (_initialsceneBox * _globalObjectTransform.getMatrix()).getSizes();
	double depth = sizes.z();

	Vector3 move = _cameraTransform.getTranlation();
	move.z() -= (_projectionTransform.getDepth() / 2);
	move.z() += (depth / 2);
	_cameraTransform.setTranslation(move);

	_projectionTransform.setDepth(depth);
}


Vector3 Engine::deviceToNDC( double X, double Y, double Z )
{
	if (!_itemCount)
		return Vector3(0,0,0);

	Mat4 mat = _renderer->getDeviceToScreenMatrix() * _projectionTransform.getMatrix().inverse();

	Vector4 res = (Vector4(X,Y,Z,1) * mat);
	res.canonicalize();
	return res.xyz();
}

Engine::FACE_TYPE Engine::translateFaceType( FACE_TYPE given )
{
	if (_lightAllFaces)
		return FACE_FRONT;

	bool b = (bool)given;
	return (FACE_TYPE)(b ^ _invertFaces ^ _cameraTransform.getInvert());
}
