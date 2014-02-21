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
#include "common/Transformations.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::updateShadowMaps() 
{
	if (!_itemCount || _shadowMapsValid)
		return;

	for (int i = 0 ; i < MAX_LIGHT ; i++) 
	{
		LightParams &lp = _lightParams[i];
		if (!lp.shadow || !lp.enabled)
			continue;

		/* check the direction for invalid data */
		if (lp.type != LIGHT_TYPE_POINT && lp.direction.len() == 0)
			continue;

		/* Translate direction and position to correct space */
		Vector3 direction = lp.direction, position = lp.position;

		if (lp.space == LIGHT_SPACE_LOCAL) 
		{
			if (lp.type != LIGHT_TYPE_POINT) 
				direction = vmul3dir(direction,_globalObjectTransform.getNormalTransformMatrix());
			if (lp.type != LIGHT_TYPE_DIRECTIONAL)
				position = vmul3point(position ,_globalObjectTransform.getMatrix());
		}

		direction.makeNormal();

		switch (lp.type) {
		case LIGHT_TYPE_DIRECTIONAL:
			createShadowMap(i*6, -direction, Vector3(0,0,0), false,  0);
			break;
		case LIGHT_TYPE_SPOT:
			createShadowMap(i*6, -direction, position, true, lp.cutoffAngle);
			break;
		case LIGHT_TYPE_POINT:
			createShadowMap(i*6+0,Vector3(-1,0,0), position, true, 100);
			createShadowMap(i*6+1,Vector3(+1,0,0), position, true, 100);
			createShadowMap(i*6+2,Vector3(0,-1,0), position, true, 100);
			createShadowMap(i*6+3,Vector3(0,+1,0), position, true, 100);
			createShadowMap(i*6+4,Vector3(0,0,-1), position, true, 100);
			createShadowMap(i*6+5,Vector3(0,0,+1), position, true, 100);
			break;
		default:
			assert(0);
		}
	}

	_shadowMapsValid = true;
}

void Engine::invalidateShadowMaps()
{
	_shadowMapsValid = false;
}

void Engine::freeShadowMaps()
{
	for (int i = 0 ; i < MAX_LIGHT * 6 ; i++) {
		delete _shadowMaps[i];
		_shadowMaps[i] = NULL;
	}
}

void Engine::setShadowParams( ShadowParams * params )
{
	_shadowParams = *params; 
	invalidateShadowMaps();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ShadowMapUniformBuffer 
{
	Mat4 mat_objectToLightSpace;
};


static void shadowMapVertexShader( void* priv, void* in, Vector4 &pos_out, Vector3 attribs_out[] )
{
	const ShadowMapUniformBuffer *u = (const ShadowMapUniformBuffer*)priv;
	const Model::Vertex& v = *(const Model::Vertex*)in;
	pos_out = vmul4point(v.position, u->mat_objectToLightSpace);
}

void Engine::createShadowMap( int i, const Vector3 &direction, const Vector3 &position, bool projective, double maxFov )
{
	// ----------------------------- Setup camera matrix----------------------------------------------------

	// calculate angle of light direction vs direction toward sky
	double cameraAngleVsup = direction.dot(Vector3(0,1,0));

	// set the camera 'up' direction to be ether UP or RIGHT depending which is father in angle distance
	Vector3 up =  (abs(cameraAngleVsup) < (sqrt(2.0) / 2)) ? Vector3(0,-1,0) : Vector3(-1,0,0 );

	// set up camera matrix that we will apply on the scene (so its inverse)
	Mat4 cameraMatrix = Transformations::lookAt(position, direction, up).inverse();

	// calculate bounding box of the scene from POV of camera using just created camera matrix
	BOUNDING_BOX sceneBoxFromLightPOV = _sceneBox *  (_globalObjectTransform.getMatrix() * cameraMatrix);

	double rangeMax = 0;
	double rangeMin = 0;

	// setup projection
	Mat4 proj;

	if (projective) 
	{
		// this is used for point and spot light (which are very similar in this regard)

		const Vector3 center = sceneBoxFromLightPOV.getCenter();

		// since camera is at origin, the direction towards the scene center is its position
		const Vector3 directionToSceneCenter = (center).returnNormal();

		// and distance to scene center is once again just len of location of scene center
		double distanceToSceneCenter = (center).len();

		// and now calculate radius of the sphere that is guaranteed to contain whole scene
		double sceneRadius = sceneBoxFromLightPOV.getSizes().len() / 2;

		if (distanceToSceneCenter > sceneRadius) {

			// we are outside of the scene for sure, so we try to minimize FOV

			// this is the angle between viewing direction (-z once again) and direction toward scene center
			double sceneCenterHalfFOV = acos((Vector3(0,0,-1)).dot(directionToSceneCenter));

			// this is the angle from direction to scene center to direction to its tangent
			double sceneHalfFOV = asin(sceneRadius/distanceToSceneCenter);

			// and so sum of both angles is half of maximum FOV
			double optimalFOV = ((sceneCenterHalfFOV + sceneHalfFOV) * 2 )  / (M_PI) * 180;

			if (optimalFOV < maxFov)
				maxFov = optimalFOV;

			rangeMax = abs(sceneBoxFromLightPOV.point1.z());
			rangeMin = abs(sceneBoxFromLightPOV.point2.z());
		} else 
		{
			// otherwise maximum needed distance is the radius of the scene
			rangeMax = sceneRadius;

			// and minimum distance is ideally 0, but as we can't use that... :-(
			rangeMin = rangeMax / 20;
		}

		proj = Transformations::getPerspectiveMatrix(maxFov, 1, rangeMin, rangeMax);
	} else {
		proj = Transformations::getOrthoProjMatrix(sceneBoxFromLightPOV.point1.x(), sceneBoxFromLightPOV.point2.x(), sceneBoxFromLightPOV.point2.y(), sceneBoxFromLightPOV.point1.y(), sceneBoxFromLightPOV.point1.z(), sceneBoxFromLightPOV.point2.z());
	}

	_shadowMapsMatrices[i] = cameraMatrix * proj;

	// 4. setup output map
	if (_shadowMaps[i] == NULL || _shadowMaps[i]->getWidth() != _shadowParams.shadowMapRes ||
			_shadowMaps[i]->getHeight() != _shadowParams.shadowMapRes)
	{
		delete _shadowMaps[i];
		_shadowMaps[i] = new DepthTexture(_shadowParams.shadowMapRes, _shadowParams.shadowMapRes);
	}

	_shadowMaps[i]->clear();


	// 5. setup common render settings
	_renderer->setDebugDepthRendering(false);
	_renderer->setViewport(_shadowParams.shadowMapRes, _shadowParams.shadowMapRes);
	_renderer->setAspectRatio(1);
	_renderer->setBackFaceCulling(false);
	_renderer->setFrontFaceCulling(false);
	_renderer->setOutputTexture(NULL);
	_renderer->setSelBuffer(NULL);
	_renderer->setZBuffer(_shadowMaps[i]);

	// 6. setup shaders 
	ShadowMapUniformBuffer uniforms;
	_renderer->setVertexShader(shadowMapVertexShader, &uniforms);
	_renderer->setPixelShader(NULL, NULL);
	_renderer->setVertexAttributes(0,0,0);

	// 7. rendering loop
	for (unsigned int i = 0 ; i < _itemCount; i++) 
	{
		SceneItem &item = _sceneItems[i];
		uniforms.mat_objectToLightSpace = item._modelTransform.getMatrix() * _globalObjectTransform.getMatrix() * cameraMatrix * proj;
		_renderer->uploadVertices(item._mainModel->vertices, sizeof(Model::Vertex), item._mainModel->getNumberOfVertices());
		_renderer->renderPolygons(item._mainModel->polygons, item._mainModel->getNumberOfPolygons(),-1);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::setupShadowMapShaderData( int objectID )
{
	_shaderData.shadowParams = _shadowParams;

	int lightID = 0;
	for (int i = 0 ; i < MAX_LIGHT ; i++) 
	{
		LightParams &lp = _lightParams[i];
		if (!lp.enabled) continue;

		ShaderLightData &light = _shaderData.lights[lightID++];
		light._shadowMapSampler.bindTexture(NULL);
		light._shadowCubemapSampler.bindTextures(NULL);

		if (!lp.shadow)
			continue;

		static const Mat4 clipToTextureSpace = 
			Mat4 (
			0.5, 0.0, 0.0, 0.0,
			0.0, 0.5, 0.0, 0.0,
			0.0, 0.0, 0.5, 0.0,
			0.5, 0.5, 0.5, 1.0
			);

		if (lp.type != LIGHT_TYPE_POINT) {
			light._shadowMapSampler.bindTexture(_shadowMaps[i*6]);
			light.shadowMapTransfrom[0] = 
				
				_shaderData.mat_cameraToWorldSpace * _shadowMapsMatrices[i*6] * clipToTextureSpace;
					
		} else 
		{
			light._shadowCubemapSampler.bindTextures((const DepthTexture **)(_shadowMaps+i*6));
			for (int face = 0 ; face < 6 ; face++)
				light.shadowMapTransfrom[face] = 
					_shaderData.mat_cameraToWorldSpace *_shadowMapsMatrices[i*6+face] * clipToTextureSpace;
		}		
	}
}
