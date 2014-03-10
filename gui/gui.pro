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
include(../common.inc)
TARGET = ../bin/gui

TEMPLATE=lib
CONFIG += qt silent staticlib ordered
INCLUDEPATH += . ./autogenerated ../ ./widgets/
DEPENDPATH += $$INCLUDEPATH

DIALOGS_DIR = resources/dialogs
UI_DIR  = autogenerated
MOC_DIR = autogenerated
RCC_DIR = autogenerated

FORMS += $$DIALOGS_DIR/*.ui
RESOURCES += resources/resources.qrc	 
SOURCES += *.cpp widgets/*.cpp
HEADERS += *.h widgets/*.h
