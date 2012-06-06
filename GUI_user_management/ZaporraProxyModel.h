#ifndef ZAPORRAPROXYMODEL_H
#define ZAPORRAPROXYMODEL_H

#include <QSortFilterProxyModel>


class ZaporraProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	enum FilterKeyColumns { NameAndSurname, No, DNI };

	ZaporraProxyModel(QObject* parent=0);

	FilterKeyColumns filterKeyColumns() const
	{ return _filterKeyColumns; }

	void setFilterKeyColumns(FilterKeyColumns value)
	{ _filterKeyColumns = value; }


private:
	bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const;

	FilterKeyColumns _filterKeyColumns;
};


#endif // ZAPORRAPROXYMODEL_H
