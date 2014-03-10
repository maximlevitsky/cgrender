#!/bin/bash

qmake -spec
make -j4
cp  ./bin/cgrender .
