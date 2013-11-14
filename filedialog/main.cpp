#include "FileDialog.h"

#include <QtDebug> // qDebug()
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString dialogPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    KShowGUI::FileDialog dialog(0, "Open Image", dialogPath, "Image Files (*.png *.jpg *.bmp *.tga);;Video Files (*.avi *.mp4)");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setSidebarUrls(QList<QUrl>()
                          << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::HomeLocation))
                          << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation))
                          << QUrl::fromLocalFile(QDesktopServices::storageLocation(QDesktopServices::MoviesLocation))
                          << QUrl::fromLocalFile(dialogPath));
    if (QDialog::Accepted == dialog.exec()) {
        QStringList sel = dialog.selectedFiles();
        qDebug() << sel;
    }
    
//    MyClass window;
//    window.show();

    //return app.exec();
    return 0;
}
