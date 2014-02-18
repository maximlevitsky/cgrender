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

////////////////////////////////////////////////////////////////////////////////////////////////////////

static void flatVertexShader( void* priv, void* in, Vector4 &pos_out, Vector3 attribs_out[] )
{
	const UniformBuffer *u = (const UniformBuffer*)priv;
	const Model::Vertex& v = *(const Model::Vertex*)in;

	if (v.polygon) {

		const Vector4& position = (toHomoCoords(v.polygon->polygonCenter) * u->mat_objectToCameraSpace);
		Vector3 normal = (v.polygon->polygonNormal * u->mat_objectToCameraSpaceNormalTransform).returnNormal();

		Color c;
		if (!u->textureSampler.isBound())
			c = u->objectColor;
		else if (u->sampleMode == TMS_NEARST)
			c = u->textureSampler.sample(v.texCoord[0], v.texCoord[1]);
		else
			c = u->textureSampler.sampleBiLinear(v.texCoord[0], v.texCoord[1]);

		attribs_out[0] = doLighting(u, c, position, normal, false);
		attribs_out[1] = doLighting(u, c, position, normal, true);
	}

	pos_out = toHomoCoords(v.position) * u->mat_objectToClipSpaceTransform;
}

static Color flatPixelShader( void* priv, const PS_INPUTS &in)
{
	const UniformBuffer *u = (const UniformBuffer*)priv;
	bool frontFace = u->forceFrontFaces ? true : in.frontface ^ u->facesReversed;

	if (u->fogParams.enabled)
		return applyFog(u, in.d, frontFace  ? in.attributes[0] : in.attributes[1]);
	return frontFace ? in.attributes[0] : in.attributes[1];
}


void useFlatShader(Renderer *render, UniformBuffer *u) 
{
	render->setVertexAttributes(2, 0, 0);
	render->setVertexShader(flatVertexShader, u);
	render->setPixelShader(flatPixelShader, u);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

static void gouraldVertexShader( void* priv, void* in, Vector4 &pos_out, Vector3 attribs_out[] )
{
	const UniformBuffer *u = (const UniformBuffer*)priv;
	const Model::Vertex& v = *(const Model::Vertex*)in;

	const Vector4 &position = (toHomoCoords(v.position) * u->mat_objectToCameraSpace);
	Vector3 normal = (v.normal * u->mat_objectToCameraSpaceNormalTransform).returnNormal();

	Color c;
	if (!u->textureSampler.isBound())
		c = u->objectColor;
	else if (u->sampleMode == TMS_NEARST)
		c = u->textureSampler.sample(v.texCoord[0], v.texCoord[1]);
	else
		c = u->textureSampler.sampleBiLinear(v.texCoord[0], v.texCoord[1]);

	attribs_out[0] = doLighting(u, c, position, normal, false);
	attribs_out[1] = doLighting(u, c, position, normal, true);
	pos_out = toHomoCoords(v.position) * u->mat_objectToClipSpaceTransform;

}

static Color gouraldPixelShader( void* priv, const PS_INPUTS &in)
{
	const UniformBuffer *u = (const UniformBuffer*)priv;
	bool frontFace = u->forceFrontFaces ? true : in.frontface ^ u->facesReversed;

	Color c = frontFace ? in.attributes[0] : in.attributes[1];
	return !u->fogParams.enabled ? c : applyFog(u, in.d, c);
}


void useGouraldShader(Renderer *render, UniformBuffer *u, bool perspectiveCorrect) 
{
	if (perspectiveCorrect)
		render->setVertexAttributes(0, 2, 0);
	else
		render->setVertexAttributes(0, 0, 2);

	render->setVertexShader(gouraldVertexShader, u);
	render->setPixelShader(gouraldPixelShader, u);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

static void phongVertexShader( void* priv, void* in, Vector4 &pos_out, Vector3 attribs_out[] )
{
	const UniformBuffer *u = (const UniformBuffer*)priv;
	const Model::Vertex& v = *(const Model::Vertex*)in;

	attribs_out[0] = (toHomoCoords(v.position) * u->mat_objectToCameraSpace).xyz();
	attribs_out[1] = (v.normal * u->mat_objectToCameraSpaceNormalTransform).returnNormal();

	if (u->textureSampler.isBound())
		attribs_out[2] = v.texCoord;

	pos_out = toHomoCoords(v.position) * u->mat_objectToClipSpaceTransform;
}

static Color phongPixelShader( void* priv, const PS_INPUTS &in)
{
	const UniformBuffer *u = (const UniformBuffer*)priv;

	const Vector4 &position = toHomoCoords(in.attributes[0]);
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

	bool frontFace = u->forceFrontFaces ? true : in.frontface ^ u->facesReversed;
	c = doLighting(u, c, position, normal, !frontFace );
	return !u->fogParams.enabled ? c : applyFog(u, in.d, c);
}

void usePhongShader(Renderer *render, UniformBuffer *u, bool perspectiveCorrect)
{
	int attribCount = 2;

	if (u->textureSampler.isBound())
		attribCount++;

	if (perspectiveCorrect)
		render->setVertexAttributes(0, attribCount, 0);
	else
		render->setVertexAttributes(0, 0, attribCount);

	render->setVertexShader(phongVertexShader, u);
	render->setPixelShader(phongPixelShader, u);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////

static void simpleVertexShader( void* priv, void* in, Vector4 &pos_out, Vector3 attribs_out[] )
{
	const UniformBuffer *u = (const UniformBuffer*)priv;
	const Model::Vertex& v = *(const Model::Vertex*)in;
	pos_out = toHomoCoords(v.position) * u->mat_objectToClipSpaceTransform;
}

static Color simplePixelShader( void* priv, const PS_INPUTS &in) 
{
	return Color(0.5,0.5,0.5);
}


void useSimpleShader(Renderer *render, UniformBuffer *u)
{
	render->setVertexAttributes(0, 0, 0);
	render->setVertexShader(simpleVertexShader, u);
	render->setPixelShader(simplePixelShader, u);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


static void simpleWireFrameShader( void* priv, void* in, Vector4 &pos_out, Vector3 attribs_out[] )
{
	const UniformBuffer *u = (const UniformBuffer*)priv;
	const WireFrameModel::Vertex& v = *(const WireFrameModel::Vertex*)in;
	pos_out = v.position * u->mat_objectToClipSpaceTransform;
	attribs_out[0] = v.c;
}

void useSimpleWireframeShader(Renderer *render, UniformBuffer *u)
{
	render->setVertexAttributes(1, 0, 0);
	render->setVertexShader(simpleWireFrameShader, u);
	render->setPixelShader(simplePixelShader, u);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void Engine::render()
{
	if (_shadingMode != SHADING_NONE)
		updateShadowMaps();

	updateOutputs();

	// clear Z buffer
	_outputZBuffer->clear();

	renderBackground();

	if(!_itemCount) return;

	// global settings
	_renderer->setAspectRatio(_initialsceneBox.getSizes().x() / _initialsceneBox.getSizes().y() );
	_renderer->setDebugDepthRendering(_depthRendering);

	createNormalModels();
	setupFogShaderData();

	for (unsigned int i = 0 ; i < _itemCount; i++)
	{
		SceneItem &item = _sceneItems[i];
		const Model &m = *item._mainModel;

		/* setup shader uniforms*/
		setupTransformationShaderData(i);
		setupMaterialsShaderData(i);
		setupLightingShaderData(i);
		setupShadowMapShaderData(i);

		/* setup shaders */
		switch(_shadingMode) 
		{
		case SHADING_GOURAD:
			useGouraldShader(_renderer, &_shaderData, _perspectiveCorrect);
			break;
		case SHADING_PHONG:
			usePhongShader(_renderer, &_shaderData, _perspectiveCorrect);
			break;
		case SHADING_FLAT:
			useFlatShader(_renderer, &_shaderData);
			break;
		case SHADING_NONE:
			useSimpleShader(_renderer, &_shaderData);
			break;
		}

		// setup culling
		if (_backfaceCulling) 
		{
			FACE_TYPE culledFace = translateFaceType(FACE_BACK);
			_renderer->setBackFaceCulling(culledFace == FACE_BACK);
			_renderer->setFrontFaceCulling(culledFace == FACE_FRONT);
		} else {
			_renderer->setBackFaceCulling(false);
			_renderer->setFrontFaceCulling(false);
		}

		// upload the model vertices
		_renderer->uploadVertices(m.vertices, sizeof(Model::Vertex), m.getNumberOfVertices());

		// render the model (or just Z + selection buffer for separate object wireframe mode
		if (_shadingMode != SHADING_NONE || _drawSeparateObjects) 
		{
			_renderer->setOutputTexture(_shadingMode != SHADING_NONE ? _outputTexture : NULL);
			_renderer->renderPolygons(m.polygons, m.getNumberOfPolygons(), i+1);
		}

		_renderer->setOutputTexture(_outputTexture );

		// draw black wireframe over model when we asked to draw a wireframe in addition to shading
		if (_drawWireFrame && _shadingMode != SHADING_NONE)
			_renderer->renderWireFrame(m.polygons, m.getNumberOfPolygons(), Color(0,0,0), true);

		// draw wireframe in object color when we don't draw anything else
		if (_shadingMode == SHADING_NONE) {
			_renderer->renderWireFrame(m.polygons, m.getNumberOfPolygons(), Color(0,0,0), false);
		}

		// Now render the misc stuff
		_renderer->setBackFaceCulling(false);
		_renderer->setFrontFaceCulling(false);

		// draw per object bounding box and axes
		if (_drawSeparateObjects) 
		{
			if (_selectedObject >= 0 && i == (unsigned int)_selectedObject)
			{
				// for selected object always draw axes and bounding box in red
				renderMiscModelWireframe(item._boxModel, Color(1,0,0), true);
				renderMiscModelWireframe(_axesModel, Color(1,0,0), true);
			} else 
			{
				// for other objects draw box if setting is up in green
				if (_drawBoundingBox)
					renderMiscModelWireframe(item._boxModel, Color(0,1,0), true);

				// and always draw axes in their color
				if (_drawAxes)
					renderMiscModelWireframe(_axesModel);
			}
		}

		if (_drawVertexNormals)
			renderMiscModelWireframe(item._vertexNormalModel);
		if (_drawPolygonNormals)
			renderMiscModelWireframe(item._polygonNormalModel);

	}

	// bounding box and axes of whole model
	setupTransformationShaderData(-1);

	if (_drawBoundingBox)
		renderMiscModelWireframe(_sceneBoxModel, Color(0,0,1), true);

	if (_drawAxes)
		renderMiscModelWireframe(_axesModel);


	// render light sources
	if (_draw_light_sources)
		drawLightSources();

}


void Engine::renderMiscModelWireframe( const WireFrameModel *m, Color c /*= Color(0,0,0)*/, bool colorValid /*= false*/ )
{
	if (!m)
		return;

	useSimpleWireframeShader(_renderer, &_shaderData);
	_renderer->uploadVertices(m->vertices, sizeof(WireFrameModel::Vertex), m->getNumberOfVertices());
	_renderer->renderWireFrame(m->polygons, m->getNumberOfPolygons(), c, colorValid);
}

void Engine::renderMiscModelPolygonWireframe(const WireFrameModel *m, Color c, bool colorValid) 
{
	if (!m)
		return;

	useSimpleWireframeShader(_renderer, &_shaderData);
	_renderer->uploadVertices(m->vertices, sizeof(WireFrameModel::Vertex), m->getNumberOfVertices());
	_renderer->renderWireFrame(m->polygons, m->getNumberOfPolygons(), c, colorValid);
	_renderer->renderPolygons(m->polygons, m->getNumberOfPolygons(),0);

}
