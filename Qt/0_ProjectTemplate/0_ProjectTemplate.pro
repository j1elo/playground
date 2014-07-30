#====================================================================
#          PROJECT CONFIGURATION
#====================================================================

# Build configuration
#TARGET = target # If not defined: same as the file name
TEMPLATE = app
#CONFIG *= # By default: qt thread
#QT *= # By default: core gui
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
include(compiler.pri) # Compiler configuration



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
