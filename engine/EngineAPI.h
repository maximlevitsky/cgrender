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
#ifndef ENGINE_API_H
#define ENGINE_API_H

#include <string>

//////////////////////////////////////////////////////////////////////////////////////////////
enum SHADING_MODE
{
	SHADING_NONE,
	SHADING_FLAT,
	SHADING_GOURAD,
	SHADING_PHONG
};

//////////////////////////////////////////////////////////////////////////////////////////////


enum TextureSampleMode
{
	TMS_NEARST,
	TMS_BILINEAR,
	TMS_BILINEAR_MIPMAPS
};

//////////////////////////////////////////////////////////////////////////////////////////////

struct LightSource
{
	//light enabled
	bool enabled;

	//type directional,point,spot
	enum LightType
	{
		LIGHT_TYPE_DIRECTIONAL,
		LIGHT_TYPE_POINT,
		LIGHT_TYPE_SPOT
	} type;


	//local or view space
	enum LightSpace
	{
		LIGHT_SPACE_VIEW,
		LIGHT_SPACE_LOCAL
	} space;


	//color 0-255 RGB
	Color color;

	// postion and direction
	Vector3 position;
	Vector3 direction;

	int cutoffAngle;
	bool shadow;

	bool debugDraw;

	////////////////////////////////////////////////////////

	void reset()
	{
		enabled = false;
		type =  LIGHT_TYPE_DIRECTIONAL;
		space = LIGHT_SPACE_VIEW;
		color = Color(255,255,255);
		position = Vector3(0,0,0);
		direction  = Vector3(0,0,0);

		cutoffAngle = 30;
		shadow = false;
		debugDraw = false;
	}

	LightSource()
	{
		reset();
	}
};

#define MAX_LIGHT 8

//////////////////////////////////////////////////////////////////////////////////////////////

struct FogParams
{
	bool enabled;

	enum FogType
	{
		FOG_LINEAR,
		FOG_EXPONETIAL,
		FOG_EXPONETIAL2,

	} type;

	/* fog color */
	Color color;

	/* start and end point for linear fog*/
	double startPoint;
	double endPoint;

	/* density for exponential fog*/
	double density;

	void reset()
	{
		enabled = false;
		type = FOG_LINEAR;
		startPoint = 0;
		endPoint = 1;
		density = 1;
	}

	FogParams() {
		reset();
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////

struct ShadowParams
{
	/* Z bias*/
	double z_bias_mul;
	double z_bias_max;

	/* soft shadow settings */
	bool pcf;
	bool poison;
	int pcf_taps;

	/* shadow map resolution */
	int shadowMapRes;

	void reset() {
		pcf = true;
		poison = true;
		pcf_taps = 4;
		z_bias_max = 0.01;
		z_bias_mul = 0.05;
		shadowMapRes = 1024;
	}

	ShadowParams() { reset(); }
};

//////////////////////////////////////////////////////////////////////////////////////////////

struct EngineOperationFlags
{
	/* here we put all engine tweak flags that don't deserve its own getter/setter*/
	bool drawBoundingBox;
	bool drawVertexNormals;
	bool drawFaces;
	bool drawWireFrame;
	bool drawAxes;
	bool backFaceCulling;
	bool leftcoordinateSystem;
	bool depthBufferVisualization;
	bool perspectiveCorrect;

	bool twofaceLighting;
	bool forceFrontFaces;
};

//////////////////////////////////////////////////////////////////////////////////////////////

struct BackgroundParams
{
	enum Mode {
		COLOR,
		TEXTURE
	} mode;

	enum ScalingMode
	{
		STRETCH,
		TILE,
	} textureScalingMode;

	std::string textureFile;
	Color color;
};

//////////////////////////////////////////////////////////////////////////////////////////////


enum ROTATION_MODE
{
	ROTATION_X = 1,
	ROTATION_Y = 2,
	ROTATION_Z = 4
};

#endif
