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

void VerticalLineRasterizer::setAttributesCount(unsigned char flatcount, unsigned char smoothcount, unsigned char noPerspectiveCount )
{
	assert (flatcount + smoothcount + noPerspectiveCount <= MAX_ATTRIBUTES);
	flatAttribCount = flatcount;
	smoothAttribCount = smoothcount;
	noPerspectiveAttribCount = noPerspectiveCount;
	attribCount = (unsigned char)(smoothcount + noPerspectiveCount);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HorizintalLineRasterizer::setAttributesCount(
	unsigned char flatCount, unsigned char smoothcount, unsigned char noPerspectiveCount )
{
	assert (flatCount + smoothcount + noPerspectiveCount <= MAX_ATTRIBUTES);
	flatAttribCount = flatCount;
	smoothAttribCount = smoothcount;
	noPerspectiveAttribCount = noPerspectiveCount;
	attribCount = (unsigned char)(smoothAttribCount + noPerspectiveAttribCount);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// draw line between points

void Renderer::drawLine( TVertex *p1, TVertex *p2, const Color &c )
{
	int x1 = (int)(p1->posScr.x()), x2 = (int)(p2->posScr.x());
	int y1 = (int)(p1->posScr.y()), y2 = (int)(p2->posScr.y());

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
// draw triangle between points

void Renderer::drawTriangle(const TVertex* p1, const TVertex* p2, const TVertex* p3)
{
	// sort the points from bottom to top by Y (do the simple bubble sort)
	if (p2->posScr.y() > p3->posScr.y())
		std::swap(p3, p2);
	if (p1->posScr.y() > p2->posScr.y())
		std::swap(p2, p1);
	if (p2->posScr.y() > p3->posScr.y())
		std::swap(p3, p2);


	// setup side line rasterizers
	_line1.setup(*p1, *p2);
	_line2.setup(*p1, *p3);

	// now do the rasterization....
	for (int stage = 0; stage < 2; stage++)
	{
		/* loop that runs from bottom up on both lines */
		while (!_line1.ended() && !_line2.ended())
		{
			_line.setup(&_line1, &_line2);
			_psInputs.y = _line1.y1_int;

			/* rasterize the horizontal line now */
			for (; !_line.ended(); _line.stepX())
			{
				/* do the (early Z test)*/
				if (_zBuffer && !_zBuffer->zTest(_line.x1_int,_line1.y1_int, _line.z1))
					continue;

				/* run pixel shader if we have output buffer */
				if (_outputTexture)
				{
					_psInputs.x = _line.x1_int;
					_psInputs.d = _line.z1;
					_line.setupPSInputs(_psInputs);

					drawPixel(_psInputs.x, _psInputs.y, _pixelShader(_psPriv, _psInputs));
				}

			}
			/* step two lines */
			_line1.stepY();
			_line2.stepY();
		}

		/* switch bottom to top trapezoid */
		if (stage == 0) {

			_line1.setup(*p2, *p3);

			if (_line1.y1_int > _line2.y1_int)
				_line2.stepY();
			else if (_line1.y1_int < _line2.y1_int)
				_line1.stepY();
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void VerticalLineRasterizer::setup(const TVertex& p1, const TVertex &p2 )
{
	int i = 0; double y1;
	// calculate initial values
	x1 = p1.posScr.x(); const double x2 = p2.posScr.x();
	y1 = p1.posScr.y(); const double y2 = p2.posScr.y();
	z1 = p1.posScr.z(); const double z2 = p2.posScr.z();
	w1 = p1.posScr.w(); const double w2 = p2.posScr.w();

	const Vector3* attr1 = p1.attr + flatAttribCount;
	const Vector3* attr2 = p2.attr + flatAttribCount;

	//setup attributes
	for (i=0; i < smoothAttribCount ; i++)
		attribs[i] = attr1[i] * w1;
	for (; i < attribCount ; i++)
		attribs[i] = attr1[i];

	// calculates integer X,Y values we will go from to
	y1_int = (int)ceil(y1); y2_int = (int)floor(y2);

	// setup steps
	const double dy = y2 - y1;
	if(dy)
	{
		for ( i=0; i < smoothAttribCount ; i++)
			attrib_steps[i] = ( attr2[i] * w2 - attribs[i] ) / dy;
		for (; i < attribCount ; i++)
			attrib_steps[i] = (attr2[i] - attr1[i]) / dy;

		x_step = (x2 - x1)/dy;
		z_step = (z2 - z1)/dy;
		w_step = (w2 - w1)/dy;

		double y1_fraction = ceil(y1) - y1;

		if (y1_fraction) {
			x1 += y1_fraction * x_step;
			z1 += y1_fraction * z_step;
			w1 += y1_fraction * w_step;

			for (int i = 0 ; i < attribCount ; i++)
				attribs[i] += (attrib_steps[i] * y1_fraction);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////


void HorizintalLineRasterizer::setup( const VerticalLineRasterizer *line1, const VerticalLineRasterizer *line2 )
{
	double dx = line2->x1 - line1->x1;

	if (dx < 0) {
		swap(line1,line2);
		dx = -dx;
	}

	z1 = line1->z1;
	w1 = line1->w1;

	// setup pixel bounds
	x1_int = (int)ceil(line1->x1);
	x2_int = (int)floor(line2->x1);

	// setup attributes
	for (int i = 0 ; i < attribCount ; i++)
		attributes[i] = line1->attribs[i];

	// setup steps
	if (dx)
	{
		for (int i = 0 ; i < attribCount ; i++)
			attribute_steps[i] = (line2->attribs[i] - line1->attribs[i])/dx;

		z_step = (line2->z1 - line1->z1)/dx;
		w_step = (line2->w1 - line1->w1)/dx;

		// account for fractional X
		double x1_frac = ceil(line1->x1)  - line1->x1;

		if (x1_frac) {
			z1 += x1_frac * z_step;
			w1 += x1_frac* w_step;

			for (int i = 0 ; i < attribCount ; i++)
				attributes[i] += (attribute_steps[i] * x1_frac);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////



void VerticalLineRasterizer::stepY()
{
	x1 += x_step;
	w1 += w_step;
	z1 += z_step;
	y1_int++;

	for (int i = 0 ; i < attribCount ; i++)
		attribs[i] += attrib_steps[i];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void HorizintalLineRasterizer::stepX()
{
	// step the attributes
	z1 += z_step;
	w1 += w_step;
	x1_int++;

	for (int i = 0 ; i < attribCount; i++)
		attributes[i] += attribute_steps[i];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void HorizintalLineRasterizer::setupPSInputs(PS_INPUTS &inputs)
{
	int i = 0;

	Vector3 *attribs = inputs.attributes + flatAttribCount;

	for (; i < smoothAttribCount ; i++)
		attribs[i] = attributes[i] / w1;

	for (; i < smoothAttribCount+noPerspectiveAttribCount ; i++)
		attribs[i] = attributes[i];
}
