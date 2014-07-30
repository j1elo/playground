#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ParentItem.h"
#include "ChildItem.h"

#include <QtCore>

class MainWindow : public QObject
{
    Q_OBJECT

public:
    MainWindow(QObject* parent = 0)
        : QObject(parent)
        , m_root(NULL)
    {
        m_dataItems << new ParentItem(0, this);
        m_dataItems << new ParentItem(1, this);
        m_dataItems << new ParentItem(2, this);
    }

    void setRootObject(QObject* object)
    {
        m_root = object;
        if (m_root) {
            connect(m_root, SIGNAL(buttonClicked(int)), this, SLOT(setItem(int)));
        }
    }

private slots:
    void setItem(int index)
    {
        if (m_root) {
            m_root->setProperty("dataItem", QVariant::fromValue(m_dataItems.at(index)));
        }
    }

private:
    QObject* m_root;
    //QList<QObject*> m_dataItems;
    QList<ParentItem*> m_dataItems;
};

#endif // MAINWINDOW_H
