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
#ifndef ENGINE_H
#define ENGINE_H

#include "common/Transformations.h"
#include "common/Mat4.h"
#include "common/Vector4.h"
#include "common/BBox.h"

#include "renderer/Renderer.h"
#include "renderer/Texture.h"
#include "model/Model.h"

#include "Shaders.h"



#define _USE_MATH_DEFINES
#include <math.h>
#include <memory>

enum LightID {
	LIGHT_ID_AMBIENT=-1,LIGHT_ID_1=0, LIGHT_ID_2,
	LIGHT_ID_3, LIGHT_ID_4, LIGHT_ID_5, LIGHT_ID_6,
	LIGHT_ID_7, LIGHT_ID_8, MAX_LIGHT
};

enum LightType {
	LIGHT_TYPE_DIRECTIONAL,
	LIGHT_TYPE_POINT,
	LIGHT_TYPE_SPOT
};

enum LightSpace
{
	LIGHT_SPACE_VIEW,
	LIGHT_SPACE_LOCAL
};


struct LightParams
{
	LightParams() {reset();}

	//light enabled
	bool enabled;
	//type directional,point,spot
	LightType type;
	//local or view space
	LightSpace space;

	//color 0-255 RGB
	Color color;

	int cutoffAngle;


	Vector3 position;
	Vector3 direction;
	bool shadow;

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
	}
};

enum FogType {
	FOG_LINEAR,
	FOG_EXPONETIAL,
	FOG_EXPONETIAL2,

};

struct FogParams 
{
	bool enabled;
	enum FogType type;

	Color color; /* fog color */

	double startPoint;
	double endPoint;
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


struct SceneItem 
{
	SceneItem() : _mainModel(NULL), _boxModel(NULL), 
		_vertexNormalModel(NULL), _polygonNormalModel(NULL), texture(NULL)
	{}

	~SceneItem() 
	{
		if (_mainModel) delete _mainModel;
		if (_boxModel) delete _boxModel;
		if (_vertexNormalModel) delete _vertexNormalModel;
		if (_polygonNormalModel) delete _polygonNormalModel;
		if (texture) Texture::unloadCached(texture);
	}

	// position and BBOX
	Vector3 _position;
	BOUNDING_BOX _modelBox;
	WireFrameModel* _boxModel;

	// object geometry
	Model* _mainModel;

	// Aux geometry
	WireFrameModel* _vertexNormalModel;
	WireFrameModel* _polygonNormalModel;

	// Model transformation and material
	Transformations::AffineTransformation _modelTransform;
	MaterialParams _material;

	const Texture *texture;
	int texScaleX;
	int texScaleY;
};

class Engine
{
public:

	// initialization
	Engine(void);

	void createLighSourcesModels();

	~Engine(void);

	void setRenderer(Renderer *renderer);
	void setOutput(Texture* output, int width, int height);
	enum SHADING_MODE {SHADING_NONE, SHADING_FLAT, SHADING_GOURAD, SHADING_PHONG};

	// model loading
	bool loadSceneFromOBJ(const char* file);
	void loadDebugScene();
	void resetScene();


	// resetting of settings
	void resetTransformations();
	void resetLighting();
	void resetMaterials();

	// transformations
	void rotateObject(int axis, double angleDelta);
	void moveObject( int axis, double delta);
	void scaleObject(int axis, double delta);
	void rotateCamera(int axis, double angleDelta);
	void moveCamera(int axis, double delta);

	// perspective settings
	void setOrtographicRendering() {_projectionTransform.setPerspectiveEnabled(false);}
	void setPerspectiveRendering() {_projectionTransform.setPerspectiveEnabled(true);}
	bool isPerspectiveRendering() const { return _projectionTransform.getPerspectiveEnabled(); }

	void setPerspectiveD(double d);
	double getPerspectiveD() const  { return _projectionTransform.getDistance(); }


	// normals
	void setInvertNormals(bool enable);
	bool getInvertNormals() { return _invertNormals; }
	void setInvertFaces(bool enable);
	bool getInvertFaces() { return _invertFaces; }

	// lightning tweaks
	void setLightAllFaces(bool enable) { _lightAllFaces = enable;}
	bool getLightAllFaces() { return _lightAllFaces; }
	void setLightBackFaces(bool enable) { _lightBackfaces = enable; }
	bool getLightBackFaces() { return _lightBackfaces; }


	// shading mode
	enum SHADING_MODE getShadingMode()  { return _shadingMode; };
	void setShadingMode(enum SHADING_MODE mode) { _shadingMode = mode; };


	// global and local settings 
	LightParams* getLightParams(int id) { return (id == -1) ? &_ambientLight : &_lightParams[id];}
	MaterialParams& getMatrialParams();
	void reloadTextures();

	// object selection helpers
	Vector3 deviceToNDC(double X, double Y, double Z);
	bool selectObject(int mouseCX, int mouseCY);

	void setDrawSeperateObjects(bool enable);
	bool getDrawSeparateObjects() { return _drawSeparateObjects; }

	// misc settings
	void setDrawBoundingBox(bool enable) { _drawBoundingBox = enable; }
	bool getDrawBoundingBox() const  { return _drawBoundingBox;}

	void setDrawVertexNormals(bool enable) { _drawVertexNormals = enable; }
	bool getdrawVertexNormals() const  { return _drawVertexNormals;}

	void setDrawPolygonNormals(bool enable) { _drawPolygonNormals = enable; }
	bool getdrawPolygonNormals() const  { return _drawPolygonNormals;}

	void setDrawWireFrame(bool enable)  {_drawWireFrame = enable; }
	bool getdrawWireFrame() const { return _drawWireFrame || _shadingMode == SHADING_NONE;}

	void setDrawAxes(bool enable) { _drawAxes = enable;}
	bool getDrawAxes() const { return _drawAxes; }

	bool getBackfaceCulling() { return _backfaceCulling;}
	void setBackFaceCulling(bool enable) { _backfaceCulling = enable; }

	bool getDepthRendering() { return _depthRendering; }
	void setDepthRendering(bool enable) { _depthRendering = enable; }

	bool getPerspectiveCorrect() { return _perspectiveCorrect; }
	void setPerspectiveCorrect(bool enable) { _perspectiveCorrect = enable; }

	void setInvertDepth(bool enable) { _cameraTransform.setInvert(enable); }
	bool getInvertDepth() { return _cameraTransform.getInvert();}

	void setDrawLightSources(bool enable) { _draw_light_sources = enable; }
	bool getDrawLightSources() { return _draw_light_sources; }

	double getNormalScale() { return _normalsScale; }
	void setNormalScale(double newscale)
	{
		invalidateNormalModels();
		_normalsScale = newscale;
	}

	TextureSampleMode getTextureSampleMode() { return _texSampleMode; }
	void setTextureSampleMode(TextureSampleMode mode) { _texSampleMode = mode; }
	void resetTextureMode() { _texSampleMode = TMS_BILINEAR_MIPMAPS; }
	
	// background
	bool loadBackgroundImage(const char* file);
	void resetBackground();
	bool getTileBackground() { return _tile_background; }
	void setTileBackground(bool enable);
	void setBackgroundColor(Color c) {_backGroundColor = c;}
	Color getBackgroundColor() const { return _backGroundColor; }

	// fog
	FogParams getFogParams();
	void setFogParams(const FogParams &params);
	void resetFog();

	// rendering
	void render();

	// shadow maps
	void invalidateShadowMaps();
	void updateShadowMaps();
	void freeShadowMaps();
	const DepthTexture* getShadowMap(int i)  { return _shadowMaps[i]; }
	ShadowParams getShadowParams() { return _shadowParams; }
	void setShadowParams(ShadowParams * params);

private:
	// all the objects to render and their properties
	SceneItem *_sceneItems;
	unsigned int _itemCount;
	BOUNDING_BOX _sceneBox;
	BOUNDING_BOX _initialsceneBox;

	WireFrameModel* _sceneBoxModel;
	WireFrameModel* _axesModel;
	WireFrameModel* _light_spot_model;
	WireFrameModel* _light_dir_model;
	WireFrameModel* _light_point_model;

	// camera properties
	Transformations::CameraTransformation _cameraTransform;
	Transformations::ProjectionTransformation _projectionTransform;

	FogParams _fogParams;

	// global object settings
	Transformations::AffineTransformation _globalObjectTransform;
	MaterialParams _globalObjectMaterial;

	TextureSampleMode _texSampleMode;

	// Lighting
	LightParams _lightParams[8];
	LightParams _ambientLight;

	// settings
	SHADING_MODE _shadingMode;
	bool _drawWireFrame;
	bool _drawVertexNormals;
	bool _drawPolygonNormals;
	bool _drawBoundingBox;
	bool _drawSeparateObjects;
	bool _drawAxes;
	bool _backfaceCulling;
	bool _lightBackfaces;
	bool _draw_light_sources;

	// debug settings
	bool _lightAllFaces;
	bool _depthRendering;
	bool _perspectiveCorrect;
	bool _invertNormals;
	bool _invertFaces;
	double _normalsScale;

	// shader data and its setup
	UniformBuffer _shaderData;
	void setupTransformationShaderData(int objectID);
	void setupLightingShaderData(int objectID);
	void setupFogShaderData();
	void setupMaterialsShaderData(int objectID);
	void setupShadowMapShaderData(int objectID);

	// background
	const Texture* _backgroundTexture;
	Color _backGroundColor;
	bool _tile_background;

	int _selectedObject;

	/* output buffers */
	int _outputSizeX;
	int _outputSizeY;
	Texture* _outputTexture;
	DepthTexture* _outputZBuffer;
	IntegerTexture* _outputSelBuffer;
	Renderer *_renderer;

	// shadow data
	DepthTexture* _shadowMaps[MAX_LIGHT*6];
	Mat4 _shadowMapsMatrices[MAX_LIGHT*6];
	bool _shadowMapsValid;
	ShadowParams _shadowParams;

private:
	Transformations::AffineTransformation *getTransformationSettings();
	double calculateInitialScaleFactor() const;
	void processScene();
	void createNormalModels();
	void invalidateNormalModels();
	void recomputeBoundingBox();
	void recomputeDepth();
	void updateOutputs();

	void createShadowMap(int i, const Vector3 &direction, const Vector3 &position, bool projective, double maxFov);
	void renderBackground();

	void renderMiscModelWireframe(const WireFrameModel *m, Color c = Color(0,0,0), bool colorValid = false);
	void renderMiscModelPolygonWireframe(const WireFrameModel* m, Color c = Color(0,0,0), bool colorValid = false);

	void drawLightSources();

	enum FACE_TYPE {
		FACE_FRONT,
		FACE_BACK,
	};

	FACE_TYPE translateFaceType(FACE_TYPE given);
};

#endif
