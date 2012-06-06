#include <QtGui>
#include <QtSql>
#include <QLibraryInfo>
#include "ZaporraGUI.h"


void printQtInfo()
{
	qDebug() << "----- Qt library information -----";
	qDebug() << "-> Qt build identification key: " << QLibraryInfo::buildKey();
	qDebug() << "-> Qt licensed person:          " << QLibraryInfo::licensee();
	qDebug() << "-> Qt licensed products:        " << QLibraryInfo::licensedProducts();
	qDebug() << "-> Path to the Qt components:";
	qDebug() << "  Default prefix:   " << QLibraryInfo::location(QLibraryInfo::PrefixPath);
	qDebug() << "  Documentation:    " << QLibraryInfo::location(QLibraryInfo::DocumentationPath);
	qDebug() << "  Headers:          " << QLibraryInfo::location(QLibraryInfo::HeadersPath);
	qDebug() << "  Libraries:        " << QLibraryInfo::location(QLibraryInfo::LibrariesPath);
	qDebug() << "  Binaries:         " << QLibraryInfo::location(QLibraryInfo::BinariesPath);
	qDebug() << "  Plugins:          " << QLibraryInfo::location(QLibraryInfo::PluginsPath);
	qDebug() << "  General data:     " << QLibraryInfo::location(QLibraryInfo::DataPath);
	qDebug() << "  Translation data: " << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
	qDebug() << "  Settings:         " << QLibraryInfo::location(QLibraryInfo::SettingsPath);
	qDebug() << "  Examples:         " << QLibraryInfo::location(QLibraryInfo::ExamplesPath);
	qDebug() << "  Demos:            " << QLibraryInfo::location(QLibraryInfo::DemosPath);
}


bool createDefaultConnection(const QString& driver)
{
	if (!QSqlDatabase::drivers().contains(driver)) {
		qDebug() << "SQL ERROR: driver not available";
		QMessageBox::critical(0,
			"Error al cargar la base de datos",
			"No se encuentra el driver "+driver,
			QMessageBox::Cancel);
		return false;
	}

	QSqlDatabase db = QSqlDatabase::addDatabase(driver);
	db.setHostName("localhost");

	if (driver == "QMYSQL") {
		db.setDatabaseName("zaporra");
		db.setUserName("zaporra");
		//db.setPassword("z0112358");
		db.setPassword("z");
	}
	else if (driver == "QSQLITE") {
		db.setDatabaseName("zaporra.db");
	}

	if (!db.open()) {
		QString errorMsg = db.lastError().text();
		qDebug() << "SQL ERROR:" << errorMsg;
		QMessageBox::critical(0, "Error abriendo la base de datos",
			QString::fromUtf8("No se ha podido conectar a la base de datos.\n"
			"Si el problema persiste, por favor avise al servicio de soporte técnico\n"
			"y adjunte la siguiente información:\n\n")+errorMsg+"\n\n"
			"Haga click en Cancelar para salir.",
			QMessageBox::Cancel);
		return false;
	}
	return true;
}


int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	bool ok;

#ifdef DB_MYSQL
	ok = createDefaultConnection("QMYSQL");
#elif defined(DB_SQLITE)
	ok = createDefaultConnection("QSQLITE");
#else
#error You must define DB_MYSQL or DB_SQLITE
#endif

	if (!ok) {
		return -1;
	}

	ZaporraGUI gui;
	gui.show();
	app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

	return app.exec();
}
