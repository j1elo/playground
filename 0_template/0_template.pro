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
QT *= core gui
greaterThan(QT_MAJOR_VERSION, 4): QT *= widgets

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

*g++* {
    # Enable support for C++11 language revision
    QMAKE_CXXFLAGS *= -std=c++0x

    # Disable some warnings, make all the others into errors
    QMAKE_CXXFLAGS *= -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable
    #QMAKE_CXXFLAGS *= -Werror

    # Disable code assertions
    #QMAKE_CFLAGS_RELEASE   *= -DNDEBUG
    #QMAKE_CXXFLAGS_RELEASE *= -DNDEBUG
}

win32-msvc* {
    # Disable deprecation of *printf functions
    DEFINES *= _CRT_SECURE_NO_WARNINGS

    # Disable code assertions
    #QMAKE_CFLAGS_RELEASE   *= /DNDEBUG
    #QMAKE_CXXFLAGS_RELEASE *= /DNDEBUG
}



#====================================================================
#          PROJECT FILES
#====================================================================

# ---- Project dependencies ----

#LIBS *= -Lsome/path -lsomelib


# ---- Project source files ----

INCLUDEPATH *= .

DEPENDPATH *= .

HEADERS *= \
    MyWidget.h

SOURCES *= main.cpp \
    MyWidget.cpp

#FORMS *= MyForm.ui
#RESOURCES *= resources.qrc
#OTHER_FILES *= readme.txt
