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
#ifndef COMMON_TRANSFORMATIONS_H
#define COMMON_TRANSFORMATIONS_H

#define _USE_MATH_DEFINES
#include <math.h>
#include "Mat4.h"
#include "Vector4.h"

namespace Transformations {

//////////////////////////////////////////////////////////////////////////////////////////////////////

static Mat4 getOrthoProjMatrix(double L, double R, double T, double B, double N, double F)
{
	return Mat4 (
		(2.0)/(R-L),		0,					0,					0,
		0,					(2.0)/(T-B),		0,					0,
		0,					0,					-2.0/(F-N),			0,
		-(R+L)/(R-L),		-(T+B)/(T-B),		-(F+N)/(F-N),		1
		);
}

static Mat4 getPerspectiveMatrix(double L, double R, double T, double B, double N, double F)
{
	return Mat4 (
		(2.0 *N)/(R-L),		0,					0,					0,
		0,					(2.0*N)/(T-B),		0,					0,
		(R+L)/(R-L),		(T+B)/(T-B),		-(F+N)/(F-N),	   -1,
		0,					0,					(-2.0*F*N)/(F-N),	0
		);
}

static Mat4 getPerspectiveMatrix(double fov, double aspect, double N, double F) 
{
	double f = 1.0 / tan(fov * 0.5 * (M_PI / 180));

	return Mat4(
		f / aspect,			0,					0,					0,
		0,					f,					0,					0,
		0,					0,					-(F+N)/(F-N),		-1,
		0,					0,					(-2.0*F*N)/(F-N),	0);
}


static Mat4 getGershonPerspectiveMatrix(double N, double F)
{
	return Mat4(
		1,					0,					0,					0,
		0,					1,					0,					0,
		0,					0,					F/(F-N),			1/F,
		0,					0,					-(N*F)/(F-N),		0);
}

static Mat4 getRotationalMatrix(const Vector3 &rot) 
{

	// rotate by X
	Mat4 rotX(
		1,					0,		   			0,					0,
		0,					cos(-rot[0]),		sin(-rot[0]),		0,
		0,					-sin(-rot[0]),		cos(-rot[0]),		0,
		0,					0,					0,					1
	);

	// rotate by Y
	Mat4 rotY(
		cos(-rot[1]) ,		0,					sin(-rot[1]),	    0,
		0,					1,					0,					0,
		-sin(-rot[1]),		0,					cos(-rot[1]),	    0,
		0,					0,					0,					1
	);

	// rotate by Z
	Mat4 rotZ(
		cos(-rot[2]) ,		sin(-rot[2]),		0,					0,
		-sin(-rot[2]),		cos(-rot[2]),		0,					0,
		0,					0,					1,					0,
		0,					0,					0,					1
	);

	return rotX * rotY * rotZ;
}

static Mat4 getScaleMatrix(const Vector3 &scales) 
{
	return Mat4(
		scales[0],			0,					0,					0,
		0,					scales[1],			0,					0,
		0,					0,					scales[2],			0,
		0,					0,					0,					1
	);
}

static Mat4 getTranlationMatrix(const Vector3 &trans) 
{
	return Mat4(
		1,					0,					0,					0,
		0,					1,					0,					0,
		0,					0,					1,					0,
		trans[0],			trans[1],			trans[2],			1
	);
}

static Mat4 lookAt(Vector3 p, Vector3 direction, Vector3 up) 
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

//////////////////////////////////////////////////////////////////////////////////////////////////////


class AffineTransformation 
{
public:
	AffineTransformation() { reset(); }

	// rotationAngles is a vector of angles in radians
	void setRotation(Vector3 rotationAngles){  rot = rotationAngles; updateMatrix();}
	// scale is a vector of coefficients to multiply the object size with
	void setScale(Vector3 scale) {scales = scale; updateMatrix();}
	// translation is a vector of displacements to move object in object space
	void setTranslation(Vector3 translation) {trans = translation; updateMatrix();}

	// return saved settings
	Vector3 getTranlation() {return trans;}
	Vector3 getRotation() {return rot;}
	Vector3 getScale() {return scales;}

	// get resulting matrix
	Mat4 getMatrix() { return matrix; }
	Mat4 getNormalTransformMatrix() { return normalTransformMatrix; }
	Mat4 getInvScaleMatrix() { return invScaleMatrix; }
	Mat4 getRotMatrix() { return rotationMatrix; }

	void reset() 
	{
		rot = Vector3(0,0,0);
		trans = Vector3(0,0,0);
		scales = Vector3(1,1,1);
		updateMatrix();
	}

private:
	void updateMatrix() 
	{
		matrix = getScaleMatrix(scales) * getRotationalMatrix(rot) *  getTranlationMatrix(trans);
		invScaleMatrix = getScaleMatrix(Vector3(1.0/scales[0], 1.0/scales[1], 1.0/scales[2]));
		normalTransformMatrix = invScaleMatrix * getRotationalMatrix(rot);
		rotationMatrix = getRotationalMatrix(rot);
	}

private:
	Mat4 matrix;
	Mat4 normalTransformMatrix;
	Mat4 invScaleMatrix;
	Mat4 rotationMatrix;
	Vector3 rot;
	Vector3 scales;
	Vector3 trans;
};


class CameraTransformation 
{
public:
	CameraTransformation() { _invert = false; reset();  }

	void setRotation(Vector3 rotationAngles){  rot = rotationAngles; updateMatrix();}
	void setTranslation(Vector3 translation) {trans = translation; updateMatrix();}

	// return saved settings
	Vector3 getTranlation() {return trans;}
	Vector3 getRotation() {return rot;}

	// get resulting matrix
	Mat4 getNormalTransformMatrix() { return normalTransformMatrix; }
	Mat4 getMatrix() { return matrix; }

	bool getInvert() { return _invert; }
	void setInvert(bool invert) { _invert = invert;  updateMatrix();}

	void reset() 
	{
		rot = Vector3(0,0,0);
		trans = Vector3(0,0,0);
		updateMatrix();
	}

private:
	void updateMatrix() 
	{
		Mat4 invert = Transformations::getScaleMatrix(Vector3(1,1,_invert ? -1 : 1));
		matrix = invert * getTranlationMatrix(-trans) * getRotationalMatrix(-rot);
		normalTransformMatrix = invert * getRotationalMatrix(-rot);
	}

private:
	Mat4 matrix;
	Mat4 normalTransformMatrix;
	Vector3 rot;
	Vector3 trans;
	bool _invert;
};


class ProjectionTransformation
{
public:
	ProjectionTransformation() : _perspective_enalbed(false) {}

	/* updates the viewing frustum front plane location (camera sensor size)*/
	void setFrontPlane(double W, double H)
	{
		_width = W;
		_height = H;
		updateMatrix();
	}

	/* update viewing frustum front plane distance (camera focal length) */
	void setDistance(double distance) 
	{
		_distance = distance;
		updateMatrix();
	}

	/* set depth of the scene - for possible Z clipping */
	void setDepth(double depth) 
	{
		_depth = depth;
		updateMatrix();
	}

	double getDepth() const {
		return _depth;
	}


	/* Enable or disable the perspective */
	void setPerspectiveEnabled(bool enable) 
	{ 
		_perspective_enalbed = enable; 
		updateMatrix();
	}

	// return current settings and the resulting matrix
	double getDistance() const { return _distance; }
	bool getPerspectiveEnabled() const { return _perspective_enalbed; }
	const Mat4 getMatrix() const  { return matrix; }

private:

	void updateMatrix() 
	{
		double N = _distance;
		double F = _distance + _depth;

		double L = -(_width / 2);
		double R = (_width / 2);

		double T = (_height / 2);
		double B = -(_height / 2);

		matrix = 
			(!_perspective_enalbed ?  
				getOrthoProjMatrix(L,R,T,B,N,F): 
				getPerspectiveMatrix(L,R,T,B,N,F));
	}

	// settings
	double _width;
	double _height;
	double _distance;
	double _depth;
	bool _perspective_enalbed;

	// resulting matrix
	Mat4 matrix;
};

};

#endif
