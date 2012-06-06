#include <QtGui>
#include <QtSql>

#include "MemberDetailsDialog.h"


MemberDetailsDialog::MemberDetailsDialog(ZaporraGUI* parentGUI)
	: QDialog(parentGUI), parent(parentGUI)
{
	setWindowTitle("Zaporra - Detalles del socio");

	nameLabel = new QLabel("&Nombre:");
	nameLineEdit = new QLineEdit;
	nameLabel->setBuddy(nameLineEdit);

	surname1Label = new QLabel("&Primer apellido:");
	surname1LineEdit = new QLineEdit;
	surname1Label->setBuddy(surname1LineEdit);

	surname2Label = new QLabel("&Segundo apellido:");
	surname2LineEdit = new QLineEdit;
	surname2Label->setBuddy(surname2LineEdit);

	dniLabel = new QLabel("&DNI:");
	dniLineEdit = new QLineEdit;
	dniLabel->setBuddy(dniLineEdit);

	tlfLabel = new QLabel(QString::fromUtf8("&Teléfono:"));
	tlfLineEdit = new QLineEdit;
	tlfLabel->setBuddy(tlfLineEdit);

	emailLabel = new QLabel(QString::fromUtf8("&Correo electrónico:"));
	emailLineEdit = new QLineEdit;
	emailLabel->setBuddy(emailLineEdit);

	addressLabel = new QLabel(QString::fromUtf8("D&irección:"));
	addressLineEdit = new QLineEdit;
	addressLabel->setBuddy(addressLineEdit);

	cityLabel = new QLabel("&Localidad:");
	cityLineEdit = new QLineEdit;
	cityLabel->setBuddy(cityLineEdit);

	postalCodeLabel = new QLabel(QString::fromUtf8("Código pos&tal:"));
	postalCodeLineEdit = new QLineEdit;
	postalCodeLabel->setBuddy(postalCodeLineEdit);

	provinceLabel = new QLabel("Pro&vincia:");
	provinceLineEdit = new QLineEdit;
	provinceLabel->setBuddy(provinceLineEdit);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QGridLayout* layout = new QGridLayout;
	layout->addWidget(nameLabel, 0, 0);
	layout->addWidget(nameLineEdit, 0, 1);
	layout->addWidget(surname1Label, 1, 0);
	layout->addWidget(surname1LineEdit, 1, 1);
	layout->addWidget(surname2Label, 2, 0);
	layout->addWidget(surname2LineEdit, 2, 1);
	layout->addWidget(dniLabel, 3, 0);
	layout->addWidget(dniLineEdit, 3, 1);
	layout->addWidget(tlfLabel, 4, 0);
	layout->addWidget(tlfLineEdit, 4, 1);
	layout->addWidget(emailLabel, 5, 0);
	layout->addWidget(emailLineEdit, 5, 1);
	layout->addWidget(addressLabel, 6, 0);
	layout->addWidget(addressLineEdit, 6, 1);
	layout->addWidget(cityLabel, 7, 0);
	layout->addWidget(cityLineEdit, 7, 1);
	layout->addWidget(postalCodeLabel, 8, 0);
	layout->addWidget(postalCodeLineEdit, 8, 1);
	layout->addWidget(provinceLabel, 9, 0);
	layout->addWidget(provinceLineEdit, 9, 1);
	layout->addWidget(buttonBox, 10, 1);
	setLayout(layout);
}


void MemberDetailsDialog::accept()
{
	QSqlTableModel* model = parent->getModel();

	if (!mapper->submit()) {
		qDebug() << "Error: QDataWidgetMapper::submit(): "+model->lastError().text();
		QMessageBox::warning(this, "Fallo!", "Error: QDataWidgetMapper::submit(): "+model->lastError().text());
	}
	//if (!model->submitAll()) {
	if (!parent->updateModel()) {
		qDebug() << "Error: ZaporraGUI::updateModel(): "+model->lastError().text();
		QMessageBox::warning(this, "Fallo!", "Error: ZaporraGUI::updateModel(): "+model->lastError().text());
	}

	delete mapper;
	QDialog::accept();
}


void MemberDetailsDialog::reject()
{
	delete mapper;
	QDialog::reject();
}


void MemberDetailsDialog::show(int index)
{
	QSqlTableModel* model = parent->getModel();

	mapper = new QDataWidgetMapper(this);
	mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
	mapper->setModel(model);
	mapper->addMapping(nameLineEdit, model->fieldIndex("name"));
	mapper->addMapping(surname1LineEdit, model->fieldIndex("surname1"));
	mapper->addMapping(surname2LineEdit, model->fieldIndex("surname2"));
	mapper->addMapping(dniLineEdit, model->fieldIndex("id"));
	mapper->addMapping(tlfLineEdit, model->fieldIndex("tel"));
	mapper->addMapping(emailLineEdit, model->fieldIndex("email"));
	mapper->addMapping(addressLineEdit, model->fieldIndex("address"));
	mapper->addMapping(cityLineEdit, model->fieldIndex("city"));
	mapper->addMapping(postalCodeLineEdit, model->fieldIndex("zip"));
	mapper->addMapping(provinceLineEdit, model->fieldIndex("state"));
	mapper->setCurrentIndex(index);

	QDialog::show();
}
