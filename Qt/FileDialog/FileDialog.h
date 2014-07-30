#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QFileDialog>
#include <QSortFilterProxyModel>

namespace KShowGUI
{

class FileDialog : public QFileDialog
{
    Q_OBJECT

public:
    FileDialog(QWidget* parent, Qt::WindowFlags f);
    explicit FileDialog(QWidget* parent = 0,
                        const QString& caption = QString(),
                        const QString& directory = QString(),
                        const QString& filter = QString());

private:
    void init();
};

class FileDialogProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FileDialogProxyModel(QObject* parent = 0);

protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;
    virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
};

} // namespace KShowGUI

#endif // FILEDIALOG_H
