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

	while (1) {
		if (!_zBuffer || _zBuffer->zTest(x1,y1, z1))
			drawPixel(x1, y1, c);

		if (x1 == x2 && y1 == y2) break;

		z1 += dz;
		int e2 = 2 * d;

		if (e2 > -dy) { d -= dy; x1 += sx; }
		if (x1 == x2 && y1 == y2) continue;
		if (e2 < dx) { d += dx; y1 += sy; }
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
static void inline setup_attribute(
		const double dy1_ooa, const double dy2_ooa, const double dx1_ooa, const double dx2_ooa,
		const T& a1, const T& a2, const T& a3,
		T& dx, T&dy)
{
	const T da1 = (a1 - a2);
	const T da2 = (a3 - a1);

	dx = da1 * dy2_ooa - da2 * dy1_ooa;
	dy = da2 * dx1_ooa - da1 * dx2_ooa;
}

void TriangleSetup::setup(const TVertex* p1, const TVertex* p2, const TVertex* p3)
{
	// general triangle setup
	double dx1 = (p1->sp.x() - p2->sp.x()); double dx2 = (p3->sp.x() - p1->sp.x());
	double dy1 = (p1->sp.y() - p2->sp.y()); double dy2 = (p3->sp.y() - p1->sp.y());
	double ooa  = 1.0 / (dx1 * dy2 - dy1 * dx2);

	// these coefficients represent the inverse of position matrix
	double dy1_ooa  = dy1 * ooa, dy2_ooa  = dy2 * ooa;
	double dx1_ooa  = dx1 * ooa, dx2_ooa  = dx2 * ooa;

	// 1/w interpolation setup - the initial 1/w is already in sp.w()
	setup_attribute(dy1_ooa, dy2_ooa, dx1_ooa, dx2_ooa, p1->sp.w(), p2->sp.w(), p3->sp.w(), d_inv_wx, d_inv_wy);
	// z interpolation setup
	setup_attribute(dy1_ooa, dy2_ooa, dx1_ooa, dx2_ooa, p1->sp.z(), p2->sp.z(), p3->sp.z(), dzx, dzy);

	// perspective corrected attributes interpolation setup (we in essence divide here by w all the attributes)
	for (int i = first_attr ; i < first_no_persp ; i++)
		setup_attribute(dy1_ooa, dy2_ooa, dx1_ooa, dx2_ooa,
				p1->attr[i] * p1->sp.w(), p2->attr[i] * p2->sp.w(), p3->attr[i] * p3->sp.w(),
				dax[i], day[i]);

	// linear attributes interpolation setup
	for (int i = first_no_persp ; i < last_attr ; i++)
		setup_attribute(dy1_ooa, dy2_ooa, dx1_ooa, dx2_ooa,
				p1->attr[i], p2->attr[i], p3->attr[i], dax[i], day[i]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void PixelState::start(const TriangleSetup &s, const TVertex* p1, const int x_start, const int y_start)
{
	double x_delta = ((double)x_start) - p1->sp.x();
	double y_delta = ((double)y_start) - p1->sp.y();

	z = p1->sp.z() + s.dzy * y_delta + s.dzx * x_delta;
	inv_w = p1->sp.w() + s.d_inv_wy * y_delta + s.d_inv_wx * x_delta;

	for (int i = s.first_attr ; i < s.first_no_persp ; i++)
		attrbs[i] = p1->attr[i] * p1->sp.w() + s.day[i] * y_delta + s.dax[i] * x_delta;

	for (int i = s.first_no_persp ; i < s.last_attr ; i++)
		attrbs[i] = p1->attr[i] + s.day[i] * y_delta + s.dax[i] * x_delta;
}

void PixelState::stepX(const TriangleSetup &s)
{
	z += s.dzx;
	inv_w += s.d_inv_wx;

	for (int i = s.first_attr ; i < s.last_attr ; i++)
		attrbs[i] += s.dax[i];
}

void PixelState::stepYX(const TriangleSetup &s, const int x_steps)
{
	z += (s.dzy + s.dzx * x_steps);
	inv_w += (s.d_inv_wy + s.d_inv_wx * x_steps);

	for (int i = s.first_attr ; i < s.last_attr ; i++)
		attrbs[i] += (s.day[i] + s.dax[i] * x_steps);
}

void PixelState::setupPSInputs(const TriangleSetup &s, PS_INPUTS &ps)
{
	for (int i = s.first_attr ; i < s.first_no_persp ; i++)
		ps.attributes[i] = attrbs[i] / inv_w;
	for (int i = s.first_no_persp ; i < s.first_no_persp ; i++)
		ps.attributes[i] = attrbs[i];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// draw triangle between points

static inline double slope(const TVertex* p1, const TVertex* p2)
{return (p2->sp.x() - p1->sp.x())/ (p2->sp.y() - p1->sp.y());}

void Renderer::drawTriangle(const TVertex* p1, const TVertex* p2, const TVertex* p3)
{
	PixelState firstColumnPixel;
	double x1, x2, dxdy1 ,dxdy2;
	bool swap_x;

	// sort the points from bottom to top by Y (do the simple bubble sort)
	if (p2->sp.y() > p3->sp.y()) std::swap(p3, p2);
	if (p1->sp.y() > p2->sp.y()) std::swap(p2, p1);
	if (p2->sp.y() > p3->sp.y()) std::swap(p3, p2);

	/* and find the Y's of interest */
	int y_start = ceil(p1->sp.y()), y_middle = ceil(p2->sp.y()), y_end = floor(p3->sp.y());

	/* setup slopes and start/end locations */
	if (y_start < y_middle) {
		/* normal case - we start from top trapezoid and switch later to bottom one*/
		dxdy1 = slope(p1,p3); dxdy2 = slope(p1,p2);
		double y_fraction = (double)y_start - p1->sp.y();
		x1 = p1->sp.x() + dxdy1 * y_fraction;
		x2 = p1->sp.x() + dxdy2 * y_fraction;
		swap_x = dxdy1 > dxdy2;

	} else if (y_start <= y_end) {
		/* we start right away from bottom trapezoid */
		dxdy1 = slope(p1,p3); dxdy2 = slope(p2,p3);
		x1 = p1->sp.x() + dxdy1 * ((double)y_start - p1->sp.y());
		x2 = p2->sp.x() + dxdy2 * ((double)y_start - p2->sp.y());
		swap_x = dxdy1 < dxdy2;
	} else return;

	/* and swap x's if necessarily so start location is before the end location*/
	if (swap_x) {
		std::swap(dxdy1,dxdy2);
		std::swap(x1,x2);
	}

	/* triangle setup */
	_setup.setup(p1,p2,p3);

	/* find discrete location of first pixel we will draw and setup our scan-line first time */
	int x_start = ceil(x1), x_end = floor(x2);
	firstColumnPixel.start(_setup, p1, x_start, y_start);
	_psInputs.y = y_start;

	while (1)
	{
		/* rasterize one scan line now */
		PixelState pixel(firstColumnPixel);
		for (_psInputs.x = x_start ; _psInputs.x <= x_end ; _psInputs.x++, pixel.stepX(_setup))
		{
			/* do the (early Z test)*/
			if (_zBuffer && !_zBuffer->zTest(_psInputs.x,_psInputs.y, pixel.z))
				continue;

			/* run pixel shader if we have output buffer */
			if (_outputTexture) {
				_psInputs.d = pixel.z;
				pixel.setupPSInputs(_setup, _psInputs);
				drawPixel(_psInputs.x, _psInputs.y, _pixelShader(_psPriv, _psInputs));
			}
		}

		/* end condition */
		if (_psInputs.y == y_end) break;

		/* advance one scan-line*/
		if (++_psInputs.y == y_middle)
		{
			/* advance one edge and switch another to bottom trapezoid edge (p2->p3)*/
			double dxdy = slope(p2,p3);
			double x = p2->sp.x() + dxdy * (((double)y_middle) - p2->sp.y());

			if (swap_x) {
				/* same as  dxdy1 > dxdy2 which after swap became
				 * dxdy2 > dxdy1 that is shorter line is on the left
				 * so switch first edge
				 */
				x1 = x; dxdy1 = dxdy; x_start = ceil(x1);
				firstColumnPixel.start(_setup, p2, x_start, y_middle);
				x2 += dxdy2; x_end = floor(x2);
			} else {
				/* switch right edge otherwise */
				int x_start_old = x_start;
				x1 += dxdy1; x_start = ceil(x1);
				firstColumnPixel.stepYX(_setup, x_start - x_start_old);
				x2 = x; dxdy2 = dxdy; x_end = floor(x2);
			}
		} else {
			/* advance both edges */
			int x_start_old = x_start;
			x1 += dxdy1; x2 += dxdy2;
			x_start = ceil(x1); x_end = floor(x2);
			firstColumnPixel.stepYX(_setup, x_start - x_start_old);
		}
	}
}
