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
#include "common/Mat4.h"

void Engine::setupTransformationShaderData( int objectID )
{
	// setup transform matrices for vertex shader
	// if objectID <0 then only take in account globlal transformations

	SceneItem &item = _sceneItems[objectID];
	Mat4 objectTransform = objectID >= 0 ? item._itemTR.getMat() : Mat4::createUnit();
	Mat4 objectNormalTransform = objectID >= 0 ? item._itemTR.getNormalTransformMatrix() : Mat4::createUnit();

	// setup transformations
	_shaderData.mat_objectToCameraSpace =
		objectTransform *  _mainTR.getMat() * _cameraTR.getMat();


	_shaderData.mat_objectToClipSpaceTransform =  
		_shaderData.mat_objectToCameraSpace * _projTR.getMatrix();

	_shaderData.mat_objectToCameraSpaceNormalTransform =  objectNormalTransform * 
		_mainTR.getNormalTransformMatrix() * _cameraTR.getNormalTransformMatrix();

	_shaderData.mat_cameraToWorldSpace = _cameraTR.getMat().inv();
	_shaderData.mat_cameraToObjectSpace = (_shaderData.mat_cameraToWorldSpace * _mainTR.getMat()).inv();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::resetTransformations() 
{
	Vector3 boxSizes = _initialsceneBox.getSizes();

	// reset per object transformation and move objects to their positions
	for (unsigned int i = 0 ; i < _itemCount ; i++) {
		_sceneItems[i]._itemTR.reset();
		_sceneItems[i]._itemTR.setMoveFactors(_sceneItems[i]._position);
	}

	// reset global object transformation
	_mainTR.reset();

	// set camera params
	double distance = boxSizes.x(); // distance from front boundary of scene box and camera
	_projTR.setFrontPlane(distance, distance/ (boxSizes.x() / boxSizes.y()));
	_projTR.setDistance(distance);


	// set depth of the clip box to length of diagonal of the scene box
	double depth = boxSizes.z();
	_projTR.setDepth(depth);
	_projTR.setPerspectiveEnabled(true);

	// distance from center of scene box and camera
	_cameraTR.reset();
	_cameraTR.setMoveFactors(Vector3(0,0, boxSizes.x()+depth/2));

	recomputeBoundingBox();
	recomputeDepth();

	invalidateNormalModels();
	invalidateShadowMaps();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::scaleObject( int axis, double delta )
{
	if(!_itemCount) return;

	/* TODO: implement adaptive scale*/

	ObjectTransformation *t = (_drawSeparateObjects && _selObj != -1) ?
			&_sceneItems[_selObj]._itemTR : &_mainTR;

	// take in account that scene might be already scaled, so need to keep sane scaling speed

	double factor = 1.0;
	if (_drawSeparateObjects) factor *= _mainTR.getScaleFactors()[axis];

	factor /= calculateInitialScaleFactor();

	Vector3 scale = t->getScaleFactors();
	scale[axis] += (delta / factor);

	// don't allow negative scale and limit scale a bit
	if (scale[axis] <= 0.001)
		scale[axis] = 0.001;

	t->setScaleFactors(scale);
	invalidateNormalModels();

	if (_drawSeparateObjects && _selObj != -1)
		recomputeBoundingBox();

	recomputeDepth();
	invalidateShadowMaps();

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::rotateObject( int axis, double angleDelta )
{
	if(!_itemCount || !rotationAxisEnabled(axis)) return;

	rotCoofs[axis] -= ((M_PI) * angleDelta / 180);

	if (_drawSeparateObjects && _selObj != -1)
	{
		/* apply to selected item*/
		_sceneItems[_selObj]._itemTR.setRotationMatrix2 (
			_mainTR.getRotMat() *_cameraTR.getRotMatI() *
			Mat4::getRotMat(rotCoofs) *
			_cameraTR.getRotMatI().inv() * _mainTR.getRotMat().inv()
		);

		recomputeBoundingBox();
	} else {
		/* apply to whole world*/
		_mainTR.setRotationMatrix2 (_cameraTR.getRotMatI() *
				Mat4::getRotMat(rotCoofs) * _cameraTR.getRotMatI().inv()
		);
	}

	recomputeDepth();
	invalidateShadowMaps();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::rotateCamera( int axis, double angleDelta )
{
	if(!_itemCount || !rotationAxisEnabled(axis)) return;

	Vector3 rotCoofs1 = _cameraTR.getRotateFactors();
	rotCoofs1[axis] -= ((M_PI) * angleDelta / 180);
	_cameraTR.setRotationFactors(rotCoofs1);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::commitRotation()
{
	if(!_itemCount) return;

	if (_drawSeparateObjects && _selObj != -1)
		_sceneItems[_selObj]._itemTR.mergeRotationFactors();

	_mainTR.mergeRotationFactors();
	rotCoofs = Vector3(0,0,0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::moveObject( int axis, double delta )
{
	if(!_itemCount) return;

	if (_drawSeparateObjects && _selObj != -1) {
		Vector3 sceneCenter = _sceneItems[_selObj]._itemTR.getMoveFactors();
		sceneCenter = vmul3point(sceneCenter, _mainTR.getMat() * _cameraTR.getMat());
		sceneCenter[axis] += delta;
		sceneCenter = vmul3point(sceneCenter, _cameraTR.getMat().inv() * _mainTR.getMat().inv());
		_sceneItems[_selObj]._itemTR.setMoveFactors(sceneCenter);
		recomputeBoundingBox();
	} else {
		// global object move
		Vector3 sceneCenter = _mainTR.getMoveFactors();
		sceneCenter = vmul3point(sceneCenter, _cameraTR.getMat());
		sceneCenter[axis] += delta;
		sceneCenter = vmul3point(sceneCenter, _cameraTR.getMat().inv());
		_mainTR.setMoveFactors(sceneCenter);
	}

	invalidateShadowMaps();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::moveCamera( int axis, double delta )
{
	Vector3 cameraLoc = _cameraTR.getMoveFactors();
	cameraLoc = vmul3point(cameraLoc, _cameraTR.getRotationMatrix());
	cameraLoc[axis] -= delta;
	cameraLoc = vmul3point(cameraLoc, _cameraTR.getRotationMatrix().inv());
	_cameraTR.setMoveFactors(cameraLoc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::setPerspectiveD( double d )
{
	_projTR.setDistance(d);
}

double Engine::calculateInitialScaleFactor() const 
{
	Vector3 sizes = _initialsceneBox.getSizes();
	double boxScalingFactor = max(sizes[0], max(sizes[1], sizes[2]));
	return (1 / boxScalingFactor) * 2;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Engine::recomputeBoundingBox() 
{
	if (!_itemCount)
		return;

	/* calculate new scene bounding box*/
	_sceneBox = _sceneItems[0]._modelBox * _sceneItems[0]._itemTR.getMat();
	for (unsigned int i = 1 ; i < _itemCount ; i++)
		_sceneBox += (_sceneItems[i]._modelBox * _sceneItems[i]._itemTR.getMat());

	/* and create wireframe model for it*/
	delete _sceneBoxModel;
	_sceneBoxModel = WireFrameModel::createBoxModel(_sceneBox, Color(0,0,1));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::recomputeDepth() 
{
#if 0
	Vector3 sizes = (_initialsceneBox * _mainTR.getMat()).getSizes();
	double depth = sizes.z();

	Vector3 move = _cameraTR.getMoveFactors();
	move.z() -= (_projTR.getDepth() / 2);
	move.z() += (depth / 2);
	_cameraTR.setMoveFactors(move);
	_projTR.setDepth(depth);
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

Vector3 Engine::getSteps( double X, double Y)
{
	if (!_itemCount) return Vector3(0,0,0);

	Mat4 additinalMat = (_drawSeparateObjects && _selObj != -1) ?
			_sceneItems[_selObj]._itemTR.getMat() : Mat4::createUnit();

	/*
	 * TODO: BUG - if center of the world is behind the camera, then projection transform is wrong.
	 * need to think about this
	 */

	Vector4 center = Vector4(0,0,0,1) * additinalMat *
			(_mainTR.getMat() * _cameraTR.getMat() * _projTR.getMatrix());

	center.canonicalize();
	center = center * _renderer->getNDCTODeviceMatrix();

	double Z = center.z();

	Vector4 res = (Vector4(X,Y,Z,1) * _renderer->getDeviceToScreenMatrix() * _projTR.getMatrix().inv());
	res.canonicalize();

	Vector4 res2 = (Vector4(X+1,Y+1,Z,1) * _renderer->getDeviceToScreenMatrix() * _projTR.getMatrix().inv());
	res2.canonicalize();

	Vector3 result = (res2 - res).xyz();

	BOUNDING_BOX box = _sceneBox * (_mainTR.getMat() * _cameraTR.getMat());
	double depth = box.getSizes().z();
	result.z() = depth / max (_outputSizeX, _outputSizeY);


	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

Engine::FACE_TYPE Engine::translateFaceType( FACE_TYPE given )
{
	if (_flags.forceFrontFaces)
		return FACE_FRONT;

	bool b = (bool)given;
	return (FACE_TYPE)(b ^ _invertFaces ^ _cameraTR.getInvert());
}

bool Engine::rotationAxisEnabled(int axis)
{
	switch(axis) {
	case 0: return _rotMode & ROTATION_X;
	case 1: return _rotMode & ROTATION_Y;
	case 2: return _rotMode & ROTATION_Z;
	default:
		assert(0);
	}
}

