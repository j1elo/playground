#include "FileDialog.h"

#include <QtDebug>
#include <QModelIndex>
#include <QFileSystemModel>
#include <QString>
#include <QStringBuilder> // Efficient string construction
#include <QFile>
#include <QDir>
#include <QDateTime>

namespace KShowGUI
{

FileDialog::FileDialog(QWidget* parent, Qt::WindowFlags f)
    : QFileDialog(parent, f)
{
    init();
}

FileDialog::FileDialog(QWidget* parent,
                       const QString& caption,
                       const QString& directory,
                       const QString& filter)
    : QFileDialog(parent, caption, directory, filter)
{
    init();
}

void FileDialog::init()
{
    // If our device has /media/data folder we stay inside it
    if (QFile::exists("/media/data")) {
        QString dialogPath = directory().absolutePath();
        if (dialogPath != "/media" && !dialogPath.startsWith("/media/")) {
            setDirectory("/media");
        }
    }

    setOptions(QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);

    /*
     * Enable both modes, workaround a bug in Qt which makes it crash
     * if it is initialized as List and later the user tries to change
     * to Detail.
     */
    setViewMode(QFileDialog::Detail);
    setViewMode(QFileDialog::List);

    KShowGUI::FileDialogProxyModel* proxyModel = new KShowGUI::FileDialogProxyModel(this);
    //proxyModel->setSortCaseSensitivity(false);
    proxyModel->setSortLocaleAware(true);
    setProxyModel(proxyModel);

    // In Linux, the WindowFlags for custom title buttons are broken and don't work,
    // so we explicitly limit the max size of the dialog.
    setMaximumSize(sizeHint());
    setMaximumWidth(maximumWidth() + 400); // Allow to grow a bit
    setMaximumHeight(maximumHeight() + 400);
}

FileDialogProxyModel::FileDialogProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{}

bool FileDialogProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QFileSystemModel* fileModel = qobject_cast<QFileSystemModel*>(sourceModel());
    Q_ASSERT_X(0 != fileModel,
               "[FileFilterProxyModel::filterAcceptsRow]",
               "Model is not a QFileSystemModel");

    // Filter by the first column, which is the "Name" of the files
    QString	filePath = fileModel->filePath(fileModel->index(source_row, 0, source_parent));
    //qDebug() << "DEBUG" << "[FileDialog::filter]" << "filePath:" << filePath;

    /*
     * Show all dirs except project media dir, to avoid reading
     * from the same directory where medias will be written.
     *
     * FIXME: this is wrong here! the task of checking that a file is not being written
     * over itself should be done by the copying routine, not by the file system models.
     * Just like a file explorer lets you copy files bout not paste over themselves.
     */
//    QString mediaPath = QString("%1/%2").arg(IPROJECT.getDataProperty("projectDir")).arg(KShowInterface::MEDIA_SUBDIR);
//    qDebug() << "DEBUG" << "[FileDialog::filter]" << "mediaPath:" << mediaPath;
//    if (filePath.startsWith(mediaPath))
//        return false;

    // If our device has /media/data folder we only show this dir
    if (QFile::exists("/media/data")) {
        return (filePath == "/"
                || filePath == "/media"
                || filePath.startsWith("/media/"));
    }

    return true;
}

bool FileDialogProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    const QFileSystemModel* fileModel = qobject_cast<const QFileSystemModel*>(left.model());
    Q_ASSERT_X(0 != fileModel,
               "[FileFilterProxyModel::lessThan]",
               "Model is not a QFileSystemModel");

    // Based on QFileSystemModel::data (qfilesystemmodel.cpp)
    // and QSortFilterProxyModel::lessThan (qsortfilterproxymodel.cpp)
    switch (left.column()) {
    case 0:
    case 2:
        if (isSortLocaleAware())
            return QString::localeAwareCompare(fileModel->fileName(left), fileModel->fileName(right)) < 0;
        else
            return QString::compare(fileModel->fileName(left), fileModel->fileName(right), sortCaseSensitivity()) < 0;
    case 1: return fileModel->size(left) < fileModel->size(right);
    case 3: return fileModel->lastModified(left) < fileModel->lastModified(right);
    default:
        qWarning() << "WARNING" << "[FileDialog::sort]" << "invalid display value column:" << left.column();
        break;
    }
    return false;
}

} // namespace KShowGUI
