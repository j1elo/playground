#include <QSqlTableModel>

#include "ZaporraProxyModel.h"


ZaporraProxyModel::ZaporraProxyModel(QObject* parent)
	: QSortFilterProxyModel(parent),
	_filterKeyColumns(NameAndSurname)
{}


bool ZaporraProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	switch (_filterKeyColumns) {
		case NameAndSurname: {
			QModelIndex index0 = sourceModel()->index(sourceRow,
				((QSqlTableModel*)sourceModel())->fieldIndex("name"), sourceParent);
			QModelIndex index1 = sourceModel()->index(sourceRow,
				((QSqlTableModel*)sourceModel())->fieldIndex("surname1"), sourceParent);
			QModelIndex index2 = sourceModel()->index(sourceRow,
				((QSqlTableModel*)sourceModel())->fieldIndex("surname2"), sourceParent);
			return (sourceModel()->data(index0).toString().contains(filterRegExp())
					|| sourceModel()->data(index1).toString().contains(filterRegExp())
					|| sourceModel()->data(index2).toString().contains(filterRegExp()));
		}
			break;

		case No: {
			QModelIndex index0 = sourceModel()->index(sourceRow,
				((QSqlTableModel*)sourceModel())->fieldIndex("no"), sourceParent);
			return sourceModel()->data(index0).toString().contains(filterRegExp());
		}
			break;

		case DNI: {
			QModelIndex index0 = sourceModel()->index(sourceRow,
				((QSqlTableModel*)sourceModel())->fieldIndex("id"), sourceParent);
			return sourceModel()->data(index0).toString().contains(filterRegExp());
		}
			break;

		default:
			return true;
			break;
	}
}
