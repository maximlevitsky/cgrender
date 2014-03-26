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
%option nounput yylineno reentrant  noyywrap never-interactive  nounistd bison-bridge prefix="mtl_"

%{
	#include "ObjLoader.h"
	#include "mtlparser.h"
	#include "mtlparser.y.hpp"
	#include <stdlib.h>
	
%}


%%
 /*********************************************************************************************/

Ka|Kd|Ks|Ns|Tf|Ni						{
										  yylval->str = strndup(yytext, yyleng);
										  return TOK_colorchannel;
										}

map_(Ka|Kd|Ks|Ns|Tf|Ni)					{
										  yylval->str = strndup(yytext, yyleng);
										  return TOK_mapChannel;
										}



newmtl									{ return TOK_newmtl; }
illum									{ return TOK_illum; }
-type									{ return TOK_type; }
-options								{ return TOK_options; }
-o										{ return TOK_o; }
-s										{ return TOK_s; }
-mm										{ return TOK_mm; }
-clamp									{ return TOK_clamp; }
-bm										{ return TOK_bm; }
-imfchan								{ return TOK_imfchan; }
-blendu									{ return TOK_blendu; }
-blendv									{ return TOK_blendv; }
-cc										{ return TOK_cc; }
-t										{ return TOK_t; }
-boost									{ return TOK_boost; }
-texres									{ return TOK_texres; }
-on										{ return TOK_on; }
-off									{ return TOK_off; }

spectral								{ return TOK_spectral; }
factor									{ return TOK_factor; }
xyz										{ return TOK_xyz; }



 /* ignored comment lines*/
<*>#[^\n]*								{}

 /* ignored non newline whitespace */
<*>[ \t\r]+								{}

 /* ignored escaped newlines - why I have to go through this :-(*/
<*>[\\][\r]?[\n]						{}

 /* newline - this is importaint to grammar */
<*>[\n]									{  return NEWLINE; }


 /* other token */
[^\n\r\t\- ][^\n\r\t ]*					{
											yylval->str = strndup(yytext, yyleng);
											yylval->integer = strtol(yytext, NULL, 10);
											yylval->real = strtod(yytext, NULL);
											
											return TOKEN;
										}

<*>.									{ 
											printf("parser: unknown data detected at %i: %s\n", yylineno, yytext);
											return -1;
										}
 /*********************************************************************************************/
%%
