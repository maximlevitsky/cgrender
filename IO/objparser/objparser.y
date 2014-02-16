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


/************************************************************************************/
/* source preamble */
/************************************************************************************/
%{
	#include "ObjLoader.h"
	#include "objparser.h"
	#include "objparser.l.hpp"
	#include "objparser.y.hpp"
	
	int yyerror(ObjLoader *loader, yyscan_t scanner, const char* p) 
	{ 
		return 0;
		//output::errorSyn(yylineno); 
	}
%}

%name-prefix "obj_"

%define api.pure
%lex-param   { yyscan_t scanner }
%parse-param { ObjLoader *loader }
%parse-param { yyscan_t scanner }

/************************************************************************************/
/* tokens */
/************************************************************************************/
%token TOK_GROUP TOK_OBJECT TOK_SMOOTHGROUP
%token TOK_USEMTL TOK_MTLLIB
%token TOK_VERTEX TOK_VERTEX_NORMAL TOK_VERTEX_UV TOK_FACE
%token TOK_NUMBER TOK_INTEGER TOK_ID

%%
/************************************************************************************/
/* rules section */
/************************************************************************************/

FILE : Statements;
Statements : Statement Statements | Statement;
Statement : VertexStatement | VertexNormalStatement | VertexUVStatement | FaceStatement |
	ObjectStatement | GroupStatement | MaterialStatement | MatrialLibStatement;


VertexStatement:
	TOK_VERTEX TOK_NUMBER TOK_NUMBER TOK_NUMBER
	{
		loader->addVertexPosition(Vector3($2.num, $3.num, $4.num));
	};
	
VertexNormalStatement:
	TOK_VERTEX_NORMAL TOK_NUMBER TOK_NUMBER TOK_NUMBER
	{
		loader->addVertexNormal(Vector3($2.num, $3.num, $4.num));
	};
	
VertexUVStatement:
	TOK_VERTEX_UV TOK_NUMBER TOK_NUMBER
	{
		loader->addTexCoord(Vector3($2.num, $3.num, 0));
	};
	
FaceStatement:
	TOK_FACE VertexItems
	{
		loader->addFace($2.vertices);
	};
	
ObjectStatement:
	TOK_OBJECT TOK_ID
	{
		loader->setObjectName($2.str);
	};
	
GroupStatement:
	TOK_GROUP TOK_ID
	{
		loader->setGroupName($2.str);
	};
	
MaterialStatement:
	TOK_USEMTL TOK_ID
	{
		loader->setMaterialName($2.str);
	};
	
MatrialLibStatement: 
	TOK_MTLLIB TOK_ID
	{
		loader->setMatrialLib($2.str);
	};
	
 /*********************************************************************************/

VertexItem:
	/* only position*/
	TOK_INTEGER 
	{
		$$.vertex = Vertex(loader->transVertexPosition($1.integer), -1, -1);
	} |

	/* position/texcoord*/
	TOK_INTEGER '/' TOK_INTEGER 
	{
		$$.vertex = Vertex(loader->transVertexPosition($1.integer), loader->transTexCoord($3.integer), -1);
		
	} |
	
	/* position/texcoord/normal*/
	TOK_INTEGER '/' TOK_INTEGER '/' TOK_INTEGER
	{
		$$.vertex = Vertex(loader->transVertexPosition($1.integer), 
				loader->transTexCoord($3.integer), 
				loader->transVertexNormal($5.integer));		
	} |
	
	/* position//normal*/
	TOK_INTEGER '/'  '/' TOK_INTEGER
	{
		$$.vertex = Vertex(loader->transVertexPosition($1.integer), -1, loader->transVertexNormal($4.integer));	
	};

VertexItems:
	VertexItem
	{
		$$.vertices.push_back($1.vertex);
	} |

	VertexItems VertexItem
	{
		$$.vertices.swap($1.vertices);
		$$.vertices.push_back($2.vertex);
	};
	
 /*********************************************************************************/

%%