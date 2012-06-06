#include <QtGui>
#include <QtSql>

#include "ZaporraGUI.h"
#include "MemberDetailsDialog.h"
#include "NewUserWizard.h"
#include "ZaporraProxyModel.h"


//http://doc.trolltech.com/4.3/demos-books-bookwindow-cpp.html
//http://www.folding-hyperspace.com/program_tip_16.htm

//TODO hacer que el delegate capture el doble click para que se abra un dialog.
//El dialog debe usar QDataWidgetMapper para actualizar los datos.

ZaporraGUI::ZaporraGUI()
{
	setupUi(this);

	initDatabaseModels();

	memberDetailsDialog = new MemberDetailsDialog(this);
	newUserWizard = new NewUserWizard(this);

	//Button connections.
	connect(memberDetailsBtn, SIGNAL(clicked()),
			this, SLOT(showMemberDetails()));
	connect(newUserBtn, SIGNAL(clicked()),
			newUserWizard, SLOT(show()));
	connect(quitBtn, SIGNAL(clicked()),
			this, SLOT(close()));

	//Other connections.
	connect(filterPatternLineEdit, SIGNAL(textChanged(const QString&)),
			proxyModel, SLOT(setFilterWildcard(const QString&)));
	connect(nameAndSurnameRB, SIGNAL(toggled(bool)),
			this, SLOT(filterTypeToggleSLT(bool)));
	connect(noRB, SIGNAL(toggled(bool)),
			this, SLOT(filterTypeToggleSLT(bool)));
	connect(dniRB, SIGNAL(toggled(bool)),
			this, SLOT(filterTypeToggleSLT(bool)));
}


ZaporraGUI::~ZaporraGUI()
{}


bool ZaporraGUI::updateModel()
{
	int selectedRow = 0;
	bool ok;

	if (memberView->selectionModel()->hasSelection()) {
		selectedRow = memberView->selectionModel()->currentIndex().row();
	}
	ok = model->submitAll();
	if (!ok) {
		qDebug() << "Error: QSqlTableModel::submitAll(): "+model->lastError().text();
	}
	memberView->selectRow(selectedRow);

	return ok;
}


void ZaporraGUI::showMemberDetails()
{
	if (memberView->selectionModel()->hasSelection()) {
		//Find the model's equivalent selected row and show its details.
		QModelIndex selectedIndex = memberView->selectionModel()->currentIndex();
		int mappedRow = proxyModel->mapToSource(selectedIndex).row();
		memberDetailsDialog->show(mappedRow);
	}
}


void ZaporraGUI::filterTypeToggleSLT(bool checked)
{
	if (checked) {
		if (nameAndSurnameRB->isChecked()) {
			proxyModel->setFilterKeyColumns(ZaporraProxyModel::NameAndSurname);
		}
		if (noRB->isChecked()) {
			proxyModel->setFilterKeyColumns(ZaporraProxyModel::No);
		}
		if (dniRB->isChecked()) {
			proxyModel->setFilterKeyColumns(ZaporraProxyModel::DNI);
		}
		proxyModel->invalidate();
	}
}


void ZaporraGUI::initDatabaseModels()
{
	//Create the data model.
	model = new QSqlRelationalTableModel(this, QSqlDatabase::database());
	model->setEditStrategy(QSqlTableModel::OnManualSubmit);
	model->setTable("member");

	//Set the relations to the other database tables.
	//model->setRelation(authorIdx, QSqlRelation("authors", "id", "name"));
	//model->setRelation(genreIdx, QSqlRelation("genres", "id", "name"));

	//Set the header captions.
	//model->setHeaderData(genreIdx, Qt::Horizontal, "Genre");
	model->setHeaderData(model->fieldIndex("no"), Qt::Horizontal, QString::fromUtf8("Nº Socio"));
	model->setHeaderData(model->fieldIndex("name"), Qt::Horizontal, "Nombre");
	model->setHeaderData(model->fieldIndex("surname1"), Qt::Horizontal, "Primer apellido");
	model->setHeaderData(model->fieldIndex("surname2"), Qt::Horizontal, "Segundo apellido");
	model->setHeaderData(model->fieldIndex("id"), Qt::Horizontal, "DNI");

	//Populate the model.
	if (!model->select()) {
		showError(model->lastError());
		return;
	}

	//Create the proxy model, (for filtering purposes).
	proxyModel = new ZaporraProxyModel(this);
	proxyModel->setSourceModel(model);
	proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	proxyModel->setDynamicSortFilter(true);

	//Set the view and show only the wanted columns.
	//memberView->setModel(model);
	memberView->setModel(proxyModel);
	memberView->setCurrentIndex(model->index(0, 0));
	for (int i=0; i<model->columnCount(); i++) {
		memberView->hideColumn(i);
	}
	memberView->showColumn(model->fieldIndex("no"));
	memberView->showColumn(model->fieldIndex("name"));
	memberView->showColumn(model->fieldIndex("surname1"));
	memberView->showColumn(model->fieldIndex("surname2"));
	memberView->showColumn(model->fieldIndex("id"));
	memberView->resizeColumnsToContents();
	memberView->sortByColumn(model->fieldIndex("no"), Qt::AscendingOrder);

	memberView->verticalHeader()->hide();
	memberView->setItemDelegate(new QSqlRelationalDelegate(memberView));

//Not needed because QTableView actually implements this.
// 	connect(memberView->horizontalHeader(), SIGNAL(sectionClicked(int)),
// 			memberView, SLOT(sortByColumn(int)));
}


void ZaporraGUI::showError(const QSqlError& err)
{
	QMessageBox::critical(this, "Error accessing database",
		"Error accessing database: " + err.text());
}
