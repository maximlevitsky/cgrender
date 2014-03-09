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
TEMPLATE = app
CONFIG += threads 
TARGET = cgrender


LIBS += \
	-L. -L../utils/pnglib/lib/ \
	../gui/.obj/qrc_resources.o \
	-lpng -lrenderer -lengine -lmtlparser -lobjparser -lmodel -lgui -lpng -lz

ICON = ../gui/resources/icon.icns

POST_TARGETDEPS += \
	librenderer.a \
	libengine.a \
	libmtlparser.a \
	libobjparser.a \
	libmodel.a \
	libgui.a \
	../gui/resources/icon.icns
