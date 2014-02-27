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
#ifndef MAT4_H
#define MAT4_H

#include <cmath>
#include <assert.h>
#include "Vector3.h"

#define EPSILON 1e-10
#define IS_DOUBLE_EQUAL(d1,d2) \
	((d1) <= (d2) + EPSILON && (d1) >= (d2) - EPSILON)

class Mat4
{
private:
	double m[4][4];
public:
	// creates an Mat4 filled with zeros
	Mat4() {
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				m[i][j] = 0;
			}
		}
	}

	// creates an predefined Mat4
	Mat4(double x1, double y1, double z1, double w1,
		double x2, double y2, double z2, double w2,
		double x3, double y3, double z3, double w3,
		double x4, double y4, double z4, double w4) 
	{
		m[0][0] = x1; m[0][1] = y1; m[0][2] = z1; m[0][3] = w1;
		m[1][0] = x2; m[1][1] = y2; m[1][2] = z2; m[1][3] = w2;
		m[2][0] = x3; m[2][1] = y3; m[2][2] = z3; m[2][3] = w3;
		m[3][0] = x4; m[3][1] = y4; m[3][2] = z4; m[3][3] = w4;
	}

	const double& operator()(int i, int j) const {
		return m[i][j];
	}

	double& operator()(int i, int j) {
		return m[i][j];
	}

	Mat4& operator=(const Mat4& Mat4) 
	{
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				m[i][j] = Mat4(i,j);
			}
		}
		return *this;
	}

	// creates unit Mat4
		Mat4 operator*(const Mat4& other) const {
		Mat4 result;
		for (int r = 0; r < 4; r++) {
			for (int c = 0; c < 4; c++) {
				for (int k = 0; k < 4; k++) {
					result(r, c) += ((*this)(r, k) * other(k, c));
				}
			}
		}

		return result;
	}

	void multiplyRowByScalar(int R, double scalar) {

		for (int i = 0; i < 4; i++) {
			(*this)(R, i) *= scalar;
		}
	}

	void swapRows(int Ri, int Rj) {

		for (int i = 0; i < 4; i++) {
			double temp = (*this)(Ri, i);
			(*this)(Ri, i) = (*this)(Rj, i);
			(*this)(Rj, i) = temp;
		}
	}

	//Ri -> Ri - s*Rj
	void subtractRow(int Ri, int Rj, double scalar) {

		Mat4 temp = *this;
		temp.multiplyRowByScalar(Rj, scalar);
		for (int i = 0; i < 4; i++) {
			(*this)(Ri, i) -= temp(Rj, i);
		}
	}

	//if first cell is zero, the Mat4 can't be reduced with our algorithm.
	//in this case the function will swap the first row with another one that
	//it's first cell is not zero.
	//if it's not the case, nothing happens.
	//if the first columns is all zero - Mat4 is singular.
	void firstCellZero(Mat4& m, int row, int column) {

		if (!IS_DOUBLE_EQUAL((*this)(row,column), 0.0)) 
			return;

		for (int i = row+1; i <4; i++) 
		{
			if (!IS_DOUBLE_EQUAL((*this)(i,column), 0.0)) 
			{
				(*this).swapRows(i, row);
				m.swapRows(i, row);
				return;
			}			
		}

		assert(0);
	}

	//leading is the first cell in a row that is not zero.
	//if there is no leading, the row is all zero therefore the Mat4 is singular.
	double getLeading(int R) {

		for (int i = 0; i < 4; i++) {
			if (!IS_DOUBLE_EQUAL((*this)(R,i), 0.0)) {
				return (*this)(R, i);
			}
		}
		assert(0);
	}

	int getLeadingCol(int R) {

		for (int i = 0; i <4; i++) {
			if (!IS_DOUBLE_EQUAL((*this)(R,i), 0.0)) {
				return i;
			}
		}
		assert(0);
	}

	Mat4 inv() const {

		Mat4 source = *this; // will be a unit Mat4
		Mat4 result = createUnit(); // will be the inverse Mat4 (if invertible)

		for (int r = 0; r < 4; r++) 
		{
			source.firstCellZero(result, r, r);

			double leading = source.getLeading(r);
			int leadingCol = source.getLeadingCol(r);
			// make leading  1
			source.multiplyRowByScalar(r, 1.0 / leading);
			result.multiplyRowByScalar(r, 1.0 / leading);

			// using current row to zero the cells under the leading
			for (int i = r + 1; i < 4; i++) {
				double scalar = source(i, leadingCol);
				source.subtractRow(i, r, scalar);
				result.subtractRow(i, r, scalar);
			}
		}
		// zero the cells above leadings
		for (int r = 3; r >= 0; r--) {
			for (int i = 0; i < r; i++) {
				double scalar = source(i, r);
				source.subtractRow(i, r, scalar);
				result.subtractRow(i, r, scalar);
			}
		}

		return result;
	}

	Mat4 transpose() const {
		Mat4 result;

		for (int i = 0 ; i < 4 ; i++)
			for (int j = 0 ; j < 4 ; j++)
				result(j,i) = m[j][i];

		return result;
	}


	static Mat4 createUnit()
	{
		Mat4 result;
		for (int i=0; i<4; i++) {
			result(i,i) = 1;
		}
		return result;

	}

	static  Mat4 getOrthoProjMatrix(double L, double R, double T, double B, double N, double F)
	{
		return Mat4 (
			(2.0)/(R-L),		0,					0,					0,
			0,					(2.0)/(T-B),		0,					0,
			0,					0,					-2.0/(F-N),			0,
			-(R+L)/(R-L),		-(T+B)/(T-B),		-(F+N)/(F-N),		1
			);
	}

	static  Mat4 getPerspMat(double L, double R, double T, double B, double N, double F)
	{
		return Mat4 (
			(2.0 *N)/(R-L),		0,					0,					0,
			0,					(2.0*N)/(T-B),		0,					0,
			(R+L)/(R-L),		(T+B)/(T-B),		-(F+N)/(F-N),	   -1,
			0,					0,					(-2.0*F*N)/(F-N),	0
			);
	}

	static  Mat4 getPersMat(double fov, double aspect, double N, double F)
	{
		double f = 1.0 / std::tan(fov * 0.5 * (M_PI / 180));

		return Mat4(
			f / aspect,			0,					0,					0,
			0,					f,					0,					0,
			0,					0,					-(F+N)/(F-N),		-1,
			0,					0,					(-2.0*F*N)/(F-N),	0);
	}

	static  Mat4 getRotMat(const Vector3 &rot)
	{
		// rotate by X
		Mat4 rotX(
			1,					0,		   			0,					0,
			0,					std::cos(-rot[0]),	std::sin(-rot[0]),	0,
			0,					-std::sin(-rot[0]),	std::cos(-rot[0]),	0,
			0,					0,					0,					1
		);

		// rotate by Y
		Mat4 rotY(
			std::cos(-rot[1]) ,	0,					std::sin(-rot[1]),	0,
			0,					1,					0,					0,
			-std::sin(-rot[1]),	0,					std::cos(-rot[1]),	0,
			0,					0,					0,					1
		);

		// rotate by Z
		Mat4 rotZ(
			std::cos(-rot[2]) ,	std::sin(-rot[2]),	0,					0,
			-std::sin(-rot[2]),	std::cos(-rot[2]),	0,					0,
			0,					0,					1,					0,
			0,					0,					0,					1
		);

		return rotX * rotY * rotZ;
	}


	static  Mat4 getScaleMatrix(const Vector3 &scales)
	{
		return Mat4(
			scales[0],			0,					0,					0,
			0,					scales[1],			0,					0,
			0,					0,					scales[2],			0,
			0,					0,					0,					1
		);
	}

	static inline Mat4 getMoveMat(const Vector3 &trans)
	{
		return Mat4(
			1,					0,					0,					0,
			0,					1,					0,					0,
			0,					0,					1,					0,
			trans[0],			trans[1],			trans[2],			1
		);
	}

	static inline Mat4 lookAt(Vector3 p, Vector3 direction, Vector3 up)
	{
		Vector3 n = direction.returnNormal();
		Vector3 u = n.cross(up).returnNormal();
		Vector3 v = n.cross(u);

		return Mat4(
			u[0], u[1], u[2], 0,
			v[0], v[1], v[2], 0,
			n[0], n[1], n[2], 0,
			p[0], p[1], p[2], 1);
	}

};

#endif
