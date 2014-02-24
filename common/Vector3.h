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
#ifndef VECTOR3_H
#define VECTOR3_H

#include "common/Utilities.h"
#include <math.h>
#include <string.h>
#include <utility>
#include "Mat4.h"

class Vector3 
{
private:
	double data[3];
public:

	////////////////////////////////////////////////////////////////////////////////////////////////
	Vector3() {}

	Vector3(double x, double y, double z)
	{

		data[0] = x;
		data[1] = y;
		data[2] = z;
	}

	Vector3& operator=(const Vector3& v) 
	{
		memcpy(data, v.data, sizeof(data));
		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	const double& operator[](int i) const  { assert(i<3) ;return data[i];}
	double& operator[](int i)  { assert(i<3) ; return data[i]; }

	double x() const { return data[0]; }
	double y() const { return data[1]; }
	double z() const { return data[2]; }

	double &x() { return data[0]; }
	double &y() { return data[1]; }
	double &z() { return data[2]; }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	double len() const {
		return sqrt( data[0] * data[0] +  data[1] * data[1] +  data[2] * data[2]);
	}

	bool isBadFP() const {
		return is_bad_fp(data[0]) || is_bad_fp(data[1]) || is_bad_fp(data[2]);
	}


	Vector3& makeNormal() 
	{
		double length = len();
		data[0] /= length;
		data[1] /= length;
		data[2] /= length;
		return *this;
	}

	Vector3 returnNormal() const
	{
		double length = len();
		return Vector3(data[0] / length, data[1] / length, data[2] / length);
	}

	Vector3 cross (const Vector3 &v) const 
	{
		double x = data[1] * v.data[2] - data[2] * v.data[1];
		double y = data[2] * v.data[0] - data[0] * v.data[2];
		double z = data[0] * v.data[1] - data[1] * v.data[0];
		return Vector3(x,y,z);
	}

	double dot (const Vector3 &v) const
	{
		return
			data[0] * v.data[0] +
			data[1] * v.data[1] +
			data[2] * v.data[2];
	}

	Vector3 minimum(const Vector3 &other) const
	{
		return Vector3 (
			min(data[0], other.data[0]), 
			min(data[1], other.data[1]), 
			min(data[2], other.data[2])
		);
	}

	Vector3 maximum(const Vector3 &other) const
	{
		return Vector3 (
			max(data[0], other.data[0]), 
			max(data[1], other.data[1]), 
			max(data[2], other.data[2])
			);
	}

	Vector3& clamp() {
		if (data[0] > 1.0) data[0] = 1.0;
		if (data[1] > 1.0) data[1] = 1.0;
		if (data[2] > 1.0) data[2] = 1.0;
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector3 operator * (double scalar) const 
	{ return Vector3(data[0]*scalar, data[1]*scalar, data[2]*scalar); }


	Vector3 operator / (double scalar) const 
	{return Vector3(data[0]/scalar, data[1]/scalar, data[2]/scalar);}

	Vector3& operator *= (double scalar) {
		data[0] *= scalar; data[1] *= scalar; data[2] *= scalar;;
		return *this;
	}

	Vector3& operator /= (double scalar) {
		data[0] /= scalar; data[1] /= scalar; data[2] /= scalar;
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector3 operator - (const Vector3& v2) const
	{ return Vector3(data[0] - v2.data[0], data[1] - v2.data[1], data[2] - v2.data[2]);}

	Vector3 operator + (const Vector3& v2) const
	{ return Vector3(data[0] + v2.data[0], data[1] + v2.data[1], data[2] + v2.data[2]);}

	Vector3 operator * (const Vector3 &v2) const 
	{ return Vector3(data[0] * v2.data[0], data[1] * v2.data[1], data[2] * v2.data[2]);}

	Vector3 operator / (const Vector3 &v2) const 
	{ return Vector3(data[0] / v2.data[0], data[1] / v2.data[1], data[2] / v2.data[2]);}

	Vector3& operator += (const Vector3 &v) {
		data[0] += v.data[0]; data[1] += v.data[1]; data[2] += v.data[2];
		return *this;
	}

	Vector3& operator -= (const Vector3 &v) {
		data[0] -= v.data[0]; data[1] -= v.data[1]; data[2] -= v.data[2];
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	Vector3 operator -() const
	{
		return Vector3(-data[0], -data[1], -data[2]);
	}
};


typedef Vector3 Color;

#endif
