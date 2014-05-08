#include "A.h"
#include "B.h"
#include "KlModel.h"

#include <QtDebug>
#include <QApplication>
#include <QTreeView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    A* a1 = new A();
    a1->setP1("a1-p1");
    a1->setP2("a1-p2");

    B* b1 = new B();
    b1->setP1("b1-p1");
    b1->setP2("b1-p2");
    a1->addChild(b1);

    B* b2 = new B();
    b2->setP1("b2-p1");
    b2->setP2("b2-p2");
    a1->addChild(b2);

    qDebug() << "String:" << a1->toString();

    KlModel* model = new KlModel();
    model->addItem(a1);

    QTreeView* view = new QTreeView();
    view->setModel(model);
    view->show();

    return app.exec();



    // Columns
    // 0: UID; 1: Title; 2: Year

//    QStandardItemModel model;
//    model.setColumnCount(2);

//    QStandardItem* root = model.invisibleRootItem();
//    QStandardItem *item1, *item2;

//    item1 = new QStandardItem("UID");
//    item2 = new QStandardItem("15646");
//    root->appendRow(QList<QStandardItem*>() << item1 << item2);

//    item1 = new QStandardItem("Title");
//    item2 = new QStandardItem("My Item");
//    root->appendRow(QList<QStandardItem*>() << item1 << item2);

//    QTreeView *tree = new QTreeView();
//    tree->setModel(&model);
//    tree->show();

    //return app.exec();
}
