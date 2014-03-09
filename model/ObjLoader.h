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
#include <vector>
#include <string>
#include <map>
#include <list>
#include <sstream>

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


struct RawVertex
{
	int pos_index;
	int tex_index;
	int normal_index;
	bool first;

	bool operator<(const struct RawVertex& o) const
	{
		if (pos_index != o.pos_index) return pos_index< o.pos_index;
		if (tex_index != o.tex_index) return tex_index< o.tex_index;
		return normal_index< o.normal_index;
	}

	bool operator==(const struct RawVertex& o)const
	{
		return
			pos_index == o.pos_index &&
			tex_index == o.tex_index &&
			normal_index == o.normal_index;
	}

	std::string print()
	{
		std::stringstream s;
		s << pos_index << "/" << tex_index << "/" << normal_index;
		return s.str();
	}
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
	void addFace(std::list<RawVertex>& faces);

	/**/
	void setObjectName(std::string name);
	void setGroupName(std::string name = "default");
	void setMaterialName(std::string name);
	void setMatrialLib(std::string name);

	int tVPos(int pos);
	int tTexCoord(int pos);
	int tVNormal(int pos);

	std::list<Model*> models;


private:
	void finishCurrentModel();

	// state
	MaterialParams currenMaterial;

	std::string currentObject;
	std::string currentGroup;

	std::vector<Vector3> positions;
	std::vector<Vector3> texcoords;
	std::vector<Vector3> normals;

	std::vector<RawVertex> vertex_build_buffer;
	std::vector<int> geometry_buffer;
	std::map<RawVertex, int> vertex_data_index;

	int allocateVertex(RawVertex &v);

	int tRelIndex(int index, std::vector<Vector3>& data);

private:
	MtlLoader matriallib;
};
