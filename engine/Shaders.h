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
#ifndef SHADERS_H
#define SHADERS_H

#include "common/Mat4.h"
#include "common/Vector4.h"
#include "renderer/Texture.h"
#include "renderer/Samplers.h"
#include "model/Material.h"
#include "EngineAPI.h"

struct ShaderFogData
{
	Color color;
	bool enabled;

	// linear fog params
	bool linear;
	double start;
	double end;
	double scale;

	// exponential fog params
	double density;
	bool exp2;
};


struct ShaderLightData 
{
	bool is_point;

	/* specular and ambient contribution of the lights*/
	Color kD;
	Color kS;

	Vector3 location;
	Vector3 direction;

	bool is_spot;
	double cutoffCOsine;
	double startCutofAttenuationCosine;

	ShadowSampler _shadowMapSampler;
	ShadowCubemapSampler _shadowCubemapSampler;
	Mat4 shadowMapTransfrom[6];
};


struct UniformBuffer 
{
public:
	// transformations
	Mat4 mat_objectToCameraSpaceNormalTransform;
	Mat4 mat_objectToCameraSpace;
	Mat4 mat_objectToClipSpaceTransform;
	Mat4 mat_cameraToObjectSpace;
	Mat4 mat_cameraToWorldSpace;

	Mat4 projNormaltransform;
	// lighting
	Color kA;
	ShaderLightData lights[8];
	unsigned char lightsCount;
	int shineness;

	// texturing
	TextureSampler textureSampler;
	TextureSampleMode sampleMode;
	Color objectColor;

	bool lightBackfaces;
	bool facesReversed;
	bool forceFrontFaces;

	// fog - for now just pass through it
	ShaderFogData fogParams;

	struct ShadowParams shadowParams;
};



Color doLighting(const UniformBuffer* u, Color &c, const Vector4 & pos, Vector3 &normal, bool backface);
Color applyFog(const UniformBuffer* u, double depth, const Color &color);

double sampleShadowMap(const ShaderLightData &light, const UniformBuffer *u, const Vector4 &pos, const Vector3 &dir, double surfaceAngeleCosine) ;

void useGouraldShader(Renderer *render, UniformBuffer *u, bool perspectiveCorrect);
void usePhongShader(Renderer *render, UniformBuffer *u, bool perspectiveCorrect);
void useFlatShader(Renderer *render, UniformBuffer *u);
void useSimpleShader(Renderer *render, UniformBuffer *u);


void phongVertexShader( void* priv, void* in, Vector4 &pos_out, Vector3 attribs_out[] );
Color phongPixelShader( void* priv, const PS_INPUTS &in);

#endif
