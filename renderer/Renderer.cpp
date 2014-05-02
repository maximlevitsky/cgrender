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
	_viewportSizeX(0), _viewportSizeY(0), _aspectRatio(1.0),
	_outputTexture(NULL), _zBuffer(NULL),

	// shaders
	_vertexShader(NULL), _pixelShader(NULL),
	// settings
	_backFaceCulling(false), _frontFaceCulling(false),
	_wireframeColor(0,0,0)
{
	_psInputs._renderer = this;
	setVertexAttributes(0,0,0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// user calls this to setup dimisions of the rendered area
void Renderer::setViewport(int width, int height)
{
	_viewportSizeX = width;
	_viewportSizeY = height;
	updateViewportDimisions();
}

void Renderer::setAspectRatio( double ratio )
{
	assert(ratio >= 0);
	_aspectRatio = ratio;
	updateViewportDimisions();
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

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::setVertexAttributes( 
	unsigned char flatCount, unsigned char smoothCount, unsigned char noPerspectiveCount )
{
	_vFlatACount = flatCount;
	_vSmoothACount = smoothCount;
	_vNoPersACount = noPerspectiveCount;
	_setup.setAttributes(_vFlatACount, _vFlatACount+_vSmoothACount, _vFlatACount + _vSmoothACount + _vNoPersACount);
}


void Renderer::uploadVertices(void* vertices, int vertexSize, int count)
{
	_vertexBuffer = vertices;
	_vertexBufferStride = vertexSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::renderBackgroundColor(Color background) 
{
	for (int row = 0 ; row < _viewportSizeY ; row++)
		for(int column = 0 ; column < _viewportSizeX ; column++)
			drawPixel(column, row, background);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


void Renderer::renderBackground( const Texture &texture, double scaleX, double scaleY )
{
	TextureSampler s;
	s.bindTexture(&texture);
	s.setScale(scaleX,scaleY);

	for (int y=0; y<_viewportSizeY ; y++)
		for (int x=0; x<_viewportSizeX ; x++)
			drawPixel(x, y, s.sampleBiLinear((double)x/_viewportSizeX, (double)y/_viewportSizeY));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::renderPolygons( unsigned int* geometry, int count, enum Renderer::RENDER_MODE mode)
{
	VertexCache cache;
	TVertex* vt[128];

	for (polygonIterator iter(geometry, count); iter.hasmore() ; iter.next())
	{
		cache.newPolygon();
		int vtCount = iter.vertexCount();

		int clipx = 0,clipy = 0;
		bool clip = false;

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
					vt[i]->sp = NDC_to_DeviceSpace(&pos);
			}

			/* check clipping conditions */
			if (std::abs(pos.x()) > pos.w() * clip_x) {
				clipx += pos.x() > 0;
				clip  = true;
			}


			if (std::abs(pos.y()) > pos.w() * clip_y) {
				clipy  += pos.y() > 0;
				clip = true;
			}
		}

		vt[vtCount] = vt[0];

		/* clipping */
		if (clip)
		{
			/* trivial reject - all vertices are out on same side */
			if (abs(clipx) == vtCount || abs(clipy) == vtCount)
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
			z += (vt[i]->sp.x() - vt[i+1]->sp.x()) * (vt[i]->sp.y()+vt[i+1]->sp.y());

		_psInputs.frontface = z < 0;

		if ((!_psInputs.frontface && _backFaceCulling) || (_psInputs.frontface && _frontFaceCulling))
			continue;

		/* setup flat attributes*/
		for (int i = 0 ; i < _vFlatACount ; i++)
			_psInputs.attributes[i] = vt[0]->attr[i];

		/* and now render the polygon by turning them to triangles*/

		if (mode & Renderer::SOLID)
			for (int i = 1 ; i < vtCount - 1 ; i++)
				drawTriangle(vt[0], vt[i], vt[i+1]);

		/* and render the wireframe */
		if (mode & Renderer::WIREFRAME)
		{
			Color c =  (mode & WIREFRAME_COLOR) ? _wireframeColor : vt[0]->attr[0];
			for (int i = 0 ; i < vtCount ; i++)
				drawLine(vt[i], vt[i+1], c);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

int Renderer::clipAgainstPlane(VertexCache &cache, TVertex* input[], int in_count, TVertex* output[], Vector4 plane)
{
	int out_count = 0;
	int attrCount = _vFlatACount+ _vSmoothACount+_vNoPersACount;

	for (int i = 0 ; i < in_count ; i++)
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
			TVertex* newVertex = cache.allocateTemp();

			if (in1 == false) {
				swap(p1,p2);
				swap(dot1,dot2);
			}

			double t = dot1 / (dot1 - dot2);

			newVertex->pos = p1->pos + (p2->pos - p1->pos) * t;
			newVertex->sp = NDC_to_DeviceSpace(&newVertex->pos);

			for (int j = _vFlatACount ; j < attrCount ;j++)
				newVertex->attr[j] = p1->attr[j] + (p2->attr[j] - p1->attr[j]) * t;

			output[out_count++] = newVertex;
		}
	}

	if (out_count) {
		for (int j =  0; j < _vFlatACount ;j++)
			output[0]->attr[j] = input[0]->attr[j];
		output[out_count] = output[0];
	}

	return out_count;
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
	double w = 1.0 / input->w();
	return Vector4(
			input->x() * scaleFactorX * w + moveFactorX,
			input->y() * scaleFactorY * w + moveFactorY,
			input->z() * 0.5 * w + 0.5,
			w
	);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::updateViewportDimisions() 
{
	double viewportAspectRatio = (double)_viewportSizeX / _viewportSizeY;

	if (_aspectRatio > viewportAspectRatio) {
		// image is wider that screen
		scaleFactorX = _viewportSizeX / 2;
		scaleFactorY = scaleFactorX / _aspectRatio;
	} else {
		scaleFactorY = _viewportSizeY / 2;
		scaleFactorX = scaleFactorY * _aspectRatio;
	}

	clip_x = ((double)_viewportSizeX-0.5) / (2 * scaleFactorX);
	clip_y = ((double)_viewportSizeY-0.5) / (2 * scaleFactorY);

	scaleFactorY = - scaleFactorY;
	moveFactorX = _viewportSizeX / 2;
	moveFactorY = _viewportSizeY / 2;

	/* update the matrices we give users to do the transforms we do ourselves manually*/
	mat_NDCtoDeviceTransform =
			Mat4::getScaleMatrix(Vector3(scaleFactorX, -scaleFactorY, 0.5)) *
			Mat4::getMoveMat(Vector3(_viewportSizeX / 2, _viewportSizeY / 2, 0.5));

	if (scaleFactorX >0 && scaleFactorY > 0)
		mat_DeviceToNDCTransform = mat_NDCtoDeviceTransform.inv();

}


void Renderer::queryLOD( int attributeIndex, double &x_step, double &y_step ) const
{
	/*TODO*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////////


