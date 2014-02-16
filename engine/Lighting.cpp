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
#include "model/Material.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::setupLightingShaderData(int objectID) 
{
	SceneItem &currentItem = _sceneItems[objectID];
	MaterialParams *material = &currentItem._material;

	/* setup other lights*/
	_shaderData.lightsCount = 0;
	for (int i = 0 ; i < MAX_LIGHT ; i++) 
	{
		LightParams &lp = _lightParams[i];

		if (!lp.enabled)
			continue;

		ShaderLightData &light = _shaderData.lights[_shaderData.lightsCount++];
		light.is_point = lp.type == LIGHT_TYPE_POINT || lp.type == LIGHT_TYPE_SPOT;
		light.is_spot = lp.type == LIGHT_TYPE_SPOT;

		if (lp.type == LIGHT_TYPE_SPOT) {
			light.cutoffCOsine = cos(lp.cutoffAngle / 2 * M_PI / 180);
			light.startCutofAttenuationCosine = cos(lp.cutoffAngle / 3 * M_PI / 180);
		}

		// transform direction and location of lights to camera space
		/* we are given true light direction, but we need opposite light direction for light model (eg : direction to light source)*/

		if (lp.space == LIGHT_SPACE_LOCAL ) 
		{
			light.direction = -(lp.direction *  _globalObjectTransform.getNormalTransformMatrix()  * _cameraTransform.getNormalTransformMatrix());
			light.location = (toHomoCoords(lp.position) * _globalObjectTransform.getMatrix() *  _cameraTransform.getMatrix()).xyz();
		} else {
			light.direction = -(lp.direction * _cameraTransform.getNormalTransformMatrix());
			light.location = (toHomoCoords(lp.position) *  _cameraTransform.getMatrix()).xyz();
		}

		light.kD = (lp.color / 255) * material->getDiffuse();
		light.kS = (lp.color / 255) * material->getSpecular();
		light.direction.makeNormal();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Engine::resetLighting()
{
	_ambientLight.reset();
	for (int i = 0 ; i < MAX_LIGHT ; i++)
		_lightParams[i].reset();

	// setup light #0 as said in 

	_lightParams[0].enabled = true;
	_lightParams[0].type = LIGHT_TYPE_DIRECTIONAL;
	_lightParams[0].space = LIGHT_SPACE_VIEW;
	_lightParams[0].direction = Vector3(0,0,-1);
	_lightParams[0].color = Color(255,255,255);
	invalidateShadowMaps();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////


Color doLighting( const UniformBuffer* u, Color &objcolor, const Vector4 & pos, Vector3 &normal, bool backface )
{

	Color c = u->kA * objcolor;

	// dealing with backfaces...
	if (backface && !u->lightBackfaces) 
		return c.clamp();

	if (backface)
		normal = - normal;

	for (int i = 0 ; i < u->lightsCount ; i++) 
	{
		const ShaderLightData &light = u->lights[i];

		// Calculate the direction the light is coming from (from point toward the light source)
		const Vector3 &lightDirection = light.is_point ? (light.location - pos.xyz()).returnNormal() : light.direction;

		// calculate the angle of incoming light vs surface normal
		double surfaceLightAngleCosine = lightDirection.dot(normal);
		if (surfaceLightAngleCosine <= 0) continue;

		// For spot lights, check if we are in the cone of the light, and calculate attenuation factor
		double factor = 1;

		if (u->lights[i].is_spot) 
		{
			double lightAngleFromDirectionCosine = lightDirection.dot(u->lights[i].direction);
			if (lightAngleFromDirectionCosine <=  u->lights[i].cutoffCOsine)
				continue;

			if (lightAngleFromDirectionCosine <= light.startCutofAttenuationCosine) 
			{
				factor = (lightAngleFromDirectionCosine - light.cutoffCOsine) / (light.startCutofAttenuationCosine - light.cutoffCOsine);
				factor = min(factor, 1.0);
			}

		}

		// check shadow
		factor *= sampleShadowMap(light, u, pos, lightDirection, surfaceLightAngleCosine);
		if (factor == 0)
			continue;

		// Diffuse lights
		c += objcolor * (u->lights[i].kD *  surfaceLightAngleCosine * factor);

		// Calculate the direction of the directly reflected light
		Vector3 relfectedLightDirection = (normal * (surfaceLightAngleCosine * 2) - lightDirection);

		// calculate the cosine of the angle of reflected light direction and viewer
		double reflectedangleToCameraCosine = -(relfectedLightDirection.dot(pos.returnNormal().xyz()));
		if (reflectedangleToCameraCosine <= 0)
			continue;

		// Specular lights
		c += u->lights[i].kS  * (pow(reflectedangleToCameraCosine,u->shineness) * factor);
	}

	return c.clamp();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::createLighSourcesModels()
{
	BOUNDING_BOX box;
	box.point1 = Vector3(-0.1,-0.1,-0.1);
	box.point2 = Vector3(0.1,0.1,0.1);

	_light_spot_model = WireFrameModel::createBoxModel(box,Color(0,1,0),1);
	_light_spot_model->addLine(Vector3(0,0,0.1), Vector3(0,0,1), 1);

	_light_dir_model = new WireFrameModel(8,4);
	_light_dir_model->addLine(Vector3(0,0,-2), Vector3(0,0,1),4);
	_light_dir_model->addLine(Vector3(0,1,-2), Vector3(0,0,1),4);
	_light_dir_model->addLine(Vector3(1,0,-2), Vector3(0,0,1),4);
	_light_dir_model->addLine(Vector3(1,1,-2), Vector3(0,0,1),4);

	_light_point_model = WireFrameModel::createBoxModel(box,Color(0,1,0),20);
	_light_point_model->addLine(Vector3(0.1,0.1,0.1), Vector3(1,1,1),0.3);
	_light_point_model->addLine(Vector3(0.1,0.1,-0.1), Vector3(1,1,-1),0.3);
	_light_point_model->addLine(Vector3(0.1,-0.1,0.1), Vector3(1,-1,1),0.3);
	_light_point_model->addLine(Vector3(0.1,-0.1,-0.1), Vector3(1,-1,-1),0.3);
	_light_point_model->addLine(Vector3(-0.1,0.1,0.1), Vector3(-1,1,1),0.3);
	_light_point_model->addLine(Vector3(-0.1,0.1,-0.1), Vector3(-1,1,-1),0.3);
	_light_point_model->addLine(Vector3(-0.1,-0.1,0.1), Vector3(-1,-1,1),0.3);
	_light_point_model->addLine(Vector3(-0.1,-0.1,-0.1), Vector3(-1,-1,-1),0.3);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Engine::drawLightSources()
{
	useSimpleShader(_renderer, &_shaderData);

	Mat4 view = _cameraTransform.getMatrix() * _projectionTransform.getMatrix();

	for (int i = 0 ; i < MAX_LIGHT ; i++) {

		LightParams &lp = _lightParams[i];
		if (!lp.enabled)
			continue;

		Mat4 objTransform = lp.space == LIGHT_SPACE_LOCAL ? _globalObjectTransform.getMatrix()  : Mat4::createUnit();

		double factor = 5 * _normalsScale / calculateInitialScaleFactor();

		switch(lp.type) {
		case LIGHT_TYPE_DIRECTIONAL:
			_shaderData.mat_objectToClipSpaceTransform = 
				Transformations::getScaleMatrix(Vector3(factor, factor, factor)) *
				Transformations::lookAt(Vector3(0,0,3 * factor), lp.direction, Vector3(1,1,1)) * objTransform *view;
			renderMiscModelWireframe(_light_dir_model, Color(0,1,0), true);
			break;

		case LIGHT_TYPE_POINT:
			_shaderData.mat_objectToClipSpaceTransform = 
				Transformations::getScaleMatrix(Vector3(factor, factor, factor)) *
				Transformations::getTranlationMatrix(lp.position) *
				objTransform *_cameraTransform.getMatrix() * _projectionTransform.getMatrix();
			renderMiscModelPolygonWireframe(_light_point_model, Color(0,1,0), true);
			break;
		case LIGHT_TYPE_SPOT:
			_shaderData.mat_objectToClipSpaceTransform = 
				Transformations::getScaleMatrix(Vector3(factor, factor, factor)) *
				Transformations::lookAt(lp.position, lp.direction, Vector3(1,1,1)) * objTransform *view;
			renderMiscModelPolygonWireframe(_light_spot_model, Color(0,1,0), true);
			break;
		}
	}
}
