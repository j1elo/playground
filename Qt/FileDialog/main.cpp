#include "FileDialog.h"

#include <QtDebug> // qDebug()
#include <QApplication>
#include <QStandardPaths>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString dialogPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    KShowGUI::FileDialog dialog(0, "Open Image", dialogPath, "Image Files (*.png *.jpg *.bmp *.tga);;Video Files (*.avi *.mp4)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setSidebarUrls(QList<QUrl>()
                          << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::HomeLocation))
                          << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
                          << QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation))
                          << QUrl::fromLocalFile(dialogPath));
    if (QDialog::Accepted == dialog.exec()) {
        QStringList sel = dialog.selectedFiles();
        qDebug() << sel;
    }
    
    return 0;
}
