#!/bin/bash

qmake -spec macx-g++
make -j4

rm -rf cgrender.app
cp -r ./bin/cgrender.app .
macdeployqt cgrender.app
