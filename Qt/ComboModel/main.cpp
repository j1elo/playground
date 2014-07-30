#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStringListModel>

class Generator : public QObject
{
    Q_OBJECT

public:
    Generator(QStringListModel* model)
        : m_model(model)
    {}

    Q_INVOKABLE void generate(const QVariant& val)
    {
        QStringList list;
        for (int i = 1; i <= 3; ++i) {
            list << QString("%1:%2").arg(val.toString()).arg(i);
        }
        m_model->setStringList(list);
    }

private:
    QStringListModel* m_model;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QStringListModel model1, model2;
    Generator generator(&model2);

    QStringList list;
    list << "one" << "two" << "three" << "four";
    model1.setStringList(list);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("model1", &model1);
    engine.rootContext()->setContextProperty("model2", &model2);
    engine.rootContext()->setContextProperty("generator", &generator);
    engine.load(QUrl("main.qml"));

    return app.exec();
}

#include "main.moc"
