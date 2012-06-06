#ifndef NewUserWizard_H
#define NewUserWizard_H

#include <QWizard>
class QSqlTableModel;
class QLabel;
class QLineEdit;

#include "ZaporraGUI.h"


class NewUserWizard : public QWizard
{
	Q_OBJECT

public:
	NewUserWizard(ZaporraGUI* parentGUI=0);

	void accept();

private:
	ZaporraGUI* parent;
};


class NewUserPage : public QWizardPage
{
	Q_OBJECT

public:
	NewUserPage(QWidget* parent=0);

private:
	QLabel* nameLabel;
	QLabel* surname1Label;
	QLabel* surname2Label;
	QLabel* dniLabel;
	QLabel* tlfLabel;
	QLabel* emailLabel;
	QLabel* addressLabel;
	QLabel* postalCodeLabel;
	QLabel* cityLabel;
	QLabel* provinceLabel;

	QLineEdit* nameLineEdit;
	QLineEdit* surname1LineEdit;
	QLineEdit* surname2LineEdit;
	QLineEdit* dniLineEdit;
	QLineEdit* tlfLineEdit;
	QLineEdit* emailLineEdit;
	QLineEdit* addressLineEdit;
	QLineEdit* postalCodeLineEdit;
	QLineEdit* cityLineEdit;
	QLineEdit* provinceLineEdit;
};


#endif // NewUserWizard_H
