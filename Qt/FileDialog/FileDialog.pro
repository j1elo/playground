#====================================================================
#          PROJECT CONFIGURATION
#====================================================================

# Build configuration
TEMPLATE = app
QT *= core gui widgets

# Build locations
DESTDIR = .
MOC_DIR = tmp
OBJECTS_DIR = $$MOC_DIR
RCC_DIR = $$MOC_DIR
UI_DIR = $$MOC_DIR
unix: QMAKE_DISTCLEAN *= -r $$MOC_DIR



#====================================================================
#          PROJECT FILES
#====================================================================

HEADERS *= \
    FileDialog.h

SOURCES *= main.cpp \
    FileDialog.cpp
