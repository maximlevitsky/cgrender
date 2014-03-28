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

%name-prefix = "mtl_"
%pure-parser

%lex-param   { yyscan_t scanner }
%parse-param { MtlLoader *loader }
%parse-param { yyscan_t scanner }

/************************************************************************************/
/* source preamble */
/************************************************************************************/
%{
	#include "ObjLoader.h"
	#include "mtlparser.h"
	#include "mtlparser.l.hpp"
	#include "mtlparser.y.hpp"
	#include <iostream>
	
	int yyerror(MtlLoader *loader, yyscan_t scanner, const char* p) 
	{ 
		std::cout << "mtlparser: syntax error at " << mtl_get_lineno(scanner) << std::endl;
		return -1;
	}
%}


/************************************************************************************/
/* tokens */
/************************************************************************************/
%token TOKEN
%token END
%token NEWLINE

%token TOK_newmtl TOK_illum
%token TOK_colorchannel TOK_mapChannel
%token TOK_type TOK_options
%token TOK_o TOK_s TOK_mm TOK_clamp TOK_bm TOK_imfchan TOK_blendu TOK_blendv TOK_cc
%token TOK_t TOK_boost TOK_texres
%token TOK_on TOK_off
%token TOK_spectral TOK_factor TOK_xyz


%%
/************************************************************************************/
/* rules section */
/************************************************************************************/

FILE : Statements;
Statements : Statement Statements | Statement;

Statement : MaterialStatement | IllumModelStatement | 
	ColorStatement | mapStatement | reflMapStatement;

//////////////////////////////////////////////////////////////////////////////////////

MaterialStatement: TOK_newmtl TOKEN Sep
{
};

IllumModelStatement: TOK_illum TOKEN Sep
{	
};

//////////////////////////////////////////////////////////////////////////////////////

ColorStatement : TOK_colorchannel Color Sep
{
	// Ka/Kd/Ks/Tf
};

ColorStatement : TOK_colorchannel TOKEN Sep
{
	// Ns/Ni/d
};

//////////////////////////////////////////////////////////////////////////////////////

mapStatement : TOK_mapChannel TexOptions TOKEN TexOptions Sep
{
};


reflMapStatement : TexOptions TOK_type TOKEN TexOptions TOKEN TexOptions Sep
{
};
	
//////////////////////////////////////////////////////////////////////////////////////
	
TexOptions: 
	TOK_options TexOptionList
	{
		/* some options were given */
	} |
	{
		/* nothing - no options here */
	};

TexOptionList:
	TexOption
	{
	} |

	TexOptionList TexOption
	{
	};

//////////////////////////////////////////////////////////////////////////////////////

TexOption:
	// move the texture
	TOK_o Vector 
	{} |

	// scale the texture
	TOK_s Vector
	{} |
	
	// gain the texture values
	TOK_mm TOKEN TOKEN
	{} |

	// repeat or clamp
	TOK_clamp Boolean 
	{} |
	
	// bump map multiplier
	TOK_bm TOKEN 
	{} |

	// channel for scalar textures
	TOK_imfchan TOKEN 
	{} |
	
	// junk
	TOK_blendu Boolean |
	TOK_blendv Boolean |
	TOK_cc Boolean |
	TOK_t Vector |
	TOK_boost TOKEN |
	TOK_texres TOKEN;	
	
/////////////////////////////////////////////////////////////////////////////////////
	
Boolean: 
	TOK_on 
	{} | 
	TOK_off 
	{};

Vector:

	TOKEN TOKEN 
	{} |
	
	TOKEN TOKEN TOKEN 
	{};
	
	
Color: 
	TOKEN TOKEN TOKEN 
	{
	} |
	
	TOK_spectral TOKEN TOK_factor 
	{
	} |

	TOK_xyz TOKEN TOKEN TOKEN 
	{		
	};
	
//////////////////////////////////////////////////////////////////////////////////////

Sep: END | NEWLINE;

%%
