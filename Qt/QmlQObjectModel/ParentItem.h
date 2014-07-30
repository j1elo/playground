#ifndef PARENTITEM_H
#define PARENTITEM_H

#include "ChildItem.h"
#include <QtCore>

class ParentItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER m_text NOTIFY textChanged)
    Q_PROPERTY(bool check MEMBER m_check NOTIFY checkChanged)
    Q_PROPERTY(int type MEMBER m_type NOTIFY typeChanged)
    Q_PROPERTY(ChildItem* child MEMBER m_child NOTIFY childChanged)

public:
    ParentItem(int value = 0, QObject* parent = 0)
        : QObject(parent)
        , m_text(QString("parent_%1").arg(value))
        , m_check((value == 1))
        , m_type(value)
        , m_child(new ChildItem(value, this))
    {}

signals:
    void textChanged(const QString& value);
    void checkChanged(bool value);
    void typeChanged(int value);
    void childChanged(ChildItem* value);

private:
    QString m_text;
    bool m_check;
    int m_type;
    ChildItem* m_child;
};

#endif // PARENTITEM_H
