#ifndef A_H
#define A_H

#include "KlModelItem.h"

#include <QString>

class A : public KlModelItem
{
public:
    A() : KlModelItem() {}

    void setP1(QString p) { m_properties[0] = p; }
    void setP2(QString p) { m_properties[1] = p; }

    virtual QString name()
    { return "{A}"; }

    virtual QString toString()
    {
        QString string;
        string += KlModelItem::toString();
        string += QString("A[%1 %2]").arg(m_properties[0]).arg(m_properties[1]);
        foreach (KlModelItem* item, m_children) {
            string += " | " + item->toString();
        }
        return string;
    }
};

#endif // A_H
