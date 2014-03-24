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
	if (p1->posScr.isBadFP() || p2->posScr.isBadFP())
		return;

	int x1 = (int)p1->posScr.x(), x2 = (int)p2->posScr.x();
	int y1 = (int)p1->posScr.y(), y2 = (int)p2->posScr.y();

	// add small bias to Z so that wireframe is rendered above the model
	double z1 = p1->posScr.z() - 0.05, z2 = p2->posScr.z() - 0.05;

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
// draw triangle between points

void Renderer::drawTriangle(const TVertex* p1, const TVertex* p2, const TVertex* p3)
{
	int two_edges_side = 1; // determines which side consists of two edges

	// sort the points from left to right by Y (do the simple bubble sort)
	if (p2->posScr.y() > p3->posScr.y())
		std::swap(p3, p2);
	if (p1->posScr.y() > p2->posScr.y())
		std::swap(p2, p1);
	if (p2->posScr.y() > p3->posScr.y())
		std::swap(p3, p2);

	// find the order of two lines
	double p1p3_dx = p1->posScr.x() - p3->posScr.x();
	double p1p3_dy = p1->posScr.y() - p3->posScr.y();

	if (p1p3_dy && p2->posScr.x() > p1->posScr.x() + ((p2->posScr.y() - p1->posScr.y()) * p1p3_dx) / p1p3_dy)
	{
		std::swap(p3, p2);
		two_edges_side = 2;
	}

	// setup side line rasterizers
	_line1.setup(*p1, *p2);
	_line2.setup(*p1, *p3);

	// now do the rasterization....
	for (int stage = 0; stage < 2; stage++)
	{
		/* loop that runs from bottom up on both lines */
		while (!_line1.ended() && !_line2.ended())
		{
			_psInputs.y = _line1.y1_int;

			_line.setup(_line1, _line2);

			/* rasterize the horizontal line now */
			for (; !_line.ended(); _line.stepX())
			{
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

			two_edges_side == 1 ? _line1.setup(*p2, *p3) : _line2.setup(*p3, *p2);

			if (_line1.y1_int > _line2.y1_int)
				_line2.stepY();
			if (_line1.y1_int < _line2.y1_int)
				_line1.stepY();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void VerticalLineRasterizer::setAttributesCount(unsigned char flatcount, unsigned char smoothcount, unsigned char noPerspectiveCount )
{
	assert (flatcount + smoothcount + noPerspectiveCount <= 8);
	flatAttribCount = flatcount;
	smoothAttribCount = smoothcount;
	noPerspectiveAttribCount = noPerspectiveCount;
	smoothAndNoPerspectiveCount = (unsigned char)(smoothcount + noPerspectiveCount);
}

void VerticalLineRasterizer::setup(const TVertex& p1, const TVertex &p2 )
{
	// calculate initial values
	double y1 = p1.posScr.y(); double y2 = p2.posScr.y();
	double dy = y2 - y1;

	const Vector3* attr1 = p1.attr + flatAttribCount;
	const Vector3* attr2 = p2.attr + flatAttribCount;

	x1 = p1.posScr.x(); double x2 = p2.posScr.x();
	z1 = p1.posScr.z(); double z2 = p2.posScr.z();
	w1 = 1.0 / p1.posScr.w(); double w2 = 1.0 / p2.posScr.w();

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
			x1 += y1_fraction * x_step;
			z1 += y1_fraction * z_step;
			w1 += y1_fraction * w_step;

			for (int i = 0 ; i < smoothAndNoPerspectiveCount ; i++)
				attribs[i] += (attrib_steps[i] * y1_fraction);
		}
	}

	// calculates integer X,Y values we will go from to
	y1_int = (int)ceil(y1); y2_int = (int)floor(y2);
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

		if (x1_frac)
		{
			z1 += x1_frac * z_step;
			w1 += x1_frac* w_step;

			for (int i = 0 ; i < smoothAttribCount + noPerspectiveAttribCount ; i++)
				attributes[i] += (attribute_steps[i] * x1_frac);
		}
	}

	x1_int = (int)ceil(line1.x1);
	x2_int = (int)floor(line2.x1);
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
