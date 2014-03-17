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
#include "model/Model.h"
#include <cmath>

/*************************************************************************************************************/
void phongVertexShader( void* priv, void* in, Vector4 &pos_out, Vector3 attribs_out[] )
{
	const UniformBuffer *u = (const UniformBuffer*)priv;
	const Model::Vertex& v = *(const Model::Vertex*)in;

	attribs_out[0] = vmul3point(v.position,u->mat_objectToCameraSpace);
	attribs_out[1] = vmul3dir(v.normal,u->mat_objectToCameraSpaceNormalTransform).returnNormal();

	if (u->textureSampler.isBound())
		attribs_out[2] = v.texCoord;

	pos_out = vmul4point(v.position,u->mat_objectToClipSpaceTransform);
}

/********************************************************************************************************/
Color phongPixelShader( void* priv, const PS_INPUTS &in)
{
	const UniformBuffer *u = (const UniformBuffer*)priv;

	const Vector3 &position = in.attributes[0];
	Vector3 normal = in.attributes[1].returnNormal();
	Color c;

	if (!u->textureSampler.isBound())
		 c = u->objectColor;
	else if (u->sampleMode == TMS_NEARST)
			c = u->textureSampler.sample(in.attributes[2][0], in.attributes[2][1]);
	else if (u->sampleMode == TMS_BILINEAR)
		c = u->textureSampler.sampleBiLinear(in.attributes[2][0], in.attributes[2][1]);
	else {
		double lodX, lodY;
		in._renderer->queryLOD(2, lodX, lodY);
		c = u->textureSampler.sampleBiLinearMipmapped(in.attributes[2][0], in.attributes[2][1], lodX, lodY);
	}

	bool frontFace = u->forceFrontFaces ? true : (in.frontface ^ u->facesReversed);
	c = doLighting(u, c, position, normal, !frontFace );
	return !u->fogParams.enabled ? c : applyFog(u, in.d, c);
}

/********************************************************************************************************/
Color doLighting( const UniformBuffer* u, Color &objcolor, const Vector3& pos, Vector3 &normal, bool backface )
{
	Color c = u->kA * objcolor;

	// dealing with backfaces...
	if (backface && !u->lightBackfaces)
		return c.clamp();

	if (backface)
		normal = -normal;

	for (int i = 0 ; i < u->lightsCount ; i++)
	{
		const ShaderLightData &light = u->lights[i];

		// Calculate the direction the light is coming from (from point toward the light source)
		const Vector3 &lightDirection = light.is_point ? (light.location - pos).returnNormal() : light.direction;

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

		if (light._shadowCubemapSampler.isBound() || light._shadowMapSampler.isBound())
		{
			// check shadow
			factor *= sampleShadowMap(light, u, pos, lightDirection, surfaceLightAngleCosine);
			if (factor == 0)
				continue;
		}

		// Diffuse lights
		c += objcolor * (u->lights[i].kD *  surfaceLightAngleCosine * factor);

		// Calculate the direction of the directly reflected light
		Vector3 relfectedLightDirection = (normal * (surfaceLightAngleCosine * 2) - lightDirection);

		// calculate the cosine of the angle of reflected light direction and viewer
		double reflectedangleToCameraCosine = -(relfectedLightDirection.dot(pos.returnNormal()));
		if (reflectedangleToCameraCosine <= 0)
			continue;

		// Specular lights
		double tmp = pow(reflectedangleToCameraCosine,u->shineness) * factor;
		c += u->lights[i].kS  * tmp;
	}

	return c.clamp();
}

/********************************************************************************************************/
double sampleShadowMap( const ShaderLightData &light, const UniformBuffer *u, const Vector3 &pos,
	const Vector3 &dir, double surfaceAngeleCosine )
{

	surfaceAngeleCosine = clamp(surfaceAngeleCosine, 0.0, 1.0);
	double bias = clamp(u->shadowParams.z_bias_mul * tan (acos(surfaceAngeleCosine)), 0.0, u->shadowParams.z_bias_max);

	if (light._shadowMapSampler.isBound())
	{
		Vector4 trans_pos = vmul4point(pos,(light.shadowMapTransfrom[0]));
		trans_pos.canonicalize();

		const ShadowSampler &sampler = light._shadowMapSampler;

		if (u->shadowParams.poison && u->shadowParams.pcf)
			return sampler.samplePoisonPCF(trans_pos.x(), trans_pos.y(), trans_pos.z()-bias, u->shadowParams.pcf_taps);
		else if (u->shadowParams.pcf)
			return sampler.samplePCF(trans_pos.x(), trans_pos.y(), trans_pos.z()-bias, u->shadowParams.pcf_taps);
		else if (u->shadowParams.poison)
			return sampler.samplePoison(trans_pos.x(), trans_pos.y(), trans_pos.z()-bias);
		else
			return sampler.sample(trans_pos.x(), trans_pos.y(), trans_pos.z()-bias);

	} else if (light._shadowCubemapSampler.isBound())
	{
		int face = light._shadowCubemapSampler.selectFace(vmul3dir(dir,u->mat_cameraToWorldSpace));
		Vector4 trans_pos = vmul4point(pos,(light.shadowMapTransfrom[face]));
		trans_pos.canonicalize();

		const ShadowCubemapSampler &sampler = light._shadowCubemapSampler;

		if (u->shadowParams.poison && u->shadowParams.pcf)
			return sampler.samplePoisonPCF(face, trans_pos.x(), trans_pos.y(), trans_pos.z()-bias, u->shadowParams.pcf_taps);
		else if (u->shadowParams.pcf)
			return sampler.samplePCF(face, trans_pos.x(), trans_pos.y(), trans_pos.z()-bias, u->shadowParams.pcf_taps);
		else if (u->shadowParams.poison)
			return sampler.samplePoison(face, trans_pos.x(), trans_pos.y(), trans_pos.z()-bias);
		else
			return sampler.sample(face, trans_pos.x(), trans_pos.y(), trans_pos.z()-bias);
	} else
		return 1;
}

/*************************************************************************************************************/
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
