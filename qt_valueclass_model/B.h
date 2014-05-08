#ifndef B_H
#define B_H

#include "KlModelItem.h"

#include <QString>

class B : public KlModelItem
{
public:
    B() : KlModelItem() {}

    void setP1(QString p) { m_properties[0] = p; }
    void setP2(QString p) { m_properties[1] = p; }

    virtual QString name()
    { return "{B}"; }

    virtual QString toString()
    {
        QString string;
        string += KlModelItem::toString();
        string += QString("B[%1 %2]").arg(m_properties[0]).arg(m_properties[1]);
        foreach (KlModelItem* item, m_children) {
            string += " | " + item->toString();
        }
        return string;
    }
};

#endif // B_H
