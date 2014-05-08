#include "KlModel.h"
#include "KlModelItem.h"

#include <QHash>
#include <QString>

KlModel::KlModel(QObject* parent)
    : QAbstractItemModel(parent)
    , m_rootItem(new KlModelItem())
{}

KlModel::~KlModel()
{}

void KlModel::addItem(KlModelItem* item)
{
    m_rootItem->addChild(item);
}

QModelIndex KlModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    KlModelItem* parentItem;
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<KlModelItem*>(parent.internalPointer());

    KlModelItem* childItem = parentItem->child(row);
    if (!childItem)
        return QModelIndex();
    else
        return createIndex(row, column, childItem);
}

QModelIndex KlModel::parent(const QModelIndex& child) const
{
    if (!child.isValid())
        return QModelIndex();

    KlModelItem* childItem = static_cast<KlModelItem*>(child.internalPointer());
    KlModelItem* parentItem = childItem->parent();
    if (!parentItem || parentItem == m_rootItem)
        return QModelIndex();
    else
        return createIndex(parentItem->row(), 0, parentItem);
}

int KlModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;

    KlModelItem* parentItem;
    if (!parent.isValid())
        parentItem = m_rootItem;
    else
        parentItem = static_cast<KlModelItem*>(parent.internalPointer());

    return parentItem->children().size();
}

int KlModel::columnCount(const QModelIndex& parent) const
{
    return 3;
}

QVariant KlModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    KlModelItem* item = static_cast<KlModelItem*>(index.internalPointer());

    switch (index.column()) {
    case 0:
        return item->name();
    case 1:
        return item->properties().value(0);
    case 2:
        return item->properties().value(1);
    default:
        return QVariant();
    }
}

Qt::ItemFlags KlModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}
