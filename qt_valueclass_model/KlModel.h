#ifndef KLMODEL_H
#define KLMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class KlModelItem;

class KlModel : public QAbstractItemModel
{
    Q_OBJECT

public:
//    Qt::ItemFlags QAbstractItemModel::flags(const QModelIndex & index) const [virtual];
//    bool QAbstractItemModel::setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) [virtual];
//    bool QAbstractItemModel::insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) [virtual];
//    bool QAbstractItemModel::removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) [virtual];
//    bool QAbstractItemModel::insertColumns(int column, int count, const QModelIndex & parent = QModelIndex()) [virtual];
//    bool QAbstractItemModel::removeColumns(int column, int count, const QModelIndex & parent = QModelIndex()) [virtual];
//    bool QAbstractItemModel::hasChildren(const QModelIndex & parent = QModelIndex()) const [virtual];

    explicit KlModel(QObject* parent = 0);
    virtual ~KlModel();

    void addItem(KlModelItem* item);

    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;

//    virtual QModelIndex sibling(int row, int column, const QModelIndex &idx) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
//    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
//    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

//    virtual QVariant headerData(int section, Qt::Orientation orientation,
//                                int role = Qt::DisplayRole) const;
//    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value,
//                               int role = Qt::EditRole);

//    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
//    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);

//    virtual QStringList mimeTypes() const;
//    virtual QMimeData *mimeData(const QModelIndexList &indexes) const;
//    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action,
//                                 int row, int column, const QModelIndex &parent) const;
//    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
//                              int row, int column, const QModelIndex &parent);
//    virtual Qt::DropActions supportedDropActions() const;

//    virtual Qt::DropActions supportedDragActions() const;
//#if QT_DEPRECATED_SINCE(5, 0)
//    QT_DEPRECATED void setSupportedDragActions(Qt::DropActions actions)
//    { doSetSupportedDragActions(actions); }
//#endif

//    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
//    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
//    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
//    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());
//    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
//                          const QModelIndex &destinationParent, int destinationChild);
//    virtual bool moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count,
//                             const QModelIndex &destinationParent, int destinationChild);

//    inline bool insertRow(int row, const QModelIndex &parent = QModelIndex());
//    inline bool insertColumn(int column, const QModelIndex &parent = QModelIndex());
//    inline bool removeRow(int row, const QModelIndex &parent = QModelIndex());
//    inline bool removeColumn(int column, const QModelIndex &parent = QModelIndex());
//    inline bool moveRow(const QModelIndex &sourceParent, int sourceRow,
//                        const QModelIndex &destinationParent, int destinationChild);
//    inline bool moveColumn(const QModelIndex &sourceParent, int sourceColumn,
//                           const QModelIndex &destinationParent, int destinationChild);

//    virtual void fetchMore(const QModelIndex &parent);
//    virtual bool canFetchMore(const QModelIndex &parent) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
//    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
//    virtual QModelIndex buddy(const QModelIndex &index) const;
//    virtual QModelIndexList match(const QModelIndex &start, int role,
//                                  const QVariant &value, int hits = 1,
//                                  Qt::MatchFlags flags =
//                                  Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const;
//    virtual QSize span(const QModelIndex &index) const;

//    virtual QHash<int,QByteArray> roleNames() const;

private:
    KlModelItem* m_rootItem;
};

#endif // KLMODEL_H
