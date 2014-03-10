#!/bin/bash

qmake -spec macx-g++
rm -rf ./bin/cgrender.app
make -j4

rm -rf cgrender.app
cp -r ./bin/cgrender.app .
macdeployqt cgrender.app