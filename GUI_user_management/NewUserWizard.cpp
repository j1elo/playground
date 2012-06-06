#include <QtGui>
#include <QtSql>

#include "NewUserWizard.h"


NewUserWizard::NewUserWizard(ZaporraGUI* parentGUI)
	: QWizard(parentGUI), parent(parentGUI)
{
	setWindowTitle("Zaporra - Nuevo socio");

// 	setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
// 	setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));

	setOption(QWizard::HaveHelpButton, false);
	setOption(QWizard::HaveNextButtonOnLastPage, false);
	setOption(QWizard::NoBackButtonOnLastPage, true);
	addPage(new NewUserPage);
}


void NewUserWizard::accept()
{
	QSqlTableModel* model = parent->getModel();

	//QSqlRecord record;
	QSqlRecord record = model->record();
	record.setValue("name", field("name").toString());
	record.setValue("surname1", field("surname1").toString());
	record.setValue("surname2", field("surname2").toString());
	record.setValue("id", field("dni").toInt());
	record.setValue("tel", field("tlf").toInt());
	record.setValue("email", field("email").toString());
	record.setValue("address", field("address").toString());
	record.setValue("city", field("city").toString());
	record.setValue("zip", field("postalCode").toInt());
	record.setValue("state", field("province").toString());
	//record.setValue("suscribed", 248);
	//record.setValue("unsuscribed", 248);

	if (!model->insertRecord(-1, record)) {
		qDebug() << "Error: QSqlTableModel::insertRecord(): "+model->lastError().text();
		QMessageBox::warning(this, "Fallo!", "Error: QSqlTableModel::insertRecord(): "+model->lastError().text());
	}
	//if (!model->submitAll()) {
	if (!parent->updateModel()) {
		qDebug() << "Error: ZaporraGUI::updateModel(): "+model->lastError().text();
		QMessageBox::warning(this, "Fallo!", "Error: ZaporraGUI::updateModel(): "+model->lastError().text());
	}

	cleanupPage(0);
	QDialog::accept();
}


NewUserPage::NewUserPage(QWidget* parent)
	: QWizardPage(parent)
{
	setTitle(QString::fromUtf8("Inserción de un nuevo socio"));
	setSubTitle("Especifica por favor los datos personales del nuevo socio.\n"
			"Los campos marcados con (*) son obligatorios.");

	nameLabel = new QLabel("&Nombre (*):");
	nameLineEdit = new QLineEdit;
	nameLabel->setBuddy(nameLineEdit);

	surname1Label = new QLabel("&Primer apellido (*):");
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

	addressLabel = new QLabel(QString::fromUtf8("D&irección (*):"));
	addressLineEdit = new QLineEdit;
	addressLabel->setBuddy(addressLineEdit);

	cityLabel = new QLabel("&Localidad (*):");
	cityLineEdit = new QLineEdit;
	cityLabel->setBuddy(cityLineEdit);

	postalCodeLabel = new QLabel(QString::fromUtf8("Código pos&tal:"));
	postalCodeLineEdit = new QLineEdit;
	postalCodeLabel->setBuddy(postalCodeLineEdit);

	provinceLabel = new QLabel("Pro&vincia:");
	provinceLineEdit = new QLineEdit;
	provinceLabel->setBuddy(provinceLineEdit);


	registerField("name*", nameLineEdit);
	registerField("surname1*", surname1LineEdit);
	registerField("surname2", surname2LineEdit);
	registerField("dni", dniLineEdit);
	registerField("tlf", tlfLineEdit);
	registerField("email", emailLineEdit);
	registerField("address*", addressLineEdit);
	registerField("city*", cityLineEdit);
	registerField("postalCode", postalCodeLineEdit);
	registerField("province", provinceLineEdit);

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
	setLayout(layout);
}
