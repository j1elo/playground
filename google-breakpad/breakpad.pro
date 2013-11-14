#====================================================================
#          PROJECT CONFIGURATION
#====================================================================

message("Processing $${_FILE_}")

CONFIG(debug, debug|release):BUILD_TARGET = debug
CONFIG(release, debug|release):BUILD_TARGET = release
message("Building [$${BUILD_TARGET}] Makefile for [$${TARGET}] on [$${QMAKE_HOST.os}] [$${QMAKE_HOST.arch}]")

# Project configuration and compiler options
#TARGET = target # If not defined: same as the file name
TEMPLATE = app
CONFIG += qt thread
QT += core
QT -= gui

# Build locations
DESTDIR = .
MOC_DIR = tmp
OBJECTS_DIR = $$MOC_DIR
RCC_DIR = $$MOC_DIR
UI_DIR = $$MOC_DIR
unix:QMAKE_DISTCLEAN *= -r $$MOC_DIR symbolstore *.log

# Additional configuration
#DEFINES *= MY_CODE=1
#DEFINES *= MY_TEXT=\\\"This is my text\\\"

*g++* {
    # Enable support for C++11 language revision
    # Google-Breakpad doesn't support C++11 QMAKE_CXXFLAGS *= -std=c++0x

    # Disable some warnings, make all the others into errors
    QMAKE_CXXFLAGS *= -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable
    #QMAKE_CXXFLAGS *= -Werror

    # Disable standard-C assertions
    QMAKE_CFLAGS_RELEASE    *=  -DNDEBUG
    QMAKE_CXXFLAGS_RELEASE  *=  -DNDEBUG

    # Compile with debug symbols
    QMAKE_CFLAGS_RELEASE    *=  -g
    QMAKE_CXXFLAGS_RELEASE  *=  -g
}

win32-msvc* {
    # Disable deprecation of *printf functions
    DEFINES *= _CRT_SECURE_NO_WARNINGS

    # Disable standard-C assertions
    QMAKE_CFLAGS_RELEASE    *=  /DNDEBUG
    QMAKE_CXXFLAGS_RELEASE  *=  /DNDEBUG
}

QMAKE_POST_LINK = breakpad-gensymbols.sh $$TARGET


#====================================================================
#          PROJECT FILES
#====================================================================

# Google Breakpad crash handling
INCLUDEPATH *= /opt/google-breakpad/include
LIBS *= -L/opt/google-breakpad/lib -lbreakpad_client

#DEPENDPATH *= \

HEADERS *= \
    CrashHandler.h

SOURCES *= main.cpp \
    CrashHandler.cpp

#FORMS *= MyForm.ui
#RESOURCES *= resources.qrc

OTHER_FILES *= \
    readme.txt
