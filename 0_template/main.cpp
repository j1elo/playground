#include "MyClass.h"

#include <QtDebug> // qDebug()
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    MyClass window;
    window.show();

    return app.exec();
}
