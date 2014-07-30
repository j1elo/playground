#include "MainWindow.h"

#include <QtQml>
#include <QApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qmlRegisterType<ParentItem>("MyApp.DataItems", 1, 0, "ParentItem");
    qmlRegisterType<ChildItem>( "MyApp.DataItems", 1, 0, "ChildItem");

    QQmlApplicationEngine engine;
    engine.load(QUrl::fromLocalFile("main.qml"));
    Q_ASSERT(engine.rootObjects().size() > 0);
    QObject* childQML = engine.rootObjects().at(0);
    Q_ASSERT(childQML);

    MainWindow window;
    window.setRootObject(childQML);

    return app.exec();
}
