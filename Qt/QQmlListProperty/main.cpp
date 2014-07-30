#include <QGuiApplication>
#include <QtQuick>
#include <QtQml> // qmlRegisterType()

class Data : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)

public:
    Data(const QString& value) : m_value(value) {}

    QString value() const { return m_value; }
    void setValue(const QString& value) {
        if (value != m_value) {
            m_value = value;
            emit valueChanged(value);
        }
    }

signals:
    void valueChanged(const QString& value);

private:
    QString m_value;
};

class DataList : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<Data> list READ list NOTIFY listChanged)

public:
    DataList() {
        for (int i = 0; i < 5; ++i) {
            m_list.append(new Data(QString("name %1").arg(i)));
        }
        startTimer(5000);
    }

    QQmlListProperty<Data> list() {
        return QQmlListProperty<Data>(
                    this, NULL,
                    &myAppend, &myCount,
                    &myAt, &myClear);
    }

    virtual void timerEvent(QTimerEvent*) {
        m_list[1]->setValue(m_list[1]->value() + "C++");
    }

signals:
    void listChanged();

private:
    static void myAppend(QQmlListProperty<Data>* list, Data* item)
    {
        DataList* dataList = qobject_cast<DataList*>(list->object);
        if (dataList && item) {
            dataList->m_list.append(item);
            emit dataList->listChanged();
        }
    }

    static int myCount(QQmlListProperty<Data>* list)
    {
        DataList* dataList = qobject_cast<DataList*>(list->object);
        if (dataList) {
            return dataList->m_list.count();
        }
        return 0;
    }

    static Data* myAt(QQmlListProperty<Data>* list, int i)
    {
        DataList* dataList = qobject_cast<DataList*>(list->object);
        if (dataList) {
            return dataList->m_list.at(i);
        }
        return NULL;
    }

    static void myClear(QQmlListProperty<Data>* list)
    {
        DataList* dataList = qobject_cast<DataList*>(list->object);
        if (dataList) {
            dataList->m_list.clear();
            emit dataList->listChanged();
        }
    }

    QList<Data*> m_list;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<Data>();

    QQuickView view;
    view.rootContext()->setContextProperty("dataList", new DataList());
    view.setSource(QUrl::fromLocalFile("main.qml"));
    view.show();

    return app.exec();
}

#include "main.moc"
