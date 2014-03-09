TEMPLATE = subdirs
SUBDIRS = renderer engine model/mtlparser model/objparser model gui

# build must be last:
CONFIG += ordered
SUBDIRS += bin