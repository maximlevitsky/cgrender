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

%option reentrant 
%option noyywrap
%option never-interactive 
%option nounistd
%option bison-bridge
%option prefix="obj_"

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


 /* grouping of objects */
o										{ return TOK_OBJECT; }
g										{ return TOK_GROUP; }
s										{ return TOK_SMOOTHGROUP; }


 /* commands*/
usemtl									{  return TOK_USEMTL; }
mtllib									{  return TOK_MTLLIB; }


 /* mesh data declaration */
v										{ return TOK_VERTEX; }
vn										{ return TOK_VERTEX_NORMAL; }
vt										{ return TOK_VERTEX_UV; }
f										{ return TOK_FACE; }


 /*integer number */
[+-]?[0-9]*								{
											yylval->integer = strtol(yytext, NULL, 10);
											return TOK_INTEGER;
										}

 /* double numbers */
[+-]?[0-9]+.[0-9]+						{
											yylval->num = strtod(yytext, NULL); 
											return TOK_NUMBER;
										}

 /* used in face declarations */
"/"										{ return '/'; }


 /* identifier */
[a-zA-z]([a-zA-z0-9_]*)					{
											yylval->str = yytext; 
											return TOK_ID;
										}

 /* ignored stuff - comments/etc */
"#"[^\n]*\n								{}
[ \t\n\r]+								{}

.										{ 
											printf("parser: unknown data detected: %s", yytext);
										}
%%