#ifndef ZAPORRAGUI_H
#define ZAPORRAGUI_H

#include <QMainWindow>
#include <QSqlError>
#include "ui_MainWindow.h"
class MemberDetailsDialog;
class NewUserWizard;
class QSqlTableModel;
class QSqlRelationalTableModel;
class ZaporraProxyModel;


class ZaporraGUI
	: public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT

public:
	ZaporraGUI();
	~ZaporraGUI();

	QSqlTableModel* getModel();

public slots:
	bool updateModel();

private slots:
	void showMemberDetails();
	void filterTypeToggleSLT(bool checked);

private:
	void initDatabaseModels();
	void showError(const QSqlError& err);

	MemberDetailsDialog* memberDetailsDialog;
	NewUserWizard* newUserWizard;

	QSqlRelationalTableModel *model;
	ZaporraProxyModel* proxyModel;
};

inline QSqlTableModel* ZaporraGUI::getModel()
{ return (QSqlTableModel*)model; }


#endif // ZAPORRAGUI_H
