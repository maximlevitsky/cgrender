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
#include <assert.h>
#include "ObjLoader.h"
#include "parsers.h"

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

	yyscan_t scanner;
	obj_lex_init(&scanner);
	obj_set_in(f, scanner);
	int retval = obj_parse(this, scanner);
	obj_lex_destroy(scanner);
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

void ObjLoader::addFace(std::list<Vertex>& faces)
{
}

/*********************************************************************************/


void ObjLoader::finishCurrentModel()
{
	/* TODO: go over all accumualated faces and create a model from them */
}

/*********************************************************************************/
void ObjLoader::setObjectName(std::string name)
{
	finishCurrentModel();
	currentObject = name;
}

void ObjLoader::setGroupName(std::string name)
{
	finishCurrentModel();
}

void ObjLoader::setMaterialName(std::string name)
{
	finishCurrentModel();
	currenMaterial.reset();

	MaterialParams* params = matriallib.getMaterial(name);
	if (params)
		currenMaterial = *params;
}

void ObjLoader::setMatrialLib(std::string name)
{
	matriallib.loadMaterials(name);
}

/*********************************************************************************/
