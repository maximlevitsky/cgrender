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
#include "common/Math.h"
#include "common/Iterators.h"

class Texture;
class DepthTexture;
class IntegerTexture;

#define MAX_ATTRIBUTES 5


//////////////////////////////////////////////////////////////////////////////////////////////////////

/* An input to pixel shader*/

class Renderer;
struct PS_INPUTS {
	/* all attributes (flat/smooth/noperspective)*/
	Vector3 attributes[MAX_ATTRIBUTES];

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


struct TVertex
{
	int _ID;
	int _seq;

	/* location of the vertex in clip and screen spaces */
	Vector4 pos;
	Vector4 sp;

	/* attributes */
	Vector3 attr[MAX_ATTRIBUTES];

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

		// miss - can't not overwrite
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

class TriangleSetup
{
public:
	void setup(const TVertex* p1, const TVertex* p2, const TVertex* p3);

	void setAttributes(int start, int mid, int end) {
		first_attr = start;
		first_no_persp = mid;
		last_attr = end;
	}
public:
	// steps for all attributes
	Vector3 dax[MAX_ATTRIBUTES];
	Vector3 day[MAX_ATTRIBUTES];

	double d_inv_wx; double d_inv_wy;
	double dzx; double dzy;

	int first_attr;
	int first_no_persp;
	int last_attr;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////

class PixelState
{
public:
	void start(const TriangleSetup &s, const TVertex* p1, const int x_start, const int y_start);
	void stepX(const TriangleSetup &s);
	void stepYX(const TriangleSetup &s, const int x_steps);
	void setupPSInputs(const TriangleSetup &s, PS_INPUTS &ps);

public:
	double z;
	double inv_w;
	Vector3 attrbs[MAX_ATTRIBUTES];
};


//////////////////////////////////////////////////////////////////////////////////////////////////////

class Renderer
{
public:

	enum RENDER_MODE
	{
		WIREFRAME = 1,
		WIREFRAME_COLOR = 2,
		SOLID = 4,
	};

	typedef void (*vertexShader) (void* priv, void *in, Vector4 &out_position, Vector3 out_attributes[]);
	typedef Color (*pixelShader) (void* priv, const PS_INPUTS &in);

	// set output buffers
	void setOutputTexture(Texture *t);
	void setZBuffer(DepthTexture *z);

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
	void setBackFaceCulling(bool enable) { _backFaceCulling = enable; }
	void setFrontFaceCulling(bool enable) { _frontFaceCulling = enable; }
	void setWireframeColor(Color c) { _wireframeColor = c; }

	// rendering
	void renderBackgroundColor(Color background);
	void renderBackground(const Texture &texture, double scaleX, double scaleY);

	void uploadVertices(void* vertices, int vertexSize, int count);
	void renderPolygons(unsigned int* geometry, int count, enum RENDER_MODE mode);

	// used for pixel shaders
	void queryLOD(int attributeIndex, double &x_step, double &y_step) const;

	Renderer();
private:

	// rasterizers helpers
	PS_INPUTS _psInputs;

	TriangleSetup _setup;

	// output buffer
	Texture* _outputTexture;
	DepthTexture *_zBuffer;

	int _viewportSizeX;
	int _viewportSizeY;
	double _aspectRatio;

	// vertex and pixel shaders
	vertexShader _vertexShader;
	pixelShader _pixelShader;
	void* _vsPriv;
	void* _psPriv;

	// vertex and pixel shader options
	unsigned char _vFlatACount;
	unsigned char _vSmoothACount;
	unsigned char _vNoPersACount;


	// vertex buffer
	void* _vertexBuffer;
	int _vertexBufferStride;

	double clip_x;
	double clip_y;

	double scaleFactorX;
	double scaleFactorY;
	double moveFactorX;
	double moveFactorY;

	// settings
	bool _backFaceCulling;
	bool _frontFaceCulling;
	Color _wireframeColor;

	// matrices for output transform
	Mat4 mat_NDCtoDeviceTransform;
	Mat4 mat_DeviceToNDCTransform;

private:

	void drawTriangle(const TVertex* p1, const TVertex* p2, const TVertex* p3);
	void drawLine(TVertex *p1, TVertex *p2, const Color &c);
	void drawPixel(int x, int y, const Color &value);

	void updateViewportDimisions();
	Vector4 NDC_to_DeviceSpace(const Vector4* input);
	int clipAgainstPlane(VertexCache &cache, TVertex* input[], int point_count, TVertex* output[], Vector4 plane);
};



#endif
