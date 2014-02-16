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
#include "model/Model.h"
#include "model/Material.h"
#include <string>
#include <map>
#include <list>

#include "common/Vector3.h"

class MtlLoader
{
public:
	MtlLoader() {};
	bool loadMaterials(std::string filename);
	MaterialParams* getMaterial(std::string name);
private:
	std::map<std::string, MaterialParams*> materials;
};


class Vertex
{
public:
	Vertex(int position_index, int texcoord_index, int normal_index) :
		position_index(position_index),
		texcoord_index(texcoord_index),
		normal_index(normal_index)
	{}

	Vertex() {}

	bool operator<(const Vertex &other)
	{
		if (position_index != other.position_index)
			return position_index < other.position_index;
		if (texcoord_index != other.texcoord_index)
			return texcoord_index < other.texcoord_index;
		if (normal_index != other.normal_index)
			return normal_index < other.normal_index;
	}

private:
	int position_index;
	int texcoord_index;
	int normal_index;
};

class ObjLoader
{
public:
	ObjLoader();
	bool  load(std::string file);

	/* functions to be called from parser */
	void addVertexPosition(Vector3 pos);
	void addTexCoord(Vector3 coord);
	void addVertexNormal(Vector3 normal);
	void addFace(std::list<Vertex>& faces);

	/**/
	void setObjectName(std::string name);
	void setGroupName(std::string name);
	void setMaterialName(std::string name);
	void setMatrialLib(std::string name);

	int transVertexPosition(int pos)
	{
		return pos >= 0 ? pos : positions.size() + pos;
	}

	int transTexCoord( int pos)
	{
		return pos >= 0 ? pos : texcoords.size() + pos;
	}

	int transVertexNormal( int pos)
	{
		return pos >= 0 ? pos : normals.size() + pos;
	}
private:

	void finishCurrentModel();

	// state
	MaterialParams currenMaterial;

	std::string currentObject;
	std::string currentGroup;

	std::vector<Vector3> positions;
	std::vector<Vector3> texcoords;
	std::vector<Vector3> normals;
private:
	MtlLoader matriallib;
	std::list<Model*> models;
};
