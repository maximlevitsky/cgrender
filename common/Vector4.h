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
#ifndef VECTOR4_H
#define VECTOR4_H

#include <math.h>
#include <utility>

#include "Mat4.h"
#include "Vector3.h"

class Vector4 
{
private:
	double data[4];
public:

	////////////////////////////////////////////////////////////////////////////////////////////////
	Vector4() {}

	bool operator<(const  Vector4& other) const {
		if (x() != other.x())
			return x() < other.x();

		if (y() != other.y())
			return y() < other.y();


		if (z() != other.z())
			return z() < other.z();

		return w() < other.w();
	}

	Vector4(double x, double y, double z, double w)
	{
		data[0] = x;
		data[1] = y;
		data[2] = z;
		data[3] = w;
	}

	Vector4& operator=(const Vector4& v) 
	{
		data[0] = v.data[0];
		data[1] = v.data[1];
		data[2] = v.data[2];
		data[3] = v.data[3];
		return *this;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	const double& operator[](int i) const  {return data[i];}
	double& operator[](int i)  { return data[i]; }

	double x() const { return data[0]; }
	double y() const { return data[1]; }
	double z() const { return data[2]; }
	double w() const { return data[3]; }

	double &x() { return data[0]; }
	double &y() { return data[1]; }
	double &z() { return data[2]; }
	double &w() { return data[3]; }

	bool isBadFP() const
	{
		return is_bad_fp(data[0]) || is_bad_fp(data[1]) || is_bad_fp(data[2]) || is_bad_fp(data[3]);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void canonicalize() 
	{
		data[0] /= data[3];
		data[1] /= data[3];
		data[2] /= data[3];
		data[3] = 1;
	}

	double len() const
	{
		return sqrt( data[0] * data[0] +  data[1] * data[1] +  data[2] * data[2] +  data[3] * data[3]);
	}

	Vector4& makeNormal() 
	{
		double length = len();
		data[0] /= length;
		data[1] /= length;
		data[2] /= length;
		data[3] /= length;
		return *this;
	}

	Vector4 returnNormal() const
	{
		double length = len();
		return Vector4(data[0] / length, data[1] / length, data[2] / length, data[3] / length);
	}

	Vector4 cross (const Vector4 &v) const 
	{
		double x = data[1] * v.data[2] - data[2] * v.data[1];
		double y = data[2] * v.data[0] - data[0] * v.data[2];
		double z = data[0] * v.data[1] - data[1] * v.data[0];
		return Vector4(x,y,z,0);
	}

	double dot (const Vector4 &v) const
	{
		return 
			data[0] * v.data[0] + 
			data[1] * v.data[1] + 
			data[2] * v.data[2] + 
			data[3] * v.data[3];
	}

	Vector4 minimum(const Vector4 &other) const
	{
		return Vector4 (
			min(data[0], other.data[0]), 
			min(data[1], other.data[1]), 
			min(data[2], other.data[2]), 
			min(data[3], other.data[3]));
	}

	Vector4 maximum(const Vector4 &other) const
	{
		return Vector4 (
			max(data[0], other.data[0]), 
			max(data[1], other.data[1]), 
			max(data[2], other.data[2]), 
			max(data[3], other.data[3]));
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector4 operator * (double scalar) const 
	{ return Vector4(data[0]*scalar, data[1]*scalar, data[2]*scalar, data[3]*scalar); }


	Vector4 operator / (double scalar) const 
	{ return Vector4(data[0]/scalar, data[1]/scalar, data[2]/scalar, data[3]/scalar);}

	Vector4& operator *= (double scalar) {
		data[0] *= scalar; data[1] *= scalar; data[2] *= scalar; data[3] *= scalar;
		return *this;
	}

	Vector4& operator /= (double scalar) {
		data[0] /= scalar; data[1] /= scalar; data[2] /= scalar; data[3] /= scalar;
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector4 operator - (const Vector4& v2) const
	{ return Vector4(data[0] - v2.data[0], data[1] - v2.data[1], data[2] - v2.data[2], data[3] - v2.data[3]);}

	Vector4 operator + (const Vector4& v2) const
	{ return Vector4(data[0] + v2.data[0], data[1] + v2.data[1], data[2] + v2.data[2], data[3] + v2.data[3]);}

	Vector4 operator * (const Vector4 &v2) const 
	{ return Vector4(data[0] * v2.data[0], data[1] * v2.data[1], data[2] * v2.data[2], data[3] * v2.data[3]);}

	Vector4 operator / (const Vector4 &v2) const 
	{ return Vector4(data[0] / v2.data[0], data[1] / v2.data[1], data[2] / v2.data[2], data[3] / v2.data[3]);}

	Vector4& operator += (const Vector4 &v) {
		data[0] += v.data[0]; data[1] += v.data[1]; data[2] += v.data[2]; data[3] += v.data[3];
		return *this;
	}

	Vector4& operator -= (const Vector4 &v) {
		data[0] -= v.data[0]; data[1] -= v.data[1]; data[2] -= v.data[2]; data[3] -= v.data[3];
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////

	Vector4 operator* (const Mat4 &other) const
	{
		return Vector4 (
			data[0]*other(0,0) + data[1]*other(1,0) + data[2]*other(2,0) + data[3]*other(3,0),
			data[0]*other(0,1) + data[1]*other(1,1) + data[2]*other(2,1) + data[3]*other(3,1),
			data[0]*other(0,2) + data[1]*other(1,2) + data[2]*other(2,2) + data[3]*other(3,2),
			data[0]*other(0,3) + data[1]*other(1,3) + data[2]*other(2,3) + data[3]*other(3,3)
		);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////

	Vector4 operator -() const
	{
		return Vector4(-data[0], -data[1], -data[2], -data[3]);
	}


	Vector3 xyz() const {
		return Vector3(data[0], data[1], data[2]);
	}
};

static Vector4 toHomoCoords(const Vector3& v) {
	return Vector4(v[0], v[1], v[2], 1);
}


#endif
