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

//////////////////////////////////////////////////////////////////////////////////////////////////////
// draw line between points

void Renderer::drawLine( TVertex *p1, TVertex *p2, const Color &c )
{
	int x1 = (int)(p1->sp.x()), x2 = (int)(p2->sp.x());
	int y1 = (int)(p1->sp.y()), y2 = (int)(p2->sp.y());

	// add small bias to Z so that wireframe is rendered above the model
	double z1 = p1->sp.z() - 0.05, z2 = p2->sp.z() - 0.05;

    int dx = (int)abs(x2 - x1);
	int dy = (int)abs(y2 - y1);
	double dz = (z2 - z1) / max(dx,dy);

	int sx = x1 < x2 ? 1 : -1;
	int sy = y1 < y2 ? 1 : -1;
	int d = dx - dy;

	while (1)
	{
		if (!_zBuffer || _zBuffer->zTest(x1,y1, z1))
			drawPixel(x1, y1, c);

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

class TriangleSetup
{
public:
	TriangleSetup(const TVertex* p1, const TVertex* p2, const TVertex* p3, int start, int end)
	{
		// general triangle setup
		double dx1 = (p1->sp.x() - p2->sp.x());
		double dx2 = (p3->sp.x() - p1->sp.x());
		double dy1 = (p1->sp.y() - p2->sp.y());
		double dy2 = (p3->sp.y() - p1->sp.y());

		double ooa  = 1.0 / (dx1 * dy2 - dy1 * dx2);
		double dy1_ooa  = dy1 * ooa, dy2_ooa  = dy2 * ooa;
		double dx1_ooa  = dx1 * ooa, dx2_ooa  = dx2 * ooa;

		// z and w setup
		double dw1 = (p1->sp.w() - p2->sp.w());
		double dw2 = (p3->sp.w() - p1->sp.w());
		dwx = dw1 * dy2_ooa - dw2 * dy1_ooa;
		dwy = dw2 * dx1_ooa - dw1 * dx2_ooa;

		double dz1 = (p1->sp.z() - p2->sp.z());
		double dz2 = (p3->sp.z() - p1->sp.z());
		dzx = dz1 * dy2_ooa - dz2 * dy1_ooa;
		dzy = dz2 * dx1_ooa - dz1 * dx2_ooa;

		for (int i = start ; i < end ; i++)
		{
			Vector3 da1 = (p1->attr[i] * p1->sp.w() - p2->attr[i] * p2->sp.w());
			Vector3 da2 = (p3->attr[i] * p3->sp.w() - p1->attr[i] * p1->sp.w());
			dax[i] = da1 * dy2_ooa - da2 * dy1_ooa;
			day[i] = da2 * dx1_ooa - da1 * dx2_ooa;
		}
	}
public:

	// steps for all attributes
	Vector3 dax[MAX_ATTRIBUTES];
	Vector3 day[MAX_ATTRIBUTES];

	double dwx; double dwy;
	double dzx; double dzy;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////

class PixelState
{
public:

	PixelState(TriangleSetup &s, const TVertex* p1, int start, int end)
	{
		double y_delta = ceil(p1->sp.y()) - p1->sp.y();
		z = p1->sp.z() + s.dzy * y_delta;
		w = p1->sp.w() + s.dwy * y_delta;

		for (int i = start ; i < end ; i++)
			attrbs[i] = p1->attr[i] * p1->sp.w() + s.day[i] * y_delta;
	}


	void stepX(TriangleSetup &s, int start, int end)
	{
		z += s.dzx;
		w += s.dwx;

		for (int i = start ; i < end ; i++)
			attrbs[i] += s.dax[i];
	}

	void stepYX(TriangleSetup &s, int x_steps, int start, int end)
	{
		z += s.dzy + s.dzx * x_steps;
		w += s.dwy + s.dwx * x_steps;

		for (int i = start ; i < end ; i++) {
			attrbs[i] += s.day[i];
			attrbs[i] += s.dax[i] * x_steps;
		}
	}

	void setupPSInputs(PS_INPUTS &ps, int start, int end)
	{
		for (int i = start ; i < end ; i++)
			ps.attributes[i] = attrbs[i] / w;
	}

public:
	double z;
	double w;
	Vector3 attrbs[MAX_ATTRIBUTES];
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// draw triangle between points

double make_step(const TVertex* p1, const TVertex* p2)
{
	const double y1 = p1->sp.y(); const double y2 = p2->sp.y();
	const double dy = y2 - y1;

	double x1 = p1->sp.x();
	double x2 = p2->sp.x();

	return dy ? ((x2 - x1)/dy) : 0;
}

void Renderer::drawTriangle(const TVertex* p1, const TVertex* p2, const TVertex* p3)
{
	// sort the points from bottom to top by Y (do the simple bubble sort)
	if (p2->sp.y() > p3->sp.y())
		std::swap(p3, p2);
	if (p1->sp.y() > p2->sp.y())
		std::swap(p2, p1);
	if (p2->sp.y() > p3->sp.y())
		std::swap(p3, p2);

	TriangleSetup setup(p1,p2,p3, _vFlatACount, _vSmoothACount);

	int y_start = ceil(p1->sp.y());
	int y_middle = ceil(p2->sp.y());
	int y_end = floor(p3->sp.y());

	double x1 = p1->sp.x();
	double x2 = p1->sp.x();

	double x1_step = make_step(p1,p3);
	double x2_step = make_step(p1,p2);

	int x_first = ceil(x1), x_last = floor(x1);

	PixelState state(setup, p1, _vFlatACount, _vSmoothACount);

	for (_psInputs.y = y_start ; _psInputs.y < y_end ; _psInputs.y++)
	{
		PixelState stateRow(state);
		int x_first_old = x_first;

		/* rasterize the horizontal line now */
		for (_psInputs.x = x_first ; _psInputs.x < x_last ; _psInputs.x++)
		{

			if (_psInputs.x <0 || _psInputs.x >= _viewportSizeX)
				continue;

			if (_psInputs.y <0 || _psInputs.y >= _viewportSizeY)
				continue;


			/* do the (early Z test)*/
			if (_zBuffer && !_zBuffer->zTest(_psInputs.x,_psInputs.y, stateRow.z))
				continue;

			/* run pixel shader if we have output buffer */
			if (_outputTexture)
			{
				_psInputs.d = stateRow.z;
				stateRow.setupPSInputs(_psInputs,  _vFlatACount, _vSmoothACount);
				drawPixel(_psInputs.x, _psInputs.y, _pixelShader(_psPriv, _psInputs));
			}

			stateRow.stepX(setup, _vFlatACount, _vSmoothACount);

		}

		x1 += x1_step; x2 += x2_step;

		if (_psInputs.y == y_middle)
		{
			x2 = p2->sp.x();
			x2_step = make_step(p2,p3);
		}

		x_first = ceil(min(x1,x2));
		x_last = floor(max(x1,x2));

		state.stepYX(setup, x_first - x_first_old, _vFlatACount, _vSmoothACount);
	}
}
