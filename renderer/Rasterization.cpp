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
// draw triangle between points

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
