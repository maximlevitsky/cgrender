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
    HEADERS += generated/$${1}.h
    return(generated/$${1}.cpp)
}

defineReplace(FILE_IN_addToHeaders) {
    # qmake emits a warning unless this function is defined; not sure why.
}

###########################################################################################
flex.output_function = addToHeaders
flex.name = Flex ${QMAKE_FILE_IN}
flex.input = FLEXSOURCES

flex.commands = \
	flex -o ${QMAKE_FILE_PATH}/generated/${QMAKE_FILE_BASE}.l.cpp \
	--header-file=${QMAKE_FILE_PATH}/generated/${QMAKE_FILE_BASE}.l.hpp \
	${QMAKE_FILE_IN}

flex.CONFIG += target_predeps
flex.clean = ${QMAKE_FILE_PATH}/generated/${QMAKE_FILE_BASE}.l.hpp \
 	${QMAKE_FILE_PATH}/generated/${QMAKE_FILE_BASE}.l.cpp

flex.variable_out = SOURCES
silent:flex.commands = @echo Lex ${QMAKE_FILE_IN} && $$flex.commands
QMAKE_EXTRA_COMPILERS += flex

###########################################################################################
bison.name = Bison ${QMAKE_FILE_IN}
bison.input = BISONSOURCES
bison.output_function = addToHeaders

bison.commands = \
	bison -d -o ${QMAKE_FILE_PATH}/generated/${QMAKE_FILE_BASE}.y.cpp \
	${QMAKE_FILE_IN}

bison.CONFIG += target_predeps
bison.variable_out = SOURCES
silent:bison.commands = @echo Bison ${QMAKE_FILE_IN} && $$bison.commands

bison.clean = \
	${QMAKE_FILE_PATH}/generated/${QMAKE_FILE_BASE}.y.hpp \
 	${QMAKE_FILE_PATH}/generated/${QMAKE_FILE_BASE}.y.cpp

QMAKE_EXTRA_COMPILERS += bison

###########################################################################################