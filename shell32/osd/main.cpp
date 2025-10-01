#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "MouseHelper.h"
#include "VolumeController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    MouseHelper mouseHelper;
    VolumeController controller;

    QQmlApplicationEngine engine;
    qmlRegisterType<VolumeController>("App", 1, 0, "VolumeController");
    engine.rootContext()->setContextProperty("MouseHelper", &mouseHelper);
    engine.rootContext()->setContextProperty("VolumeController", &controller);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}