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
#include "common/Utilities.h"

struct point 
{
	Vector4 position;
	Vector3 attributes[8];
};

static int clipAgainstPlane(point* input, int point_count, point* output, Vector4 plane, int attrib_count)
{
	point *out = output;
	int out_count = 0;

	for (int i = 0 ; i < point_count ; i++) 
	{
		// find the edge to clip
		point* p1 = input+i;
		point* p2 = ((i == point_count - 1) ? (input) : (input + i + 1));

		double dot1 =  p1->position.dot(plane);
		double dot2 =  p2->position.dot(plane);

		bool in1 = dot1 > 0.0;
		bool in2 = dot2 > 0.0;

		if (in1) {
			out->position = p1->position;
			for (int j = 0 ; j < attrib_count ;j++)
				out->attributes[j] = p1->attributes[j];

			out++;
			out_count++;
		}

		if (in1 != in2) 
		{
			if (in1 == false) {
				swap(p1,p2);
				swap(dot1,dot2);
			}

			double diff = dot1 - dot2;
			double t = dot1 / diff;

			out->position = p1->position + (p2->position - p1->position) * t;
			for (int j = 0 ; j < attrib_count ;j++)
				out->attributes[j] = p1->attributes[j] + (p2->attributes[j] - p1->attributes[j]) * t;

			out++;
			out_count++;
		}
	}

	return out_count;
}


void Renderer::drawClippedTriangle( const Vector4 *p1, const Vector4 *p2, const Vector4 *p3, const Vector3 *a1, const Vector3 *a2, const Vector3 *a3 )
{
	// whole triangle is behind front plane so no need to draw anything
	if (p1->w() <= 0 && p2->w() <= 0 && p3->w() <= 0)
		return;

	int attrCount = _vertexSmoothAttributeCount+_vertexNoPerspectiveCount;
	point input[16], output[16];

	// setup initial input
	input[0].position = *p1;
	input[1].position = *p2;
	input[2].position = *p3;

	for (int i = 0 ; i < attrCount ;i++) input[0].attributes[i] = a1[i];
	for (int i = 0 ; i < attrCount ;i++) input[1].attributes[i] = a2[i];
	for (int i = 0 ; i < attrCount ;i++) input[2].attributes[i] = a3[i];

	// clip against all clip planes
	int input_count = 3;
	input_count = clipAgainstPlane(input,  input_count, output, Vector4(-1,0,0,clip_x),attrCount);
	input_count = clipAgainstPlane(output, input_count, input,  Vector4(0,1,0,clip_y),attrCount);
	input_count = clipAgainstPlane(input,  input_count, output, Vector4(1,0,0,clip_x),attrCount);
	input_count = clipAgainstPlane(output, input_count, input,  Vector4(0,-1,0,clip_y),attrCount);

	// transform to clip space
	for (int i = 0 ; i < input_count ; i++)  {
		input[i].position = NDC_to_DeviceSpace(&input[i].position);
	}

	double z = 0;
	for (int i = 0 ; i < input_count ; i++)  
	{
		point* p1 = input+i;
		point* p2 = ((i == input_count - 1) ? (input) : (input + i + 1));
		z += (p1->position.x() - p2->position.x()) * (p1->position.y()+p2->position.y());
	}

	_psInputs.frontface = z > 0;
	

	// now break to triangles and render
	point* pp1 = &input[0];
	for (int i = 1 ; i < input_count - 1 ; i++) 
	{
		point *pp2 = &input[i];
		point *pp3 = (i == input_count - 1) ? &input[0] : &input[i + 1];
		drawTriangle(&pp1->position, &pp2->position, &pp3->position, pp1->attributes, pp2->attributes, pp3->attributes);
	}
}


bool Renderer::fastClipTriangle( const Vector4& v1, const Vector4 &v2, const Vector4 &v3 )
{
	// only clip all triangle and render whole thing otherwise, because its harmless
	if ((v1.x() > v1.w() * clip_x) && (v2.x() > v2.w() * clip_x) && (v3.x() > v3.w() * clip_x)) 
		return true;

	if ((v1.x() < -v1.w() * clip_x) && (v2.x() < -v2.w() * clip_x) && (v3.x() < -v3.w() * clip_x)) 
		return true;

	if ((v1.y() > v1.w() * clip_y) && (v2.y() > v2.w() * clip_y) && (v3.y() > v3.w() * clip_y)) 
		return true;

	if ((v1.y() < -v1.w() * clip_y) && (v2.y() < -v2.w() * clip_y) && (v3.y() < -v3.w() * clip_y))
		return true;

	return false;
}


bool Renderer::fastClipLine(const Vector4& v1, const Vector4 &v2) 
{
	if ((v1.x() > v1.w() * clip_x) && (v2.x() > v2.w() * clip_x) ) 
		return true;

	if ((v1.x() < -v1.w() * clip_x) && (v2.x() < -v2.w() * clip_x) ) 
		return true;

	if ((v1.y() > v1.w() * clip_y) && (v2.y() > v2.w() * clip_y)) 
		return true;

	if ((v1.y() < -v1.w() * clip_y) && (v2.y() < -v2.w() * clip_y))
		return true;

	return false;
}
