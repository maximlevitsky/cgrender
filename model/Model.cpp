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
#include "Model.h"
#include <assert.h>
#include "common/Iterators.h"


Model::Model(int vertexMaxCount, int polygonMaxCount):  
	vertexMaxCount(vertexMaxCount), _vertexCount(0)
{
	/* Create a object with predefined amount of vertices in all its polygons*/
	vertices = new Vertex[vertexMaxCount+1];
	polygons = new unsigned int [vertexMaxCount+polygonMaxCount+1];

	polygonData = new Model::PolygonData[polygonMaxCount+1];

	current_polygon_ptr = NULL;
	current_polygon_last_vertex_ptr = NULL;
	polygon_aviable_space = vertexMaxCount + polygonMaxCount;
	currentPolygonData = NULL;
	_polygonCount = 0;
}

Model::~Model()
{
	delete [] vertices;
	delete [] polygons;
	delete [] polygonData;
}

Model::PolygonData* Model::allocatePolygon(void)
{
	/* Add a new polygon, with predefined amount of vertices*/
	assert(polygon_aviable_space > 1);

	if (!current_polygon_ptr) {
		current_polygon_ptr = polygons;
		currentPolygonData = polygonData;
	}
	else {
		current_polygon_ptr = current_polygon_last_vertex_ptr;
		currentPolygonData++;
	}

	current_polygon_last_vertex_ptr = current_polygon_ptr + 1;
	*current_polygon_ptr = 0;

	_polygonCount++;
	polygon_aviable_space--;
	return currentPolygonData;
}

int Model::allocateVertex( const Model::Vertex &v )
{
	assert(_vertexCount < vertexMaxCount);

	int ID = _vertexCount;
	Vertex* _v = &vertices[_vertexCount++];
	*_v = v;
	return ID;
}

void Model::addVertexToPolygon(int ID) 
{
	assert (polygon_aviable_space > 0);

	// add vertex to the end of current polygon
	*(current_polygon_last_vertex_ptr++) = ID;

	// increase the vertex count of current polygon
	(*current_polygon_ptr)++;
	polygon_aviable_space--;
}

BOUNDING_BOX  Model::getBoundingBox() const
{
	BOUNDING_BOX retval;

	if (!_vertexCount)
		return retval;

	double x1 = vertices[0].position[0];
	double y1 = vertices[0].position[1];
	double z1 = vertices[0].position[2]; 
	double x2 = vertices[0].position[0];
	double y2 = vertices[0].position[1];
	double z2 = vertices[0].position[2];

	for (int i=0; i<_vertexCount; i++) {
		if (vertices[i].position[0] < x1) {
			x1 = vertices[i].position[0];
		} 
		if (vertices[i].position[1] < y1) {
			y1 = vertices[i].position[1];
		}  
		if (vertices[i].position[2] < z1) {
			z1 = vertices[i].position[2];
		}
		if (vertices[i].position[0] > x2) {
			x2 = vertices[i].position[0];
		}
		if (vertices[i].position[1] > y2) {
			y2 = vertices[i].position[1];
		}
		if (vertices[i].position[2] > z2) {
			z2 = vertices[i].position[2];
		} 

	}

	retval.point1 = Vector3(x1,y1,z1);
	retval.point2 = Vector3(x2,y2,z2);
	return retval;
}

void Model::moveTo( Vector3 newCenter )
{
	for (int i = 0 ; i < _vertexCount; i++) 
	{
		vertices[i].position[0] -= newCenter[0];
		vertices[i].position[1] -= newCenter[1];
		vertices[i].position[2] -= newCenter[2];
	}

	for (int i = 0 ; i < _polygonCount; i++)
		polygonData[i].polygonCenter -= newCenter;
}

void Model::invertVertexNormals()
{
	for (int i = 0 ; i < _vertexCount; i++)
		vertices[i].normal = -vertices[i].normal;
}

void Model::invertPolygonNormals()
{
	for (int i = 0 ; i < _polygonCount; i++)
		polygonData[i].polygonNormal = -polygonData[i].polygonNormal;
}


void Model::finalize()
{
	for (polygonIterator iter(polygons, _polygonCount); iter.hasmore() ; iter.next() )
	{
		assert(vertices[iter[0]].polygon);

		if (iter.vertexCount() < 3)
			vertices[iter[0]].polygon->polygonCenter = Vector3(1,1,1);

		// find center point while iterating over all polygon vertexes
		Vector3 centerPoint(0,0,0);
		Vector3 normal(0,0,0);

		for ( int j = 0 ; j < iter.vertexCount() ; j++) {

			Vertex &v1= vertices[iter[j]];
			Vertex &v2 = j == iter.vertexCount() - 1 ?
					vertices[iter[0]] : vertices[iter[j+1]];

			centerPoint += v1.position;

			normal.x() += (v1.position.y() - v2.position.y()) * (v1.position.z()+v2.position.z());
			normal.y() += (v1.position.z() - v2.position.z()) * (v1.position.x()+v2.position.x());
			normal.z() += (v1.position.x() - v2.position.x()) * (v1.position.y()+v2.position.y());
		}

		for ( int j = 0 ; j < iter.vertexCount() ; j++)
		{
			Vertex &v1= vertices[iter[j]];
			if (v1.normal.len() == 0)
				v1.normal = normal;
		}


		centerPoint /= iter.vertexCount();
		vertices[iter[0]].polygon->polygonCenter = centerPoint;
		vertices[iter[0]].polygon->polygonNormal = normal;
	}
}

Model* Model::createTriangleModel( Vector3 a, Vector3 b, Vector3 c )
{
	Model *result = new Model(3, 3);
	Vector3 dummyNormal(0,0,1);

	Model::PolygonData *d;
	d = result->allocatePolygon();
	d->polygonCenter = Vector3(0,0,0);
	d->polygonNormal = Vector3(0,0,0);

	Vertex v;
	v.normal = dummyNormal;

	v.position = a;
	v.polygon = d;
	result->addVertexToPolygon(result->allocateVertex(v));

	v.position = b;
	result->addVertexToPolygon(result->allocateVertex(v));

	v.position = c;
	result->addVertexToPolygon(result->allocateVertex(v));
	result->finalize();
	return result;
}


#if 0
void calculateUV( const UniformBuffer *u, const Vector4 pos, double &u1, double &v1 )
{
	if (u->texMode == TM_PROJECTION) {
		u1 = pos.x();
		v1 = pos.y();
		return;
	}

	Vector4 opos = (pos * u->mat_cameraToObjectSpace).returnNormal();

	switch(u->texMode)
	{
	case TM_PROJECTION:
		// simple position
		u1 = pos.x();
		v1 = pos.y();
		break;
	case TM_CYLINDER:
		u1 = 0.5 + atan2(opos.z(), opos.x()) / (2 * M_PI);
		v1 = (opos.y() - u->y_min) / (u->y_max - u->y_min);
		break;
	case TM_SPHERE:
		u1 = 0.5 + atan2(opos.z(), opos.x()) / (2 * M_PI);
		v1 = 0.5 - asin(opos.y()) / (M_PI);
		break;
	default:
		assert(0);
	}
}
#endif
