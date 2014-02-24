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

#include "common/Transformations.h"
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
	_debugDepthRendering(false),_backFaceCulling(false), _frontFaceCulling(false)
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

	_line1.setAttributesCount(_vertexSmoothAttributeCount, _vertexNoPerspectiveCount);
	_line2.setAttributesCount(_vertexSmoothAttributeCount, _vertexNoPerspectiveCount);
	_line.setAttributesCount(_vertexFlatAttributeCount, _vertexSmoothAttributeCount, _vertexNoPerspectiveCount);
	_vertexBuffer.setAttribCount(_vertexFlatAttributeCount+_vertexSmoothAttributeCount+noPerspectiveCount);
}


void Renderer::fillBackground(Color background) 
{
	for (int row = 0 ; row < _viewportSizeY ; row++) {
		for(int column = 0 ; column < _viewportSizeX ; column++) {

			drawPixel(column, row, background);
			if (_selBuffer) _selBuffer->setPixelValue(column,row, 0);
		}
	}
}

void Renderer::fillBackgroundTexture( const Texture &texture, double scaleX, double scaleY )
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

void Renderer::uploadVertices(void* vertices, int vertexSize, int count)
{
	assert(_vertexShader);

	try {
		_vertexBuffer.setVertexCount(count);
	} catch (...) {
		return;
	}

	unsigned char* vertices_data = (unsigned char*)vertices;

	for (int i = 0; i < count ; i++, vertices_data += vertexSize) 
	{
		// run vertex shader to get us to clip space
		Vector4 & pos = _vertexBuffer.position(i);
		_vertexShader(_vsPriv, vertices_data, pos, _vertexBuffer[i]);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// wireframe rendering

void Renderer::renderWireFrame( unsigned int* geometry, int count, Color c, bool colorValid )
{
	if (!_vertexBuffer.bufferSize)
		return;
	
	for (polygonIterator iter(geometry, count); iter.hasmore() ; iter.next())
	{
		if ((_backFaceCulling || _frontFaceCulling) && iter.vertexCount() >= 3) 
		{
			bool backface = determineBackface(iter);
			if ((backface && _backFaceCulling) || (!backface && _frontFaceCulling))
				continue;
		}

		for (int j=0, k=1; j<iter.vertexCount(); ++j, ++k)  
		{
			int indexOfVertex1 = iter[j];
			int indexOfVertex2 = k == iter.vertexCount() ? iter[0] : iter[k];

			const Vector4 &v1 = _vertexBuffer.position(indexOfVertex1);
			const Vector4 &v2 = _vertexBuffer.position(indexOfVertex2);

			if (v1.w() > 0 && v2.w() > 0 && !fastClipLine(v1, v2))
				drawLine(NDC_to_DeviceSpace(&v1), NDC_to_DeviceSpace(&v2),
						colorValid ? c : _vertexBuffer[indexOfVertex1][0]);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::renderPolygons( unsigned int* geometry, int count, int objectID )
{
	if (!_vertexBuffer.bufferSize)
		return;

	if (objectID != -1 && _selBuffer) _objectID = objectID;

	for (polygonIterator iter(geometry, count); iter.hasmore() ; iter.next()) 
	{
		_psInputs.frontface = !determineBackface(iter);

		if ((!_psInputs.frontface && _backFaceCulling) || (_psInputs.frontface && _frontFaceCulling))
			continue;

		/* setup flat attributes*/
		for (int i = 0 ; i < _vertexFlatAttributeCount ; i++)
			_psInputs.attributes[i] = _vertexBuffer[iter[0]][i];

		const Vector4 &v1 = _vertexBuffer.position(iter[0]);
		const Vector3 *a1 = _vertexBuffer[iter[0]]+_vertexFlatAttributeCount;

		/* and now render the polygon by turning it to triangles*/
		for (int i = 1 ; i < iter.vertexCount() - 1 ; i++) 
		{
			const Vector4 &v2 = _vertexBuffer.position(iter[i]);
			const Vector4 &v3 = _vertexBuffer.position(iter[i+1]);

			Vector3 *a2 = _vertexBuffer[iter[i]]+_vertexFlatAttributeCount;
			Vector3 *a3 = _vertexBuffer[iter[i+1]]+_vertexFlatAttributeCount;

			if (v1.w() > 0 && v2.w() > 0 && v3.w() > 0)
			{
				if (!fastClipTriangle(v1,v2, v3)) {
					Vector4 p1 = NDC_to_DeviceSpace(&v1);
					Vector4 p2 = NDC_to_DeviceSpace(&v2);
					Vector4 p3 = NDC_to_DeviceSpace(&v3);

					drawTriangle(&p1,&p2,&p3, a1,a2,a3);
				}
			}
			else
				/* otherwise we need clipping */
				drawClippedTriangle(&v1, &v2,&v3,a1,a2,a3);
		}
	}
}

// drawing primitives
//////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * draw triangle between points
 * all points are arrays of vertex shader outputs, where first item is the position
 */

void Renderer::drawTriangle(const Vector4* p1, const Vector4* p2,
		const Vector4* p3, const Vector3* a1, const Vector3* a2,
		const Vector3* a3) {
	int two_edges_side = 1; // determines which side consists of two edges
	// sort the points from left to right by Y (do the simple bubble sort)
	if (p2->y() > p3->y()) {
		swap(p3, p2);
		swap(a3, a2);
	}
	if (p1->y() > p2->y()) {
		swap(p2, p1);
		swap(a2, a1);
	}
	if (p2->y() > p3->y()) {
		swap(p3, p2);
		swap(a3, a2);
	}

	// find the order of two lines
	double p1p3_dx = p1->x() - p3->x();
	double p1p3_dy = p1->y() - p3->y();
	if (p1p3_dy
			&& p2->x() > p1->x() + ((p2->y() - p1->y()) * p1p3_dx) / p1p3_dy) {
		swap(p3, p2);
		swap(a3, a2);
		two_edges_side = 2;
	}

	// setup side line rasterizers
	_line1.setup(a1, a2, *p1, *p2);
	_line2.setup(a1, a3, *p1, *p3);

	// now do the rasterization....
	for (int stage = 0; stage < 2; stage++)
	{
		/* loop that runs from bottom up on both lines */
		while ((!_line1.ended())
				&& (!_line2.ended() && _line1.y1_int < _viewportSizeY)) {
			_psInputs.y = _line1.y1_int;
			/* rasterize the horizontal line now */
			for (_line.setup(_line1, _line2);
					!_line.ended() && _line.x1_int < _viewportSizeX;
					_line.stepX()) {
				_psInputs.x = _line.x1_int;
				_psInputs.d = _line.z1;
				shadePixel();
			}
			/* step two lines */
			_line1.stepY();
			_line2.stepY();
		}

		/* switch bottom to top trapezoid */
		if (stage == 0) {
			two_edges_side == 1 ?
					_line1.setup(a2, a3, *p2, *p3) :
					_line2.setup(a3, a2, *p3, *p2);
			if (_line1.y1_int > _line2.y1_int)
				_line2.stepY();

			if (_line1.y1_int < _line2.y1_int)
				_line1.stepY();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// draw a line. Used only to render wireframes

void Renderer::drawLine( Vector4 p1, Vector4 p2, const Color &c )
{
	if (p1.isBadFP() || p2.isBadFP())
		return;

	int x1 = (int)p1.x(), x2 = (int)p2.x();
	int y1 = (int)p1.y(), y2 = (int)p2.y();

	// add small bias to Z so that wireframe is rendered above the model
	double z1 = p1.z() - 0.05, z2 = p2.z() - 0.05;

    int dx = (int)abs(x2 - x1);
	int dy = (int)abs(y2 - y1);
	double dz = (z2 - z1) / max(dx,dy);

	int sx = x1 < x2 ? 1 : -1;
	int sy = y1 < y2 ? 1 : -1;
	int d = dx - dy;

	while (1) 
	{
		if (x1 >= 0 && x1 < _viewportSizeX && y1 >= 0 && y1 < _viewportSizeY
			&& (!_zBuffer || _zBuffer->zTest(x1,y1, z1))) 
		{
			if (_zBuffer) _zBuffer->setPixelValue(x1,y1,z1);
			drawPixel(x1, y1, c);
		}

		if (x1 == x2 && y1 == y2)
			break;

		z1 += dz;
		int e2 = 2 * d;

		if (e2 > -dy) {
			d -= dy;
			x1 += sx;
		}

		if (x1 == x2 && y1 == y2)
			continue;

		if (e2 < dx) {
			d += dx;
			y1 += sy;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////////////////////////

void Renderer::drawPixel( int x, int y, const Color &value )
{
	if (x < 0 || y < 0 || x >= _viewportSizeX || y >= _viewportSizeY)
		return;

	_outputTexture->setPixelValue(x,y, 
		DEVICE_PIXEL((uint8_t)(value[0]*255), (uint8_t)(value[1]*255), (uint8_t)(value[2]*255)));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

bool Renderer::determineBackface( polygonIterator iter )
{
	double z = 0;


	for (int i = 0 ; i < iter.vertexCount() ; i++) 
	{
		const Vector4 &v1 = NDC_to_DeviceSpace(&_vertexBuffer.position(iter[i]));
		const Vector4 &v2 = (i == iter.vertexCount() - 1) ?  
			NDC_to_DeviceSpace(&_vertexBuffer.position(iter[0])) : NDC_to_DeviceSpace(&_vertexBuffer.position(iter[i+1]));

		z += (v1.x() - v2.x()) * (v1.y()+v2.y());
	}

	return z > 0;
}

Vector4 Renderer::NDC_to_DeviceSpace( const Vector4* input )
{
	Vector4 output = *input * mat_NDCtoDeviceTransform;

	double w = output.w();
	output.x() /= w;
	output.y() /= w;
	output.z() /= w;
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
		Transformations::getScaleMatrix(Vector3(scaleFactorX, -scaleFactorY, 0.5)) *
		Transformations::getTranlationMatrix(Vector3(_viewportSizeX / 2, _viewportSizeY / 2, 0.5));

	if (scaleFactorX >0 && scaleFactorY > 0)
		mat_DeviceToNDCTransform = mat_NDCtoDeviceTransform.inverse();


	clip_x = (double)_viewportSizeX / (2 * scaleFactorX);
	clip_y = (double)_viewportSizeY / (2 * scaleFactorY);
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

void VerticalLineRasterizer::setAttributesCount(unsigned char smoothcount, unsigned char noPerspectiveCount )
{
	assert (smoothcount + noPerspectiveCount <= 8);
	smoothAttribCount = smoothcount;
	noPerspectiveAttribCount = noPerspectiveCount;
	smoothAndNoPerspectiveCount = (unsigned char)(smoothcount + noPerspectiveCount);
}

void VerticalLineRasterizer::setup( const Vector3 *attr1, const Vector3 *attr2, const Vector4& p1, const Vector4 &p2 )
{
	// calculate initial values
	double y1 = p1.y(); double y2 = p2.y();
	double dy = y2 - y1;

	x1 = p1.x(); double x2 = p2.x();
	z1 = p1.z(); double z2 = p2.z();
	w1 = 1.0 / p1.w(); double w2 = 1.0 / p2.w();

	if (dy) 
	{
		x_step = (x2 - x1)/dy; 
		z_step = (z2 - z1)/dy;
		w_step = (w2 - w1)/dy;
	}

	//setup attributes
	int i = 0;
	for (; i < smoothAttribCount ; i++)
	{
		attribs[i] = attr1[i] * w1;
		if (dy) attrib_steps[i] = ( attr2[i] * w2 - attr1[i] *w1 ) / dy;
	}

	for (; i < smoothAndNoPerspectiveCount ; i++) 
	{
		attribs[i] = attr1[i];
		if (dy) attrib_steps[i] = (attr2[i] - attr1[i]) / dy;
	}

	// calculate steps, advance by Y, etc
	if (dy) 
	{
		double y1_fraction = ceil(y1) - y1;
		if (y1_fraction) 
		{
			x1 += (y1_fraction * (x2 - x1))/dy;
			z1 += (y1_fraction * (z2 - z1))/dy;
			w1 += (y1_fraction * (w2 - w1))/dy;

			for (int i = 0 ; i < smoothAndNoPerspectiveCount ; i++)
				attribs[i] += (attrib_steps[i] * y1_fraction);
		}
	}

	// calculates integer X,Y values we will go from to
	y1_int = (int)ceil(y1); y2_int = (int)floor(y2);

	// skip all way up if in negative
	if (y1_int < 0) 
	{
		int steps = min(0, y2_int+1) - y1_int;

		x1 += x_step * steps; 
		w1 += w_step * steps; 
		z1 += z_step * steps;

		for (int i = 0 ; i < smoothAndNoPerspectiveCount ; i++) 
			attribs[i] += attrib_steps[i] * steps;

		y1_int += steps;
	}
}

void VerticalLineRasterizer::stepY()
{
	// step X,Z exactly 
	x1 += x_step; 
	w1 += w_step; 
	z1 += z_step;

	y1_int++;

	for (int i = 0 ; i < smoothAndNoPerspectiveCount ; i++) 
		attribs[i] += attrib_steps[i];
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HorizintalLineRasterizer::setAttributesCount( 
	unsigned char flatCount, unsigned char smoothcount, unsigned char noPerspectiveCount )
{
	assert (flatCount + smoothcount + noPerspectiveCount <= 8);
	flatAttribCount = flatCount; 
	smoothAttribCount = smoothcount;
	noPerspectiveAttribCount = noPerspectiveCount;
}

void HorizintalLineRasterizer::setup( const VerticalLineRasterizer &line1, const VerticalLineRasterizer &line2 )
{
	double dx = line2.x1 - line1.x1;
	z1 = line1.z1;
	w1 = line1.w1;

	if (dx) 
	{
		z_step = (line2.z1 - line1.z1)/dx; 
		w_step = (line2.w1 - line1.w1)/dx;
	}

	for (int i = 0 ; i < smoothAttribCount + noPerspectiveAttribCount ; i++) 
	{
		attributes[i] = line1.attribs[i];
		if (dx) attribute_steps[i] = (line2.attribs[i] - line1.attribs[i])/dx;
	}

	// account for fractional X
	if (dx) 
	{
		double x1_frac = ceil(line1.x1)  - line1.x1;
		if (x1_frac) {

			z1 += (x1_frac * (line2.z1 - line1.z1))/dx;
			w1 += (x1_frac* (line2.w1 - line1.w1))/dx;

			for (int i = 0 ; i < smoothAttribCount + noPerspectiveAttribCount ; i++)
				attributes[i] += (attribute_steps[i] * x1_frac);
		}
	}

	x1_int = (int)ceil(line1.x1);
	x2_int = (int)floor(line2.x1);

	if (x1_int < 0 ) 
	{
		if (x2_int < 0) {
			x1_int = 0;
		} else 
		{
			int steps = min(0, x2_int+1) - x1_int;

			z1 += z_step * steps;
			w1 += w_step  * steps;
			x1_int += steps;

			for (int i = 0 ; i < smoothAttribCount + noPerspectiveAttribCount; i++)
				attributes[i] += (attribute_steps[i] * steps);
		}
	}
}

void HorizintalLineRasterizer::stepX()
{
	// step the attributes
	z1 += z_step;
	w1 += w_step;
	x1_int++;

	for (int i = 0 ; i < smoothAttribCount + noPerspectiveAttribCount; i++)
		attributes[i] += attribute_steps[i];
}

void HorizintalLineRasterizer::setupPSInputs(PS_INPUTS &inputs)
{
	int i = 0;

	Vector3 *attribs = inputs.attributes + flatAttribCount;

	for (; i < smoothAttribCount ; i++)
		attribs[i] = attributes[i] / w1;

	for (; i < smoothAttribCount+noPerspectiveAttribCount ; i++)
		attribs[i] = attributes[i];
}
