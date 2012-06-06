TEMPLATE = app
TARGET =

CONFIG += qt debug
QT += sql

#DEFINES += DB_MYSQL
DEFINES += DB_SQLITE

UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp

DEPENDPATH +=
INCLUDEPATH +=

HEADERS += ZaporraGUI.h \
		MemberDetailsDialog.h \
		NewUserWizard.h \
		ZaporraProxyModel.h

SOURCES += ZaporraGUI.cpp \
		MemberDetailsDialog.cpp \
		NewUserWizard.cpp \
		ZaporraProxyModel.cpp \
		main.cpp

FORMS += MainWindow.ui
