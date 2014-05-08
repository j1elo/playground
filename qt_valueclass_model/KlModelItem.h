#ifndef KLMODELITEM_H
#define KLMODELITEM_H

#include <QList>
#include <QString>
#include <QHash>

class KlModelItem
{
public:
    KlModelItem()
        : m_row(0)
        , m_parent(nullptr)
    {}

    void addChild(KlModelItem* item)
    {
        item->setParent(this);
        m_children.append(item);
    }

    KlModelItem* child(int index)
    {
        if (index >= m_children.size())
            return nullptr;
        else
            return m_children.at(index);
    }

    int row()
    { return m_row; }

    virtual QString name()
    { return "{KliveItem}"; }

    virtual QString toString()
    { return QString("KliveItem[%1 children]").arg(m_children.size()); }

    // Direct getters:

    KlModelItem* parent()
    { return m_parent; }

    QList<KlModelItem*> children()
    { return m_children; }

    QHash<int, QString> properties()
    { return m_properties; }

protected:
    void setParent(KlModelItem* parent)
    { m_parent = parent; }

protected:
    int m_row;
    KlModelItem* m_parent;
    QList<KlModelItem*> m_children;
    QHash<int, QString> m_properties;
};

#endif // KLMODELITEM_H
