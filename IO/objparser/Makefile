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
TARGET=libobjparser.a

OBJS=generated/objparser.l.o generated/objparser.y.o

CLEAN_EXTRA=\
	generated/objparser.y.hpp generated/objparser.y.cpp \
	generated/objparser.l.cpp generated/objparser.l.hpp

-include ../../Makefile.common

# autogenertated parsers
generated/objparser.l.cpp : objparser.l  generated/objparser.y.cpp objparser.h
	flex -o  generated/objparser.l.cpp --header-file=generated/objparser.l.hpp objparser.l

generated/objparser.y.cpp : objparser.y objparser.h
	bison  -d -o generated/objparser.y.cpp objparser.y
	
