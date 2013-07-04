#include "qtquick2applicationviewer.h"
#include <QtGui/QGuiApplication>
#include <QUrl>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QtQuick2ApplicationViewer viewer;
    //viewer.setMainQmlFile(QStringLiteral("qml/GraphEd/main.qml"));
    viewer.setSource(QUrl("qrc:/qml/GraphEd/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
