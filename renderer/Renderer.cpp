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
#include "Renderer.h"
#include "Texture.h"
#include "Samplers.h"

#include "common/Mat4.h"
#include "common/Vector4.h"
#include "common/Vector3.h"

#include <assert.h>


Renderer::Renderer(void) : 

	// output buffer
	_viewportSizeX(0), _viewportSizeY(0), _outputTexture(NULL), _zBuffer(NULL), _selBuffer(NULL),
	_aspectRatio(1.0), _objectID(0),
	
	// shaders
	_vertexShader(NULL), _pixelShader(NULL),
	// settings
	_debugDepthRendering(false),_backFaceCulling(false), _frontFaceCulling(false),
	_wireframeColor(0,0,0)
{
	_psInputs._renderer = this;
	setVertexAttributes(0,0,0);
}

Renderer::~Renderer(void)
{
	delete _zBuffer;
	delete _selBuffer;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// user calls this to setup dimisions of the rendered area
void Renderer::setViewport(int width, int height)
{
	_viewportSizeX = width;
	_viewportSizeY = height;
	updateNDCToDisplayTransform();
}

void Renderer::setAspectRatio( double ratio )
{
	assert(ratio >= 0);
	_aspectRatio = ratio;
	updateNDCToDisplayTransform();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// user calls this to set output buffers

void Renderer::setOutputTexture( Texture *t )
{
	assert (!t || (t->getWidth() >=  _viewportSizeX && t->getHeight() >= _viewportSizeY));
	_outputTexture = t;
}

void Renderer::setZBuffer(DepthTexture *z)
{
	assert (!z ||( z->getWidth() >=  _viewportSizeX && z->getHeight() >= _viewportSizeY));
	_zBuffer = z;
}
void Renderer::setSelBuffer(IntegerTexture *s)
{
	assert (!s || (s->getWidth() >=  _viewportSizeX && s->getHeight() >= _viewportSizeY));
	_selBuffer = s;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::setVertexAttributes( 
	unsigned char flatCount, unsigned char smoothCount, unsigned char noPerspectiveCount )
{
	_vertexFlatAttributeCount = flatCount;
	_vertexSmoothAttributeCount = smoothCount;
	_vertexNoPerspectiveCount = noPerspectiveCount;

	_line1.setAttributesCount(_vertexFlatAttributeCount, _vertexSmoothAttributeCount, _vertexNoPerspectiveCount);
	_line2.setAttributesCount(_vertexFlatAttributeCount, _vertexSmoothAttributeCount, _vertexNoPerspectiveCount);
	_line.setAttributesCount(_vertexFlatAttributeCount, _vertexSmoothAttributeCount, _vertexNoPerspectiveCount);
}


void Renderer::uploadVertices(void* vertices, int vertexSize, int count)
{
	_vertexBuffer = vertices;
	_vertexBufferStride = vertexSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


void Renderer::renderBackgroundColor(Color background) 
{
	for (int row = 0 ; row < _viewportSizeY ; row++) {
		for(int column = 0 ; column < _viewportSizeX ; column++) {

			drawPixel(column, row, background);
			if (_selBuffer) _selBuffer->setPixelValue(column,row, 0);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


void Renderer::renderBackground( const Texture &texture, double scaleX, double scaleY )
{

	TextureSampler s;
	s.bindTexture(&texture);
	s.setScale(scaleX,scaleY);

	for (int y=0; y<_viewportSizeY ; y++)
	{
		for (int x=0; x<_viewportSizeX ; x++)
		{
			drawPixel(x, y, s.sampleBiLinear((double)x/_viewportSizeX, (double)y/_viewportSizeY));
			if (_selBuffer) _selBuffer->setPixelValue(x,y, 0);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::renderPolygons( unsigned int* geometry, int count, int objectID ,enum Renderer::RENDER_MODE mode)
{
	VertexCache cache;
	TVertex* vt[128];


	if (objectID != -1 && _selBuffer) _objectID = objectID;

	for (polygonIterator iter(geometry, count); iter.hasmore() ; iter.next())
	{
		cache.newPolygon();
		int vtCount = iter.vertexCount();

		int leftside = 0,rightside = 0,topside = 0,bottomside = 0;
		bool needclip = false;

		/* first pass over vertices - run vertex shader, do perspective divide and test trivial clipping*/
		for (int i = 0 ; i < iter.vertexCount() ; i++)
		{
			bool valid;
			vt[i] = cache.get(iter[i], valid);
			const Vector4 &pos = vt[i]->pos;

			/* run vertex shader on all vertexes of current polygon, which are not in the cache */
			if (!valid)
			{
				_vertexShader(_vsPriv,(char*)_vertexBuffer + _vertexBufferStride * iter[i],
						vt[i]->pos,vt[i]->attr );

				/* do perspective divide - might be redundant if clipped later*/
				if (pos.w() > 0)
					vt[i]->posScr = NDC_to_DeviceSpace(&pos);
			}

			/* check clipping conditions */
			if (pos.x() < -pos.w() * clip_x) {
				leftside++; needclip = true;
			} else if (pos.x() > pos.w() * clip_x) {
				rightside++; needclip = true;
			}

			if (pos.y() < -pos.w() * clip_y) {
				bottomside++; needclip = true;
			} else if (pos.y() > pos.w() * clip_y) {
				topside++; needclip = true;
			}
		}

		vt[vtCount] = vt[0];

		/* clipping */
		if (needclip)
		{
			/* trivial reject - all vertices are out on same side */
			if (rightside == vtCount || bottomside == vtCount || leftside == vtCount || topside == vtCount)
				continue;

			/* otherwise do the clipping */
			TVertex* vt2[128];
			vtCount = clipAgainstPlane(cache, vt,  vtCount, vt2, Vector4(-1, 0, 0,clip_x));
			vtCount = clipAgainstPlane(cache, vt2, vtCount, vt,  Vector4( 0, 1, 0,clip_y));
			vtCount = clipAgainstPlane(cache, vt,  vtCount, vt2, Vector4( 1, 0, 0,clip_x));
			vtCount = clipAgainstPlane(cache, vt2, vtCount, vt,  Vector4( 0,-1, 0,clip_y));

			/* its still possible that we clipped everything */
			if (!vtCount) continue;
		}

		/* determine back-face and do face cull */
		double z = 0;

		for (int i = 0 ; i < vtCount ; i++)
			z += (vt[i]->posScr.x() - vt[i+1]->posScr.x()) * (vt[i]->posScr.y()+vt[i+1]->posScr.y());

		_psInputs.frontface = z < 0;

		if ((!_psInputs.frontface && _backFaceCulling) || (_psInputs.frontface && _frontFaceCulling))
			continue;

		/* setup flat attributes*/
		for (int i = 0 ; i < _vertexFlatAttributeCount ; i++)
			_psInputs.attributes[i] = vt[0]->attr[i];

		/* and now render the polygon by turning it to triangles*/

		if (mode & Renderer::SOLID)
			for (int i = 1 ; i < vtCount - 1 ; i++)
				drawTriangle(vt[0], vt[i], vt[i+1]);

		if (mode & Renderer::WIREFRAME)
		{
			Color c =  (mode & WIREFRAME_COLOR) ? _wireframeColor : vt[0]->attr[0];
			for (int i = 0 ; i < vtCount ; i++)
				drawLine(vt[i], vt[i+1], c);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

int Renderer::clipAgainstPlane(VertexCache &cache, TVertex* input[], int point_count, TVertex* output[], Vector4 plane)
{
	int out_count = 0;
	int attrCount = _vertexFlatAttributeCount+ _vertexSmoothAttributeCount+_vertexNoPerspectiveCount;

	for (int i = 0 ; i < point_count ; i++)
	{
		// find the edge to clip
		TVertex* p1 = input[i], *p2 = input[i+1];

		double dot1 =  p1->pos.dot(plane);
		double dot2 =  p2->pos.dot(plane);

		bool in1 = dot1 > 0.0;
		bool in2 = dot2 > 0.0;

		if (in1) output[out_count++] = p1;

		if (in1 != in2)
		{
			TVertex* nv = cache.allocateTemp();

			if (in1 == false) {
				swap(p1,p2);
				swap(dot1,dot2);
			}

			double t = dot1 / (dot1 - dot2);

			nv->pos = p1->pos + (p2->pos - p1->pos) * t;
			nv->posScr = NDC_to_DeviceSpace(&nv->pos);

			for (int j = _vertexFlatAttributeCount ; j < attrCount ;j++)
				nv->attr[j] = p1->attr[j] + (p2->attr[j] - p1->attr[j]) * t;

			output[out_count++] = nv;
		}
	}

	if (out_count) {
		for (int j =  0; j < _vertexFlatAttributeCount ;j++)
			output[0]->attr[j] = input[0]->attr[j];

		output[out_count] = output[0];
	}

	return out_count;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::shadePixel()
{
	/* do the (early Z test)*/
	if (_zBuffer) {
		if (!_zBuffer->zTest(_psInputs.x,_psInputs.y, _psInputs.d))
			return;
		_zBuffer->setPixelValue(_psInputs.x,_psInputs.y,_psInputs.d);
	}

	/* update the selection buffer */
	if (_selBuffer)
		_selBuffer->setPixelValue(_psInputs.x,_psInputs.y, _objectID);

	if (!_outputTexture)
		return;

	// debug depth print
	if (_debugDepthRendering && _zBuffer)
	{
		drawPixel(_psInputs.x, _psInputs.y, _zBuffer->debugGetPixel(_psInputs.x,_psInputs.y));
		return;
	}

	/* run pixel shader if we have output buffer */
	_line.setupPSInputs(_psInputs);
	drawPixel(_psInputs.x, _psInputs.y, _pixelShader(_psPriv, _psInputs));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::drawPixel( int x, int y, const Color &value )
{
	_outputTexture->setPixelValue(x,y,
		DEVICE_PIXEL((uint8_t)(value[0]*255), (uint8_t)(value[1]*255), (uint8_t)(value[2]*255)));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

Vector4 Renderer::NDC_to_DeviceSpace( const Vector4* input )
{
	Vector4 output = *input * mat_NDCtoDeviceTransform;

	double w = output.w();
	output.x() /= w;
	output.y() /= w;
	output.z() /= w;
	output.w() =  1.0 / w;
	return output;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::updateNDCToDisplayTransform() 
{
	double viewportAspectRatio = (double)_viewportSizeX / _viewportSizeY;
	double scaleFactorX, scaleFactorY;

	if (_aspectRatio > viewportAspectRatio) {
		// image is wider that screen
		scaleFactorX = _viewportSizeX / 2;
		scaleFactorY = scaleFactorX / _aspectRatio;
	} else {
		scaleFactorY = _viewportSizeY / 2;
		scaleFactorX = scaleFactorY * _aspectRatio;
	}

	mat_NDCtoDeviceTransform =
			Mat4::getScaleMatrix(Vector3(scaleFactorX, -scaleFactorY, 0.5)) *
			Mat4::getMoveMat(Vector3(_viewportSizeX / 2, _viewportSizeY / 2, 0.5));

	if (scaleFactorX >0 && scaleFactorY > 0)
		mat_DeviceToNDCTransform = mat_NDCtoDeviceTransform.inv();


	clip_x = ((double)_viewportSizeX-0.5) / (2 * scaleFactorX);
	clip_y = ((double)_viewportSizeY-0.5) / (2 * scaleFactorY);
}


void Renderer::queryLOD( int attributeIndex, double &x_step, double &y_step ) const
{
	// this is called from pixel shader to get approximate LOD based on an attribute deltas
	// TODO: for now we assume 2d texture coordinate
	assert(attributeIndex > _vertexFlatAttributeCount);
	attributeIndex -= _vertexFlatAttributeCount;

	Vector3 horStep = _line.attribute_steps[attributeIndex];

	if (attributeIndex < _vertexSmoothAttributeCount) 
	{
		x_step = horStep.x() / _line.w1;
		y_step = horStep.y() / _line.w1;
	} else {
		x_step = horStep.x();
		y_step = horStep.y();

	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////


