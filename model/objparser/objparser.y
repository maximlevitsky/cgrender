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

%name-prefix = "obj_"
%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { ObjLoader *loader }
%parse-param { yyscan_t scanner }

/************************************************************************************/
/* source preamble */
/************************************************************************************/
%{
	
	#define YY_HEADER_EXPORT_START_CONDITIONS
	
	#include "ObjLoader.h"
	#include "objparser.h"
	#include "objparser.l.hpp"
	#include "objparser.y.hpp"
	#include <list>
	#include <iostream>
	
	int yyerror(ObjLoader *loader, yyscan_t scanner, const char* p) 
	{ 
		std::cout << "syntax error at " << obj_get_lineno(scanner) << std::endl;
		return -1;
	}
	
	void obj_push_state (int new_state ,yyscan_t yyscanner);
	void obj_pop_state(yyscan_t yyscanner);
%}

/************************************************************************************/
/* tokens */
/************************************************************************************/
%token TOK_GROUP TOK_OBJECT TOK_SMOOTHGROUP
%token TOK_USEMTL TOK_MTLLIB
%token TOK_VERTEX TOK_VERTEX_NORMAL TOK_VERTEX_UV TOK_FACE
%token TOK_REAL TOK_INTEGER TOK_ID
%token NEWLINE
%token END 0

%%
/************************************************************************************/
/* rules section */
/************************************************************************************/

FILE : Statements;
Statements : Statements Statement | Statement;

Statement : VertexPosStatement | VertexNormalStatement | 
			VertexUVStatement | FaceStatement |
			ObjectStatement | GroupStatement | MaterialStatement | MatrialLibStatement | 
			SmoothGroupStmnt | NEWLINE;

VertexPosStatement:
	TOK_VERTEX EXPECT_REAL TOK_REAL TOK_REAL TOK_REAL END_EXPECT Sep
	{
		loader->addVertexPosition(Vector3($3.real, $4.real, $5.real));
	};
	
VertexNormalStatement:
	TOK_VERTEX_NORMAL EXPECT_REAL TOK_REAL TOK_REAL TOK_REAL END_EXPECT Sep
	{
		/* TODO */
		loader->addVertexNormal(Vector3($3.real, $4.real, $5.real));
	};
	
VertexUVStatement:
	TOK_VERTEX_UV EXPECT_REAL TOK_REAL TOK_REAL END_EXPECT Sep
	{
		loader->addTexCoord(Vector3($3.real, $4.real, 0));
	};
	
VertexUVStatement:
	TOK_VERTEX_UV EXPECT_REAL TOK_REAL TOK_REAL TOK_REAL END_EXPECT Sep
	{
		/* TODO */
		loader->addTexCoord(Vector3($3.real, $4.real, 0));
	};

	
FaceStatement:
	TOK_FACE VertexItems Sep
	{
		loader->addFace(*$2.vertices);
		delete $2.vertices;
		$2.vertices = NULL;
	};
	
ObjectStatement:
	TOK_OBJECT EXPECT_ID TOK_ID END_EXPECT Sep
	{
		loader->setObjectName($3.str);
		free($3.str);
	};
	
GroupStatement:
	TOK_GROUP  IdList  Sep
	{
		loader->setGroupName($2.str);
		free($2.str);
	};
	
MaterialStatement:
	TOK_USEMTL EXPECT_ID TOK_ID END_EXPECT Sep
	{
		loader->setMaterialName($3.str);
		free($3.str);
	};
	
MatrialLibStatement: 
	TOK_MTLLIB EXPECT_ID TOK_ID END_EXPECT Sep
	{
		loader->setMatrialLib($3.str);
		free($3.str);
	};
	
SmoothGroupStmnt:
	TOK_SMOOTHGROUP EXPECT_ID TOK_ID END_EXPECT Sep 
	{
		free($3.str);
	};
	/* ignore */

 /*********************************************************************************/

VertexItem:
	/* only position*/
	TOK_INTEGER 
	{
		$$.vertex.pos_index = loader->tVPos($1.integer);
		$$.vertex.tex_index = -1;
		$$.vertex.normal_index = -1;
		$$.vertex.first = false;
	} |

	/* position/texcoord*/
	TOK_INTEGER '/' TOK_INTEGER 
	{
		$$.vertex.pos_index = loader->tVPos($1.integer);
		$$.vertex.tex_index = loader->tTexCoord($3.integer);
		$$.vertex.normal_index = -1;		
		$$.vertex.first = false;
	} |
	
	/* position/texcoord/normal*/
	TOK_INTEGER '/' TOK_INTEGER '/' TOK_INTEGER
	{
		$$.vertex.pos_index = loader->tVPos($1.integer);
		$$.vertex.tex_index = loader->tTexCoord($3.integer);
		$$.vertex.normal_index = loader->tVNormal($5.integer);		
		$$.vertex.first = false;
	} |
	
	/* position//normal*/
	TOK_INTEGER '/'  '/' TOK_INTEGER
	{
		$$.vertex.pos_index = loader->tVPos($1.integer);
		$$.vertex.tex_index = -1;
		$$.vertex.normal_index = loader->tVNormal($4.integer);
		$$.vertex.first = false;
	};

VertexItems:
	VertexItem
	{
		$$.vertices = new std::list<RawVertex>(1, $1.vertex);
	} |

	VertexItems VertexItem
	{
		$$.vertices = $1.vertices;
		$$.vertices->push_back($2.vertex);
	};
	
 /*********************************************************************************/

	/* for now take first group*/
IdList :
	IdList EXPECT_ID TOK_ID END_EXPECT
	{
		$$.str = $3.str;
		free($3.str);
	}

	| EXPECT_ID TOK_ID END_EXPECT
	{
		$$.str = $2.str;
	} |
	
	EXPECT_ID END_EXPECT
	{ 
		$$.str = strdup("");
	}
;

EXPECT_ID:
	{ obj_push_state(IDENTIFER_EXPECTED, scanner); };
EXPECT_REAL:
	{ obj_push_state(REAL_EXPECTED, scanner); };
END_EXPECT:
	{obj_pop_state(scanner);};
	
Sep: END | NEWLINE;
	
 /*********************************************************************************/

%%
