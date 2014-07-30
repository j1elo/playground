#include "MyWidget.h"

#include <QtDebug> // qDebug()

MyWidget::MyWidget(QWidget* parent)
    : QWidget(parent)
{
    qDebug() << "[MyWidget::MyWidget]";
}

MyWidget::~MyWidget()
{
    qDebug() << "[MyWidget::~MyWidget]";
}
