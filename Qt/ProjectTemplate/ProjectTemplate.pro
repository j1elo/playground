#====================================================================
#          PROJECT CONFIGURATION
#====================================================================

# Build configuration
#TARGET = target # If not defined: same name as this file
TEMPLATE = app
#CONFIG += # By default: qt thread
#QT += # By default: core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Build locations
DESTDIR = bin
MOC_DIR = tmp
OBJECTS_DIR = $$MOC_DIR
RCC_DIR = $$MOC_DIR
UI_DIR = $$MOC_DIR
unix: QMAKE_DISTCLEAN += -r $$MOC_DIR

# Additional configuration
#DEFINES += MY_CODE=1
#DEFINES += MY_TEXT=\\\"Dummy text\\\"
include(compiler.pri) # Compiler configuration



#====================================================================
#          PROJECT FILES
#====================================================================

# ---- Project dependencies ----

#LIBS += -Lsome/path -lsomelib


# ---- Project source files ----

INCLUDEPATH += .

DEPENDPATH += .

SOURCES += src/main.cpp \
    src/MainWindow.cpp

HEADERS += \
    src/MainWindow.h

FORMS += src/MainWindow.ui
#RESOURCES += images.qrc
#OTHER_FILES += readme.txt
