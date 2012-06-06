#ifndef MemberDetailsDialog_H
#define MemberDetailsDialog_H

#include <QDialog>
class QSqlTableModel;
class QDataWidgetMapper;
class QLabel;
class QLineEdit;
class QDialogButtonBox;

#include "ZaporraGUI.h"


class MemberDetailsDialog : public QDialog
{
	Q_OBJECT

public:
	MemberDetailsDialog(ZaporraGUI* parentGUI=0);

public slots:
	void accept();
	void reject();
	void show(int index);

private:
	ZaporraGUI* parent;
	QDataWidgetMapper* mapper;

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

	QDialogButtonBox* buttonBox;
};


#endif // MemberDetailsDialog_H
