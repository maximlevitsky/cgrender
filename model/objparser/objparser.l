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
 /* declaration section */
 /************************************************************************************/

%{
	#include "ObjLoader.h"
	#include "objparser.h"
	#include "objparser.y.hpp"
	#include <stdlib.h>
%}

%option nounput
%option yylineno
%option reentrant 
%option noyywrap
%option never-interactive 
%option nounistd
%option bison-bridge
%option prefix="obj_"

%x IDENTIFER
%s DOUBLE_NUMBERS_EXPECTED


%%
 /************************************************************************************/
 /* rules section */
 /************************************************************************************/
 
 /*
 	yytext - text of last token
 	yyleng - length of last token
 	yylineno - last line number of the input
 	yyval = output for the grammar parser
 */


 /* ignored comment lines*/
<*>#[^\n]*								{}

 /* ignored non newline whitespace */
<*>[ \t\r]+								{}

 /* ignored escaped newlines - why I have to go through this :-(*/
<*>[\\][\r]?[\n]						{}

 /* newline - this is importaint to grammar */
<*>[\n]									{ BEGIN(INITIAL); return NEWLINE; }


 /* commands*/
o										{ BEGIN(IDENTIFER);  return TOK_OBJECT; }
g										{ BEGIN(IDENTIFER);  return TOK_GROUP; }
s										{ BEGIN(IDENTIFER);  return TOK_SMOOTHGROUP; }
usemtl									{ BEGIN(IDENTIFER);  return TOK_USEMTL; }
mtllib									{ BEGIN(IDENTIFER);  return TOK_MTLLIB; }

 /* mesh data declaration */
v										{ BEGIN(DOUBLE_NUMBERS_EXPECTED); return TOK_VERTEX; }
vn										{ BEGIN(DOUBLE_NUMBERS_EXPECTED); return TOK_VERTEX_NORMAL; }
vt										{ BEGIN(DOUBLE_NUMBERS_EXPECTED); return TOK_VERTEX_UV; }
f										{ return TOK_FACE; }

 /*********************************************************************************************/

 /* integer in vertex data - have seen it in the wild :-(*/
<DOUBLE_NUMBERS_EXPECTED>[+-]?[0-9]*	{
											yylval->real = strtod(yytext, NULL); 
											return TOK_NUMBER;
										}
 /*integer number */
[+-]?[0-9]*								{
											yylval->integer = strtol(yytext, NULL, 10);
											return TOK_INTEGER;
										}
 /* double numbers */
[+-]?[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?	{
											yylval->real = strtod(yytext, NULL); 
											return TOK_NUMBER;
										}

 /* identifier */
<IDENTIFER>[^\n\r\t ]*					{
											yylval->str = strndup(yytext, yyleng);
											return TOK_ID;
										}

 /*********************************************************************************************/

 /* used in face declarations */
"/"										{ return '/'; }

<*>.									{ 
											printf("parser: unknown data detected at %i: %s\n", yylineno, yytext);
											return -1;
										}
%%