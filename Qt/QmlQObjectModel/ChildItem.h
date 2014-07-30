#ifndef CHILDITEM_H
#define CHILDITEM_H

#include <QtCore>

class ChildItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER m_text NOTIFY textChanged)
    Q_PROPERTY(bool check MEMBER m_check NOTIFY checkChanged)
    Q_PROPERTY(int type MEMBER m_type NOTIFY typeChanged)

public:
    ChildItem(int value = 0, QObject* parent = 0)
        : QObject(parent)
        , m_text(QString("child_%1").arg(value))
        , m_check((value == 1))
        , m_type(value)
    {}

signals:
    void textChanged(const QString& value);
    void checkChanged(bool value);
    void typeChanged(int value);

private:
    QString m_text;
    bool m_check;
    int m_type;
};

#endif // CHILDITEM_H
