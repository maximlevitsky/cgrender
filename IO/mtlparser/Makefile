#################################################################################
#
#	This file is part of CG4.
#
#	Copyright (c) Inbar Donag and Maxim Levitsky
#
#    CG4 is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 2 of the License, or
#    (at your option) any later version.
#
#    CG4 is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with CG4.  If not, see <http://www.gnu.org/licenses/>.
#
##################################################################################

CXXFLAGS += -I../../../ -I../../
OUT=.obj
TARGET=libmtlparser.a

OBJS=generated/mtlparser.l.o generated/mtlparser.y.o
CLEAN_EXTRA= \
	generated/mtlparser.y.hpp generated/mtlparser.y.cpp \
	generated/mtlparser.l.cpp generated/mtlparser.l.hpp

-include ../../Makefile.common


# autogenertated parsers
generated/mtlparser.l.cpp : mtlparser.l  generated/mtlparser.y.cpp mtlparser.h
	flex -o  generated/mtlparser.l.cpp --header-file=generated/mtlparser.l.hpp mtlparser.l

generated/mtlparser.y.cpp : mtlparser.y mtlparser.h
	bison  -d -o generated/mtlparser.y.cpp mtlparser.y
	
