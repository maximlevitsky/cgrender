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
#ifndef MODEL_H
#define MODEL_H

#include "common/Vector4.h"
#include "common/Utilities.h"
#include "common/BBox.h"
#include "Material.h"
#include <string>

class Model 
{
public:

	struct PolygonData
	{
		Vector3 polygonNormal;
		Vector3 polygonCenter;
	};

	struct Vertex
	{
		Vector3 position;
		Vector3 normal;
		Vector3 texCoord;
		PolygonData* polygon;
	};

	Model(int vertexMaxCount, int polygonMaxCount);

	// information on the model
	int getNumberOfVertices() const { return _vertexCount; }
	int getNumberOfPolygons() const { return _polygonCount; }
	BOUNDING_BOX getBoundingBox() const;

	// model manipulation
	void moveTo(Vector3 newCenter); 	// moves permanently whole model to new center
	void invertVertexNormals();
	void invertPolygonNormals();

	// continuation of construction
	void allocatePolygon(const PolygonData  &data);
	int allocateVertex(const Vertex &v);
	void addVertexToPolygon(int ID);

	void finalize();
	~Model();

public:
	// model data
	Vertex* vertices; 	// this is array of all vertices of all polygons.

	PolygonData *polygonData;
	unsigned int *polygons;

	// model properties
	MaterialParams _defaultMaterial;

private:
	int _vertexCount;
	int _polygonCount;

	// construction
	int vertexMaxCount;
	PolygonData *currentPolygonData;
	int polygon_aviable_space;
	unsigned int *current_polygon_ptr;
	unsigned int *current_polygon_last_vertex_ptr;

public:
	static Model* createTriangleModel(Vector3 a, Vector3 b, Vector3 c);
};


/* Simple model for boxes, axes, normals, etc*/
class WireFrameModel 
{
public:

	WireFrameModel(int vertexMaxCount, int maxPolygonCount);

	struct Vertex 
	{
		Vertex(const Vector4 pos, const Color c) : position(pos), c(c) {}
		Vertex() {}

		Vector4 position;
		Color c;
	};

	void addLine(Vector3 start, Vector3 direction, double length, Color c = Color(0,0,0));
	void addQuad(unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4);
	int allocateVertex(const Vertex &v);

	// helpers to create one
	static WireFrameModel* createBoxModel(BOUNDING_BOX box, Color c, int extraLines = 0);
	static WireFrameModel* createVertexNormalModel(const Model* model, double scale, Mat4 scaleMatrix);
	static WireFrameModel* createPolygonNormalModel(const Model* model, double scale, Mat4 scaleMatrix);
	static WireFrameModel* createAxisModel(double scale);

	int getNumberOfVertices() const { return vertexCount; }
	int getNumberOfPolygons() const { return polygonCount; }

	// data
	unsigned int *polygons;
	Vertex* vertices;

	~WireFrameModel();
private:
	int polygonCount;
	int vertexCount;
	int _maxPolygonCount;
	int _maxVertexCount;

	unsigned int *next_polygon;
};

#endif
