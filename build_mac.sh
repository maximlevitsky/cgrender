#!/bin/bash

qmake -spec macx-g++
rm -rf ./bin/cgrender.app
make -j4
macdeployqt ./bin/cgrender.app