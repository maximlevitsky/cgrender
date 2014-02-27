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
#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H

class ObjectTransformation
{
public:
	ObjectTransformation() { reset(); }

	/* read stuff */
	Vector3 getMoveFactors() {return moveFactors;}
	Vector3 getScaleFactors() {return scaleFactors;}
	Mat4 getRotMat() { return rotationMatrix; }

	Mat4 getMat() { return matrix; }
	Mat4 getNormalTransformMatrix() { return normalTransformMatrix; }
	Mat4 getInvScaleMatrix() { return invScaleMatrix; }


	/* rotation */
	void setRotationMatrix(Mat4 matrix) {
		rotationMatrix = matrix;
		updateMatrix();
	}

	void setRotationMatrix2(Mat4 matrix)
	{
		rotationMatrixTemp = matrix;
		updateMatrix();
	}

	Mat4 getRotationMatrix2() const
	{ return rotationMatrixTemp; }


	void mergeRotationFactors() {
		rotationMatrix = rotationMatrix * rotationMatrixTemp;
		rotationMatrixTemp = Mat4::createUnit();
		updateMatrix();
	}

	/* scale */
	void setScaleFactors(Vector3 scale) {
		scaleFactors = scale;
		updateMatrix();
	}

	/* translate */
	void setMoveFactors(Vector3 translation) {
		moveFactors = translation;
		updateMatrix();
	}


	void reset()
	{
		rotationMatrixTemp = Mat4::createUnit();
		rotationMatrix = Mat4::createUnit();
		moveFactors = Vector3(0,0,0);
		scaleFactors = Vector3(1,1,1);
		updateMatrix();
	}


private:
	void updateMatrix()
	{
		matrix =
			Mat4::getScaleMatrix(scaleFactors) *
			rotationMatrix * rotationMatrixTemp *
			Mat4::getMoveMat(moveFactors);

		invScaleMatrix = Mat4::getScaleMatrix(
				Vector3(1.0/scaleFactors[0], 1.0/scaleFactors[1], 1.0/scaleFactors[2]));

		normalTransformMatrix = invScaleMatrix * rotationMatrix * rotationMatrixTemp;
	}

private:
	Mat4 rotationMatrix;
	Mat4 rotationMatrixTemp;
	Vector3 scaleFactors;
	Vector3 moveFactors;

	//////////////////////////////
	Mat4 matrix;
	Mat4 normalTransformMatrix;
	Mat4 invScaleMatrix;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CameraTransformation
{
public:
	CameraTransformation() { _invert = false; reset();  }

	/* data access */
	Vector3 getMoveFactors() {return moveFactors;}
	Mat4 getRotationMatrix() { return rotationMatrix; }

	Mat4 getRotMatI()
	{
		Mat4 invert = Mat4::getScaleMatrix(Vector3(1,1,_invert ? -1 : 1));
		return invert * rotationMatrix;
	}
	Mat4 getNormalTransformMatrix() { return normalTransformMatrix; }
	Mat4 getMat() { return matrix; }
	bool getInvert() { return _invert; }


	/* rotation */
	void setRotationMatrix(Mat4 matrix) {
		rotationMatrix = matrix;
		updateMatrix();
	}

	/* translate */
	void setMoveFactors(Vector3 translation)
	{
		moveFactors = translation;
		updateMatrix();
	}

	/* invert */
	void setInvert(bool invert)
	{
		_invert = invert;
		updateMatrix();
	}

	void reset()
	{
		rotationMatrix = Mat4::createUnit();
		moveFactors = Vector3(0,0,0);
		updateMatrix();
	}

private:
	void updateMatrix()
	{
		Mat4 invert = Mat4::getScaleMatrix(Vector3(1,1,_invert ? -1 : 1));
		matrix = invert * Mat4::getMoveMat(-moveFactors) * rotationMatrix;

		normalTransformMatrix =  invert *  rotationMatrix;
	}

private:
	Mat4 rotationMatrix;
	Vector3 moveFactors;
private:
	Mat4 matrix;
	Mat4 normalTransformMatrix;
	bool _invert;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////


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

	void getFrontPlane(double *W, double *H) {
		*W = _width;
		*H = _height;
	}

	/* update viewing frustum front plane distance (camera focal length) */
	void setDistance(double distance)
	{
		_distance = distance;
		updateMatrix();
	}

	double getDistance() const { return _distance; }


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
					Mat4::getOrthoProjMatrix(L,R,T,B,N,F):
					Mat4::getPerspMat(L,R,T,B,N,F));
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

#endif
