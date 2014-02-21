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
	#include "mtlparser.h"
	#include "mtlparser.y.hpp"
	#include <stdlib.h>
%}

%option reentrant
%option noyywrap
%option never-interactive
%option nounistd
%option bison-bridge
%option prefix="mtl_"

%%
 /************************************************************************************/
 /* rules section */
 /************************************************************************************/
 
 /*
 	yytext - text of last token
 	mtl_lval = output for the grammar parser
 */


 /* ignored stuff - comments/etc */
"#"[^\n]*\n											{}
[ \t\n\r]+											{}



.													{ 
														printf("parser: unknown data detected: %s", yytext);
													}
%%