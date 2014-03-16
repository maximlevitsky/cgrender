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

defineReplace(addToHeaders) {
    HEADERS += $${FLEXBISON_DIR}/$${1}.h
    return($${FLEXBISON_DIR}/$${1}.cpp)
}

###########################################################################################
flex.name = Flex header ${QMAKE_FILE_IN}
flex.CONFIG += target_predeps

flex.input = FLEXSOURCES
flex.variable_out = GENERATED_FILES
flex.output = $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.l.hpp
flex.clean = $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.l.hpp $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.l.cpp

flex.commands = \
	flex -o $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.l.cpp \
	--header-file=$${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.l.hpp \
	${QMAKE_FILE_IN}

silent:flex.commands = @echo Lex ${QMAKE_FILE_IN} && $$flex.commands

flex_impl.name = Flex source for ${QMAKE_FILE_IN}
flex_impl.input = FLEXSOURCES
flex_impl.output = $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.l.cpp
flex_impl.variable_out = GENERATED_SOURCES

flex_impl.commands = $$escape_expand(\\n)
flex_impl.depends = $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.l.hpp

QMAKE_EXTRA_COMPILERS += flex flex_impl

###########################################################################################

bison.name = Bison header ${QMAKE_FILE_IN}
bison.CONFIG += target_predeps
bison.input = BISONSOURCES
bison.variable_out = GENERATED_FILES
bison.output = $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.y.hpp
bison.clean = $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.y.hpp $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.y.cpp

bison.commands = bison -d -o $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.y.cpp ${QMAKE_FILE_IN}

bison_impl.name = Bison source for ${QMAKE_FILE_IN}
bison_impl.input = BISONSOURCES
bison_impl.variable_out = GENERATED_SOURCES
bison_impl.commands = $$escape_expand(\\n)
bison_impl.depends = $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.y.hpp
bison_impl.output = $${FLEXBISON_DIR}/${QMAKE_FILE_BASE}.y.cpp

QMAKE_EXTRA_COMPILERS += bison bison_impl
silent:bison.commands = @echo Bison ${QMAKE_FILE_IN} && $$bison.commands

QMAKE_CXXFLAGS += -DYYDEBUG=1

###########################################################################################
