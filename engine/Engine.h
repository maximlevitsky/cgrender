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

#include "common/Mat4.h"
#include "common/Vector4.h"
#include "common/BBox.h"
#include "renderer/Texture.h"
#include "model/Model.h"
#include "Shaders.h"
#include "Transformations.h"
#include "EngineAPI.h"

class Renderer;


struct SceneItem 
{
	SceneItem() :
		_mainModel(NULL),
		_boxModel(NULL),
		_vertexNormalModel(NULL),
		_polygonNormalModel(NULL),
		texture(NULL)
	{}

	~SceneItem() 
	{
		delete _mainModel;
		delete _boxModel;
		delete _vertexNormalModel;
		delete _polygonNormalModel;
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
	ObjectTransformation _itemTR;
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
	~Engine(void);

	void setRenderer(Renderer *renderer);
	void setOutput(Texture* output, int width, int height);

	// model loading
	bool loadSceneFromOBJ(const char* file);
	void loadDebugScene();
	void resetScene();
	std::string getLoadedModelFilename() const { return _currentModelFile; }


	// transformations
	void rotateObject(int axis, double angleDelta);
	void moveObject( int axis, double delta);
	void scaleObject(int axis, double delta);

	void commitRotation();
	void resetTransformations();
	void setRotationmode(ROTATION_MODE mode) { _rotMode = mode; }

	// camera settings
	void rotateCamera(int axis, double angleDelta);
	void moveCamera(int axis, double delta);
	void setOrtographicRendering() {_projTR.setPerspectiveEnabled(false);}
	void setPerspectiveRendering() {_projTR.setPerspectiveEnabled(true);}
	bool isPerspectiveRendering() const { return _projTR.getPerspectiveEnabled(); }
	void setPerspectiveD(double d);
	double getPerspectiveD() const  { return _projTR.getDistance(); }

	// shading mode
	enum SHADING_MODE getShadingMode()  { return _shadingMode; };
	void setShadingMode(enum SHADING_MODE mode) { _shadingMode = mode; };


	// lighting settings
	LightSource* getLightParams(int id) { return (id == -1) ? &_ambientLight : &_lightParams[id];}
	void invalidateShadowMaps();
	void resetLighting();

	// Shadow settings
	ShadowParams getShadowParams() { return _shadowParams; }
	void setShadowParams(ShadowParams * params);

	// Material settings (currently of selected object)
	MaterialParams& getMatrialParams();
	void resetMaterials();

	// background settings
	BackgroundParams getBackgroundSettings() const { return _backgroundSettings; }
	void setBackGroundSettings(const BackgroundParams newSettings);
	void resetBackground();

	// fog settings
	FogParams getFogParams();
	void setFogParams(const FogParams &params);
	void resetFog();

	// simple engine flags
	EngineOperationFlags getEngineOperationFlags() const { return _flags; }
	void setEngineOperationFlags(const EngineOperationFlags newFlags);

	// misc settings
	double getNormalScale() { return _normalsScale; }
	void setNormalScale(double newscale);
	TextureSampleMode getTextureSampleMode() { return _texSampleMode; }
	void setTextureSampleMode(TextureSampleMode mode) { _texSampleMode = mode; }
	void resetTextureSampleMode() { _texSampleMode = TMS_BILINEAR_MIPMAPS; }
	void setInvertNormals(bool enable);
	bool getInvertNormals() { return _invertNormals; }
	void setInvertFaces(bool enable);
	bool getInvertFaces() { return _invertFaces; }


	// object selection helpers
	Vector3 getSteps(double X, double Y);
	bool selectObject(int mouseCX, int mouseCY);
	void setDrawSeperateObjects(bool enable);
	bool getDrawSeparateObjects() { return _drawSeparateObjects; }


	// rendering
	void render();

	// debug access for shadow maps
	const DepthTexture* getShadowMap(int i)  { return _shadowMaps[i]; }

private:
	// all the objects to render and their properties
	std::string _currentModelFile;
	SceneItem *_sceneItems;
	unsigned int _itemCount;
	int _selObj;
	BOUNDING_BOX _sceneBox;
	BOUNDING_BOX _initialsceneBox;

	// global object settings
	ObjectTransformation _mainTR;
	MaterialParams _globalObjectMaterial;

	// camera properties
	CameraTransformation _cameraTR;
	ProjectionTransformation _projTR;

	// settings
	EngineOperationFlags _flags;
	FogParams _fogParams;
	ShadowParams _shadowParams;
	BackgroundParams _backgroundSettings;

	bool _drawSeparateObjects;
	bool _invertNormals;
	bool _invertFaces;
	double _normalsScale;
	TextureSampleMode _texSampleMode;
	SHADING_MODE _shadingMode;
	ROTATION_MODE _rotMode;


	// Lighting
	LightSource _lightParams[8];
	LightSource _ambientLight;

	// shader data and its setup
	UniformBuffer _shaderData;
	void setupTransformationShaderData(int objectID);
	void setupLightingShaderData(int objectID);
	void setupFogShaderData();
	void setupMaterialsShaderData(int objectID);
	void setupShadowMapShaderData(int objectID);


	/* output buffers */
	int _outputSizeX;
	int _outputSizeY;
	Texture* _outputTexture;
	DepthTexture* _outputZBuffer;
	IntegerTexture* _outputSelBuffer;
	Renderer *_renderer;

	// shadow maps
	DepthTexture* _shadowMaps[MAX_LIGHT*6];
	Mat4 _shadowMapsMatrices[MAX_LIGHT*6];
	bool _shadowMapsValid;

	// background texture
	const Texture* _backgroundTexture;

	/* misc models */
	WireFrameModel* _sceneBoxModel;
	WireFrameModel* _axesModel;
	WireFrameModel* _light_spot_model;
	WireFrameModel* _light_dir_model;
	WireFrameModel* _light_point_model;

	Vector3 rotCoofs;

private:
	double calculateInitialScaleFactor() const;
	void processScene();
	void createNormalModels();
	void invalidateNormalModels();
	void recomputeBoundingBox();
	void recomputeDepth();
	void updateOutputs();
	void reloadTextures();

	void createShadowMap(int i, const Vector3 &direction, const Vector3 &position, bool projective, double maxFov);
	void updateShadowMaps();
	void freeShadowMaps();

	void renderBackground();
	void renderMiscModelWireframe(const WireFrameModel *m, Color c = Color(0,0,0), bool colorValid = false);
	void renderMiscModelPolygonWireframe(const WireFrameModel* m, Color c = Color(0,0,0), bool colorValid = false);
	void renderLightSources();
	void createLighSourcesModels();
	bool rotationAxisEnabled(int axis);

	enum FACE_TYPE {
		FACE_FRONT,
		FACE_BACK,
	};

	FACE_TYPE translateFaceType(FACE_TYPE given);
};

#endif
