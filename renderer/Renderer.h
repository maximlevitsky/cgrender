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

#ifndef RENDER_H
#define RENDER_H

#include "common/Mat4.h"
#include "common/Vector4.h"
#include "common/Vector3.h"
#include "common/Utilities.h"
#include "common/Iterators.h"

class Texture;
class DepthTexture;
class IntegerTexture;

//////////////////////////////////////////////////////////////////////////////////////////////////////

/* An input to pixel shader*/

class Renderer;
struct PS_INPUTS {
	/* all attributes (flat/smooth/noperspective)*/
	Vector3 attributes[8];

	/* Pixel position*/
	int x;
	int y;

	/* Pixel depth*/
	double d;

	/* set if the polygon this pixel belongs is front face*/
	bool frontface;

	Renderer *_renderer;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////

struct DEVICE_PIXEL
{
	DEVICE_PIXEL(unsigned char red, unsigned char green, unsigned char blue) :
		Blue(blue), Green(green), Red(red)  {}

	DEVICE_PIXEL() {}

	unsigned char Blue;
	unsigned char Green;
	unsigned char Red;
	unsigned char Alpha;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
class VerticalLineRasterizer
{
public:

	/* Initial setup to do when attribute counts change*/
	void setAttributesCount(unsigned char smoothcount, unsigned char noPerspectiveCount);

	/* Setup the class for for interpolation of one line of which p1 is the low end and p2 the high one*/
	void setup(const Vector3 *attr1, const Vector3 *attr2, const Vector4& p1, const Vector4 &p2);

	/* step one pixel up */
	void stepY();

	/* check if we reached the last pixel of this line */
	bool ended() { return (y1_int > y2_int); }

	/* integer positions of current and last pixel y*/
	int y1_int; int y2_int;

	// position and steps 
	double x1; double x_step; 
	double z1; double z_step; 
	double w1; double w_step;

	// attributes
	Vector3 attribs[8];
	Vector3 attrib_steps[8];
private:
	/* attributes count*/
	unsigned char smoothAttribCount;
	unsigned char noPerspectiveAttribCount;
	unsigned char smoothAndNoPerspectiveCount;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////

class HorizintalLineRasterizer 
{
public:

	/* attributes count setup */
	void setAttributesCount(unsigned char flatCount, unsigned char smoothcount, unsigned char noPerspectiveCount);

	/* setup the class to interpolate horizontal line from current positions of two vertical lines
	   always from line1 to line2
	*/
	void setup(const VerticalLineRasterizer &line1, const VerticalLineRasterizer &line2);

	/* step one pixel to the right*/
	void stepX();

	/* put current state of the rasterizers to pixel shader inputs*/
	void setupPSInputs(PS_INPUTS &inputs);
public:

	// position and steps
	double z1; double z_step;
	double w1; double w_step;

	int x1_int; int x2_int;

	bool ended() { return x1_int > x2_int; }

	//attributes
	Vector3 attributes[8];
	Vector3 attribute_steps[8];
private:

	//settings
	unsigned char flatAttribCount;
	unsigned char smoothAttribCount;
	unsigned char noPerspectiveAttribCount;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////


struct TVertex
{
	int _ID;
	int _seq;

	/* location of the vertex in clip and screen spaces */
	Vector4 _posClipspace;
	Vector4 _positionScreenspace;

	/* attributes */
	Vector3 _attributes[8];

	TVertex() : _seq(0), _ID(-1) {};
};

//////////////////////////////////////////////////////////////////////////////////////////////////////


class VertexCache
{
public:

	VertexCache() : _seq(0), _nextFreeTVertex(_storage)
	{}

	void newPolygon()
	{
		_seq++;
		_nextFreeTVertex = _storage;
	}

	TVertex* get(int id, bool &valid)
	{
#if 0
		unsigned int set = id & 511;

		// cache hit
		if (_cache[set]._ID == id)
		{
			_cache[set]._seq = _seq;
			valid = true;
			return &_cache[set];
		}

		valid = false;

		// miss - can overwrite the entry
		if (_cache[set]._seq != _seq)
		{
			_cache[set]._seq = _seq;
			_cache[set]._ID = id;
			return &_cache[set];
		}
#endif
		// miss - can't not overwrite
		valid = false;
		return allocateTemp();
	}


	TVertex* allocateTemp()
	{
		return _nextFreeTVertex++;
	}


private:
	int _seq;
	TVertex* _nextFreeTVertex;
	TVertex _cache[512];
	TVertex _storage[128];
};

//////////////////////////////////////////////////////////////////////////////////////////////////////

class Renderer
{
public:

	typedef void (*vertexShader) (void* priv, void *in, Vector4 &out_position, Vector3 out_attributes[]);
	typedef Color (*pixelShader) (void* priv, const PS_INPUTS &in);

	// set output buffers
	void setOutputTexture(Texture *t);
	void setZBuffer(DepthTexture *z);
	void setSelBuffer(IntegerTexture *z);

	// set rendering window
	void setViewport(int width, int height);
	void setAspectRatio(double ratio);
	Mat4 getDeviceToScreenMatrix() { return mat_DeviceToNDCTransform; }
	Mat4 getNDCTODeviceMatrix() { return mat_NDCtoDeviceTransform; }

	// shaders
	void setVertexShader( vertexShader vs, void* priv ) { _vertexShader = vs; _vsPriv = priv;}
	void setPixelShader( pixelShader ps, void* priv ) {_pixelShader = ps;_psPriv = priv;}

	// shader attributes
	void setVertexAttributes(unsigned char flatCount, 
		unsigned char smoothCount, unsigned char noPerspectiveCount);

	// settings
	void setDebugDepthRendering(bool enable)  { _debugDepthRendering = enable; }
	void setBackFaceCulling(bool enable) { _backFaceCulling = enable; }
	void setFrontFaceCulling(bool enable) { _frontFaceCulling = enable; }

	// rendering
	void fillBackground(Color background);
	void fillBackgroundTexture(const Texture &texture, double scaleX, double scaleY);

	void uploadVertices(void* vertices, int vertexSize, int count);
	void renderWireFrame(unsigned int* geometry, int count, Color c, bool colorValid);
	void renderPolygons(unsigned int* geometry, int count, int objectID);

	// used for pixel shaders
	void queryLOD(int attributeIndex, double &x_step, double &y_step) const;

	Renderer();
	~Renderer(void);
private:

	// rasterizers helpers
	HorizintalLineRasterizer _line;
	VerticalLineRasterizer _line1;
	VerticalLineRasterizer _line2;
	PS_INPUTS _psInputs;

	// output buffer
	Texture* _outputTexture;
	DepthTexture *_zBuffer;
	IntegerTexture *_selBuffer;
	int _objectID;

	int _viewportSizeX;
	int _viewportSizeY;
	double _aspectRatio;

	// vertex and pixel shaders
	vertexShader _vertexShader;
	pixelShader _pixelShader;
	void* _vsPriv;
	void* _psPriv;

	// temp buffer for vertices transform
	void* _vertexBuffer;
	int _vertexBufferStride;

	// matrices for output transform
	Mat4 mat_NDCtoDeviceTransform;
	Mat4 mat_DeviceToNDCTransform;

	double clip_x;
	double clip_y;
	void updateNDCToDisplayTransform();

	// vertex and pixel shader options
	unsigned char _vertexFlatAttributeCount;
	unsigned char _vertexSmoothAttributeCount;
	unsigned char _vertexNoPerspectiveCount;

	// settings
	bool _backFaceCulling;
	bool _frontFaceCulling;
	bool _debugDepthRendering;
private:
	void drawTriangle(const Vector4* p1, const Vector4* p2, const Vector4* p3,
			const Vector3* a1, const Vector3* a2, const Vector3* a3);
	void drawClippedTriangle(const Vector4 *p1, const Vector4 *p2, const Vector4 *p3, const Vector3 *a1, const Vector3 *a2, const Vector3 *a3);
	void shadePixel();
	void drawLine(Vector4 p1, Vector4 p2, const Color &c);
	void drawPixel(int x, int y, const Color &value);

	Vector4 NDC_to_DeviceSpace(const Vector4* input);

	bool fastClipTriangle(const Vector4& v1, const Vector4 &v2, const Vector4 &v3);
	bool fastClipLine(const Vector4& v1, const Vector4 &v2);

	int clipAgainstPlane(VertexCache &cache, TVertex* input[], int point_count, TVertex* output[], Vector4 plane);
};



#endif
