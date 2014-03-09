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
	
	int yyerror(MtlLoader *loader, yyscan_t scanner, const char* p) 
	{ 
		return 0;
		//output::errorSyn(yylineno); 
	}
%}


/************************************************************************************/
/* tokens */
/************************************************************************************/
%token MTL_NUMBER

%%
/************************************************************************************/
/* rules section */
/************************************************************************************/

FILE : Statements;
Statements : Statement Statements | Statement;
Statement : MTL_NUMBER
%%
