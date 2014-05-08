#====================================================================
#          PROJECT CONFIGURATION
#====================================================================

message("Processing $${_FILE_}")
CONFIG(debug, debug|release): BUILD_TARGET = debug
CONFIG(release, debug|release): BUILD_TARGET = release
message("Building [$${BUILD_TARGET}] Makefile for [$${TARGET}] on [$${QMAKE_HOST.os}] [$${QMAKE_HOST.arch}]")

# Build configuration
#TARGET = target # If not defined: same as the file name
TEMPLATE = app
CONFIG *= qt thread
QT *= core gui widgets xml

# Build locations
DESTDIR = .
MOC_DIR = tmp
OBJECTS_DIR = $$MOC_DIR
RCC_DIR = $$MOC_DIR
UI_DIR = $$MOC_DIR
unix: QMAKE_DISTCLEAN *= -r $$MOC_DIR

# Additional configuration
#DEFINES *= MY_CODE=1
#DEFINES *= MY_TEXT=\\\"This is my text\\\"
include(compiler.pri) # Compiler configuration

#CONFIG += qt debug
#CONFIG += qt release



#====================================================================
#          PROJECT FILES
#====================================================================

# ---- Project dependencies ----

#LIBS *= -Lsome/path -lsomelib


# ---- Project source files ----

INCLUDEPATH *= .

DEPENDPATH *= .

HEADERS *= \
    A.h \
    B.h \
    KlModel.h \
    KlModelItem.h

SOURCES *= main.cpp \
    KlModel.cpp

#RESOURCES *= qml.qrc
#FORMS *= MyForm.ui
#OTHER_FILES *= readme.txt
