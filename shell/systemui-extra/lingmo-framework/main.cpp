#include <QCoreApplication>
#include <QtDBus/QDBusConnection>
#include <QtDebug>

#include "screen.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // Todo
    // 启动后台服务，加载Dbus接口,注册DBus对象
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService(QStringLiteral("org.lingmo.framework"))) {
        sessionBus.registerObject("/device/screen", new Screen(),
                                 QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals
                                 );
    }


    return a.exec();
}
