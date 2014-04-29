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

static double make_step(const TVertex* p1, const TVertex* p2)
{
	const double y1 = p1->sp.y(); const double y2 = p2->sp.y();
	const double dy = y2 - y1;
	double x1 = p1->sp.x();
	double x2 = p2->sp.x();
	return ((x2 - x1)/dy);
}

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

void TriangleSetup::setup(const TVertex* p1, const TVertex* p2, const TVertex* p3)
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

	for (int i = first_attr ; i < first_no_persp ; i++)
	{
		Vector3 da1 = (p1->attr[i] * p1->sp.w() - p2->attr[i] * p2->sp.w());
		Vector3 da2 = (p3->attr[i] * p3->sp.w() - p1->attr[i] * p1->sp.w());
		dax[i] = da1 * dy2_ooa - da2 * dy1_ooa;
		day[i] = da2 * dx1_ooa - da1 * dx2_ooa;
	}

	for (int i = first_no_persp ; i < last_attr ; i++)
	{
		Vector3 da1 = (p1->attr[i] - p2->attr[i]);
		Vector3 da2 = (p3->attr[i] - p1->attr[i]);
		dax[i] = da1 * dy2_ooa - da2 * dy1_ooa;
		day[i] = da2 * dx1_ooa - da1 * dx2_ooa;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void PixelState::start(const TriangleSetup &s, const TVertex* p1, const int x_start, const int y_start)
{
	double x_delta = ((double)x_start) - p1->sp.x();
	double y_delta = ((double)y_start) - p1->sp.y();

	z = p1->sp.z() + s.dzy * y_delta + s.dzx * x_delta;
	w = p1->sp.w() + s.dwy * y_delta + s.dwx * x_delta;

	for (int i = s.first_attr ; i < s.first_no_persp ; i++)
		attrbs[i] = p1->attr[i] * p1->sp.w() + s.day[i] * y_delta + s.dax[i] * x_delta;

	for (int i = s.first_no_persp ; i < s.last_attr ; i++)
		attrbs[i] = p1->attr[i] + s.day[i] * y_delta + s.dax[i] * x_delta;
}

void PixelState::stepX(const TriangleSetup &s)
{
	z += s.dzx;
	w += s.dwx;

	for (int i = s.first_attr ; i < s.last_attr ; i++)
		attrbs[i] += s.dax[i];
}

void PixelState::stepYX(const TriangleSetup &s, const int x_steps)
{
	z += (s.dzy + s.dzx * x_steps);
	w += (s.dwy + s.dwx * x_steps);

	for (int i = s.first_attr ; i < s.last_attr ; i++)
		attrbs[i] += (s.day[i] + s.dax[i] * x_steps);
}

void PixelState::setupPSInputs(const TriangleSetup &s, PS_INPUTS &ps)
{
	for (int i = s.first_attr ; i < s.first_no_persp ; i++)
		ps.attributes[i] = attrbs[i] / w;
	for (int i = s.first_no_persp ; i < s.first_no_persp ; i++)
		ps.attributes[i] = attrbs[i];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// draw triangle between points

void Renderer::drawTriangle(const TVertex* p1, const TVertex* p2, const TVertex* p3)
{
	PixelState firstColumnPixel;

	// sort the points from bottom to top by Y (do the simple bubble sort)
	if (p2->sp.y() > p3->sp.y())
		std::swap(p3, p2);
	if (p1->sp.y() > p2->sp.y())
		std::swap(p2, p1);
	if (p2->sp.y() > p3->sp.y())
		std::swap(p3, p2);

	/* and find the Ys of interest */
	int y_start = ceil(p1->sp.y());
	int y_middle = ceil(p2->sp.y());
	int y_end = floor(p3->sp.y());
	if (y_start > y_end)
		return;

	/* find steps on both sides*/
	double x1_step = make_step(p1,p3);
	double x2_step = make_step(p1,p2);

	/* find initial X on both sides */
	double y_fraction = ((double)y_start) - p1->sp.y();
	double x1 = p1->sp.x() + x1_step * y_fraction;
	double x2 = p1->sp.x() + x2_step * y_fraction;

	/* and switch sides if necessarily*/
	bool right_side_long = x1_step > x2_step;
	if (right_side_long) {
		std::swap(x1_step,x2_step);
		std::swap(x1,x2);
	}

	_setup.setup(p1,p2,p3);

	int x_start = ceil(x1), x_end = floor(x2);
	firstColumnPixel.start(_setup, p1, x_start, y_start);

	for (_psInputs.y = y_start ;  ; _psInputs.y++)
	{
		/* switch to bottom trapezoid if necessary*/
		if (_psInputs.y == y_middle)
		{
			double x_step = make_step(p2,p3);
			double y_fraction = ((double)y_middle) - p2->sp.y();
			double x = p2->sp.x() + x_step * y_fraction;

			if (right_side_long) {
				x1 = x; x1_step = x_step; x_start = ceil(x1);
				firstColumnPixel.start(_setup, p2, x_start, y_middle);
			} else {
				x2 = x; x2_step = x_step; x_end = floor(x2);
			}
		}

		/* rasterize the scan line now */
		PixelState pixel(firstColumnPixel);
		for (_psInputs.x = x_start ; _psInputs.x <= x_end ; _psInputs.x++, pixel.stepX(_setup))
		{
			/* do the (early Z test)*/
			if (_zBuffer && !_zBuffer->zTest(_psInputs.x,_psInputs.y, pixel.z))
				continue;

			/* run pixel shader if we have output buffer */
			if (_outputTexture)
			{
				_psInputs.d = pixel.z;
				pixel.setupPSInputs(_setup, _psInputs);
				drawPixel(_psInputs.x, _psInputs.y, _pixelShader(_psPriv, _psInputs));
			}
		}

		/* advance one scan line */
		if (_psInputs.y == y_end)
			break;

		int x_start_old = x_start;
		x1 += x1_step; x2 += x2_step;
		x_start = ceil(x1); x_end = floor(x2);
		firstColumnPixel.stepYX(_setup, x_start - x_start_old);
	}
}
