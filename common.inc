
OBJECTS_DIR = .obj
CONFIG -= warn_on
CONFIG += silent

EXTRA_INCLUDE = . ..

macx {

	PREBULT=../bin/prebuilt/mac

	EXTRA_INCLUDE += $$PREBULT/include
	EXTRA_LIBS= $$PREBULT/lib/libpng.a $$PREBULT/lib/libz.a

	QMAKE_CXXFLAGS+= \
		-g  -flto \
		-mtune=core2 -march=core2  -O3 -ffast-math   \
		-std=c++11 -Wall -Wno-reorder -Wno-unneeded-internal-declaration
}

unix:!macx {

	EXTRA_LIBS= -lpng -lz

	QMAKE_CXXFLAGS+= \
		-g  -flto \
		-mtune=core2 -march=core2  -O3 -ffast-math   \
		-std=c++11 -Wall -Wno-reorder  \
		-mfpmath=sse -mssse3 -msse3 -msse2 \
		-malign-double -ftree-loop-distribution -fstrict-aliasing -floop-interchange \
		-ftree-vectorize -ftree-slp-vectorize
}


INCLUDEPATH += $$EXTRA_INCLUDE
DEPENDSPATH += $$EXTRA_INCLUDE
