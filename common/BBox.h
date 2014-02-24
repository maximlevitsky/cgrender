
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

#ifndef BBOX_H
#define BBOX_H

#include "Vector3.h"
#include "Mat4.h"


class BOUNDING_BOX
{
public:
	BOUNDING_BOX() : point1(0,0,0), point2(0,0,0) {}

	BOUNDING_BOX& operator+=(const BOUNDING_BOX &other)
	{
		point1 = point1.minimum(other.point1);
		point2 = point2.maximum(other.point2);
		return *this;
	}

	BOUNDING_BOX operator*(const Mat4 mat)
	{
		BOUNDING_BOX result;

		Vector3 row1(mat(0,0), mat(0,1), mat(0,2));
		Vector3 row2(mat(1,0), mat(1,1), mat(1,2));
		Vector3 row3(mat(2,0), mat(2,1), mat(2,2));
		Vector3 row4(mat(3,0), mat(3,1), mat(3,2));

		Vector3 xa = row1 * point1.x();
		Vector3 xb = row1 * point2.x();

		Vector3 ya = row2 * point1.y();
		Vector3 yb = row2 * point2.y();

		Vector3 za = row3 * point1.z();
		Vector3 zb = row3 * point2.z();

		result.point1 = xa.minimum(xb) + ya.minimum(yb) + za.minimum(zb) + row4;
		result.point2 = xa.maximum(xb) + ya.maximum(yb) + za.maximum(zb) + row4;

		return result;
	}

	void moveTo(Vector3 point)
	{
		point1[0] -= point[0];
		point1[1] -= point[1];
		point1[2] -= point[2];

		point2[0] -= point[0];
		point2[1] -= point[1];
		point2[2] -= point[2];
	}

	Vector3 getSizes() const
	{
		return point2 - point1;
	}

	Vector3 getCenter() const
	{
		return (point2 + point1) / 2;
	}

public:
	Vector3 point1;
	Vector3 point2;

};
#endif
