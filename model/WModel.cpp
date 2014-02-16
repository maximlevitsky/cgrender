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

WireFrameModel::WireFrameModel( int vertexMaxCount, int maxPolygonCount ) : _maxPolygonCount(maxPolygonCount), _maxVertexCount(vertexMaxCount),
	polygonCount(0), vertexCount(0)
{
	vertices = new Vertex[_maxVertexCount+1];
	polygons = new unsigned int [_maxPolygonCount+_maxVertexCount+1];
	next_polygon = polygons;
}

WireFrameModel::~WireFrameModel()
{
	delete [] vertices;
	delete [] polygons;
}

void WireFrameModel::addLine( Vector3 start, Vector3 direction, double length, Color c /*= Color(0,0,0)*/ )
{
	assert(polygonCount < _maxPolygonCount);

	unsigned int *geom_data = next_polygon;
	geom_data[0] = 2;
	geom_data[1] = allocateVertex(Vertex(toHomoCoords(start), c));
	geom_data[2] = allocateVertex(Vertex(toHomoCoords(start + direction  * length), c));
	next_polygon += 3;
	polygonCount++;
}

void WireFrameModel::addQuad( unsigned int v1, unsigned int v2, unsigned int v3, unsigned int v4 )
{
	unsigned int *geom_data = next_polygon;
	geom_data[0] = 4;
	geom_data[1] = v1;
	geom_data[2] = v2;
	geom_data[3] = v3;
	geom_data[4] = v4;
	next_polygon += 5;
	polygonCount++;
}


int WireFrameModel::allocateVertex(const WireFrameModel::Vertex &v)
{
	int ID = vertexCount++;
	Vertex *_v = &vertices[ID];
	*_v = v;
	return ID;
}

WireFrameModel* WireFrameModel::createBoxModel( BOUNDING_BOX box, Color c, int extraLines /*= 0*/ )
{
	WireFrameModel *boxModel = new WireFrameModel(6*4+extraLines*2, 6+extraLines);
	Vector3 dummyNormal(0,0,0);
	Vector3 sizes = box.point2 - box.point1;

	//vertexes
	unsigned int f[4];
	unsigned int b[4];

	// front face points
	Vector4 point = toHomoCoords(box.point1);
	f[0] = boxModel->allocateVertex(Vertex(point,c));
	point[0] += sizes[0];
	f[1] = boxModel->allocateVertex(Vertex(point,c));
	point[1] += sizes[1];
	f[2] = boxModel->allocateVertex(Vertex(point,c));
	point[0] -= sizes[0];
	f[3] = boxModel->allocateVertex(Vertex(point,c));

	// back face points
	point = toHomoCoords(box.point1);
	point[2] += sizes[2];
	b[0] = boxModel->allocateVertex(Vertex(point,c));
	point[0] += sizes[0];
	b[1] = boxModel->allocateVertex(Vertex(point,c));
	point[1] += sizes[1];
	b[2] = boxModel->allocateVertex(Vertex(point,c));
	point[0] -= sizes[0];
	b[3] = boxModel->allocateVertex(Vertex(point,c));

	// front side
	boxModel->addQuad(f[0], f[1], f[2], f[3]);
	// back size
	boxModel->addQuad(b[0], b[1], b[2], b[3]);
	// left side
	boxModel->addQuad(f[0], f[3], b[3], b[0]);
	// right side
	boxModel->addQuad(f[1], f[2], b[2], b[1]);
	// top side
	boxModel->addQuad(f[2], f[3], b[3], b[2]);
	// bottom side
	boxModel->addQuad(f[0], f[1], b[1], b[0]);

	return boxModel;
}

WireFrameModel* WireFrameModel::createVertexNormalModel( const Model* model, double scale, Mat4 scaleMatrix )
{
	WireFrameModel *result = new WireFrameModel(model->getNumberOfVertices() * 2*2, model->getNumberOfVertices()*2);

	for (int i = 0 ; i < model->getNumberOfVertices() ; i++)
	{
		Vector3 vertexNormal = (model->vertices[i].normal * scaleMatrix);
		if (vertexNormal.len() == 0)
			continue;

		vertexNormal.makeNormal();
		vertexNormal = vertexNormal * scaleMatrix;

		if (vertexNormal.len() < 1e-10f || vertexNormal.len() > 1e5)
			continue;

		result->addLine(model->vertices[i].position, vertexNormal, scale, Color(255,182,193) / 255);
		result->addLine(model->vertices[i].position +vertexNormal*scale,
			vertexNormal, scale/3, Color(1,0,0));
	}
	return result;
}

WireFrameModel* WireFrameModel::createPolygonNormalModel( const Model* model, double scale, Mat4 scaleMatrix )
{
	WireFrameModel *result = new WireFrameModel(model->getNumberOfPolygons()*2*2, model->getNumberOfPolygons()*2);

	for (polygonIterator iter(model->polygons, model->getNumberOfPolygons()); iter.hasmore() ; iter.next() )
	{
		if (iter.vertexCount() < 3)
			continue;

		const Model::Vertex &firstVertex = model->vertices[iter[0]];

		// take polygon normal from first vertex
		Vector3 polygonNormal = firstVertex.polygon->polygonNormal * scaleMatrix;
		if (polygonNormal.len() == 0)
			continue;

		polygonNormal.makeNormal();
		polygonNormal = polygonNormal * scaleMatrix;

		Vector3 centerPoint = firstVertex.polygon->polygonCenter;

		result->addLine(centerPoint, polygonNormal, scale, Color(255,127,80) / 255);
		result->addLine(centerPoint +polygonNormal*scale, polygonNormal, scale/3, Color(1,0,0));
	}

	return result;
}

WireFrameModel* WireFrameModel::createAxisModel(double scale)
{
	WireFrameModel *result = new WireFrameModel(6, 3);
	result->addLine(Vector3(0,0,0), Vector3(1,0,0), scale, Color(1,0,0));
	result->addLine(Vector3(0,0,0), Vector3(0,1,0), scale, Color(0,1,0));
	result->addLine(Vector3(0,0,0), Vector3(0,0,1), scale, Color(0,0,1));
	return result;
}
