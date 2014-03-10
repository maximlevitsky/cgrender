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

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include "ObjLoader.h"

using std::cout;

typedef void* yyscan_t;

#include "objparser/generated/objparser.y.hpp"
#include "objparser/generated/objparser.l.hpp"

#include "mtlparser/generated/mtlparser.y.hpp"
#include "mtlparser/generated/mtlparser.l.hpp"

extern int obj_parse (ObjLoader *loader, yyscan_t scanner);
extern int mtl_parse (MtlLoader *loader, yyscan_t scanner);
extern int obj_debug,mtl_debug;

/*********************************************************************************/

bool MtlLoader::loadMaterials(std::string file)
{
	FILE *f = fopen(file.c_str(), "r");
	if (f == NULL)
		return false;

	yyscan_t scanner;
	mtl_lex_init(&scanner);
	mtl_set_in(f, scanner);
	int retval = mtl_parse(this, scanner);
	mtl_lex_destroy(scanner);
	return retval;
}

MaterialParams* MtlLoader::getMaterial(std::string name)
{
	auto iter = materials.find(name);
	if (iter == materials.end())
		return NULL;
	return iter->second;
}


/*********************************************************************************/

ObjLoader::ObjLoader()
{
	currentObject = "";
	currentGroup = "default";
}

/*********************************************************************************/

bool ObjLoader::load(std::string file)
{
	FILE *f = fopen(file.c_str(), "r");
	if (f == NULL)
		return false;

	cout << "Loading OBJ file from " << file << std::endl;

	yyscan_t scanner;
	obj_lex_init(&scanner);
	obj_set_in(f, scanner);
	obj_set_out(stdout, &scanner);
	obj_set_debug(1, &scanner);
	obj_debug = 0;
	int retval = obj_parse(this, scanner);
	obj_lex_destroy(scanner);

	finishCurrentModel();
	return retval;
}
/*********************************************************************************/

void ObjLoader::addVertexPosition(Vector3 pos)
{
	positions.push_back(pos);
}

void ObjLoader::addTexCoord(Vector3 coord)
{
	texcoords.push_back(coord);
}
void ObjLoader::addVertexNormal(Vector3 normal)
{
	normals.push_back(normal);
}

/********************************************************************************/

int ObjLoader::tRelIndex(int index, std::vector<Vector3>& data)
{
	if (index < 0)
	{
		/* if index is invalid, we will return negative value which is OK to mark invalid entry */
		return data.size() + index;
	}

	if (index > (int)data.size())
		return -1;

	return index - 1;
}

int ObjLoader::tVPos(int index)
{
	return tRelIndex(index, positions);
}

int ObjLoader::tTexCoord(int index)
{
	return tRelIndex(index, texcoords);
}

int ObjLoader::tVNormal(int index)
{
	return tRelIndex(index, normals);
}

/********************************************************************************/

int ObjLoader::allocateVertex(RawVertex &v)
{
	vertex_build_buffer.push_back(v);
	int retval = vertex_build_buffer.size() - 1;
	return retval;
}

void ObjLoader::addFace(std::list<RawVertex>& vertexes)
{
	std::list<int> vertexIndexes;
#if 0
	cout << "adding face:";

	for (auto iter: vertexes)
	{
		cout << " " << iter.print();
	}

	cout << std::endl;
#endif
	/* go over all new vertexes and try to reuse vertexes from vertex buffer */
	for (auto iter: vertexes)
	{
		assert(!iter.first);

		auto vertex_ptr = vertex_data_index.find(iter);
		int newVertexIndex;

		if (vertex_ptr != vertex_data_index.end())
		{
			newVertexIndex = vertex_ptr->second;
		} else {
			newVertexIndex = allocateVertex(iter);
		}

		vertexIndexes.push_back(newVertexIndex);
	}


	/* now we got list of indexes in vertex buffer
	 * We need to find the one that is not first of other polygon
	 * (all new vertexes we had received from parser are marked with first == false)
	 */

	for (int i = 0; i < (int)vertexIndexes.size() ; i++)
	{
		int index = vertexIndexes.front();

		assert(index >= 0 && index <= (int)vertex_build_buffer.size());
		RawVertex *v = &vertex_build_buffer[index];

		if (v->first == false)
			break;

		vertexIndexes.pop_front();
		vertexIndexes.push_back(index);
	}


	assert (vertexIndexes.size() > 0);
	assert(vertexIndexes.front() >= 0 && vertexIndexes.front() < (int)vertex_build_buffer.size());

	if (vertex_build_buffer[vertexIndexes.front()].first)
	{
		RawVertex v = vertex_build_buffer[vertexIndexes.front()];
		v.first = false;
		int newVertexIndex = allocateVertex(v);

		vertexIndexes.pop_front();
		vertexIndexes.push_front(newVertexIndex);

	}

	/* and now mark first vertex of our new face as front */
	RawVertex *v = &vertex_build_buffer[vertexIndexes.front()];
	assert(!v->first);
	v->first = true;

	/* now try to add all the vertexes to vertex cache map */
	for (auto iter : vertexIndexes )
	{
		int index = iter;
		RawVertex *v = &vertex_build_buffer[index];

		/* don't share vertexes with no given normal as it will be later calculated*/
		if (v->normal_index < 0)
			continue;

		vertex_data_index.erase(*v);
		vertex_data_index.insert(std::make_pair(*v, index));
	}

	geometry_buffer.push_back(vertexIndexes.size());
	geometry_buffer.insert(geometry_buffer.end(), vertexIndexes.begin(), vertexIndexes.end());
}

/*********************************************************************************/


void ObjLoader::finishCurrentModel()
{
	if (!geometry_buffer.size())
		return;

	Model *m = new Model(vertex_build_buffer.size(), geometry_buffer.size() - vertex_build_buffer.size());
	m->_defaultMaterial = currenMaterial;

	/* add all vertexes to the model */
	for (auto vertex: vertex_build_buffer)
	{
		Model::Vertex v;
		v.position = vertex.pos_index >= 0 ? positions[vertex.pos_index] : Vector3(0,0,0);
		v.texCoord = vertex.tex_index >= 0 ? texcoords[vertex.tex_index] : Vector3(0,0,0);
		v.normal = vertex.normal_index >= 0 ? normals[vertex.normal_index].returnNormal() : Vector3(0,0,0);
		v.polygon = NULL;
		m->allocateVertex(v);
	}

	/* add all the faces to the model */

	for (int i = 0 ; i < (int)geometry_buffer.size() ;)
	{
		/* get leading count */
		int count = geometry_buffer[i];
		i++;

		assert(count > 0);

		/* get geometry data */
		Model::PolygonData *p =  m->allocatePolygon();

		int first_vertex_idx = geometry_buffer[i];
		Model::Vertex *v = &m->vertices[first_vertex_idx];

		if(v->polygon == NULL)
			v->polygon = p;
		else {
			printf("shared data bug at geometry buffer at %i\n", i);
		}

		for (int j = 0 ; j < count ; j++)
		{
			int vertex_index = geometry_buffer[i];
			assert(vertex_index >= 0 && vertex_index < (int)vertex_build_buffer.size());
			m->addVertexToPolygon(geometry_buffer[i]);
			i++;
		}
	}

	geometry_buffer.clear();
	vertex_build_buffer.clear();
	vertex_data_index.clear();

	m->finalize();

	models.push_back(m);
}

/*********************************************************************************/
void ObjLoader::setObjectName(std::string name)
{
	finishCurrentModel();
	currentObject = name;

	cout << "Loading object:" << currentObject << std::endl;
}

void ObjLoader::setGroupName(std::string name)
{
	finishCurrentModel();
	currentGroup = name;

	cout << "Loading group:" << currentGroup << std::endl;
}

void ObjLoader::setMaterialName(std::string name)
{
	finishCurrentModel();
	currenMaterial.reset();

	MaterialParams* params = matriallib.getMaterial(name);
	if (params)
		currenMaterial = *params;

	cout << "Using material:" << name << std::endl;

}

void ObjLoader::setMatrialLib(std::string name)
{
	matriallib.loadMaterials(name);
	cout << "Using material library:" << name << std::endl;
}

/*********************************************************************************/
