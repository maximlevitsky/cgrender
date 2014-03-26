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
%option nounput yylineno reentrant  noyywrap never-interactive  nounistd bison-bridge stack prefix="obj_"
%x IDENTIFER_EXPECTED
%s REAL_EXPECTED

%{
	#include "ObjLoader.h"
	#include "objparser.h"
	#include "objparser.y.hpp"
	#include <stdlib.h>
%}

 /*********************************************************************************************/
%%

 /* ignored comment lines*/
<*>#[^\n]*								{}

 /* ignored non newline whitespace */
<*>[ \t\r]+								{}

 /* ignored escaped newlines - why I have to go through this :-(*/
<*>[\\][\r]?[\n]						{}

 /* newline - this is importaint to grammar */
<*>[\n]									{ BEGIN(INITIAL); return NEWLINE; }


 /* commands*/
o										{ return TOK_OBJECT; }
g										{ return TOK_GROUP; }
s										{ return TOK_SMOOTHGROUP; }
usemtl									{ return TOK_USEMTL; }
mtllib									{ return TOK_MTLLIB; }

 /* mesh data declaration */
v										{ return TOK_VERTEX; }
vn										{ return TOK_VERTEX_NORMAL; }
vt										{ return TOK_VERTEX_UV; }
f										{ return TOK_FACE; }

 /*********************************************************************************************/

 /* integer in vertex data - have seen it in the wild :-(*/
<REAL_EXPECTED>[+-]?[0-9]*				{
											yylval->real = strtod(yytext, NULL); 
											return TOK_REAL;
										}
 /* identifier */
<IDENTIFER_EXPECTED>[^\n\r\t ]*			{
											yylval->str = strndup(yytext, yyleng);
											return TOK_ID;
										}
 /*integer number */
[+-]?[0-9]*								{
											yylval->integer = strtol(yytext, NULL, 10);
											return TOK_INTEGER;
										}
 /* double numbers */
[+-]?[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?	{
											yylval->real = strtod(yytext, NULL); 
											return TOK_REAL;
										}

 /* used in face declarations */
"/"										{ return '/'; }

 /*********************************************************************************************/

<*>.									{ 
											printf("parser: unknown data detected at %i: %s\n", yylineno, yytext);
											return -1;
										}
%%

void obj_push_state (int new_state ,yyscan_t yyscanner) 
{
	return yy_push_state(new_state, yyscanner);
}

void obj_pop_state(yyscan_t yyscanner) 
{
	return yy_pop_state(yyscanner);
}