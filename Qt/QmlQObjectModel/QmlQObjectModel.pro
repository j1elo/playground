#====================================================================
#          PROJECT CONFIGURATION
#====================================================================

# Build configuration
TEMPLATE = app
QT *= qml quick widgets

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
    MainWindow.h \
    ParentItem.h \
    ChildItem.h

SOURCES *= main.cpp

OTHER_FILES *= main.qml
