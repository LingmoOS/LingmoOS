// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "server1.h"
#include "server2.h"

#include <QDBusMessage>

#ifdef NEW_QT
extern Q_DECL_IMPORT void qDBusAddFilterHook(int (*)(const QString &, const QDBusMessage &));
#else
extern Q_DECL_IMPORT void qDBusAddSpyHook(void (*)(const QDBusMessage &));
#endif

bool isAllow(QString callerService)
{
    // Check(callerService); // a specific check
    return false;
}

#ifdef NEW_QT
int MyHook(const QString &baseService, const QDBusMessage &msg)
{
    qInfo() << "[hook]baseService=" << baseService;
#else
void MyHook(const QDBusMessage &msg)
{
#endif

    // demand:
    // 1, The service1 will check, and if the check fails, the call to SetData() will not be allowed
    // 2, The service2 does not check.

    if (msg.member() == "SetData" && msg.interface() == "org.services.demo"
        && msg.path() == "/org/services/demo"
#ifdef NEW_QT
        && baseService
                == QDBusConnection::connectToBus(QDBusConnection::SessionBus, "org.services.demo1")
                           .baseService()
#endif
        // BUG2:In the hook, it is impossible to judge whether the call is service1 or service2
        && !isAllow(msg.service())) {
        QDBusMessage reply = msg.createErrorReply("org.services.error.notallow", "error");
        QDBusConnection::sessionBus().send(reply);
        // Expected results: After the hook end, do not continue to run SetData()
        // Actual results: After the hook end, continue to run SetData()
        // BUG1: If the check fails, the call cannot be aborted
#ifdef NEW_QT
        return -1;
#else
        return;
#endif
    }

    // Expected results: After the hook end, continue to run SetData()
    // Actual results: After the hook end, continue to run SetData()
#ifdef NEW_QT
    return 0;
#else
    return;
#endif
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
#ifdef NEW_QT
    qDBusAddFilterHook(MyHook);
#else
    qDBusAddSpyHook(MyHook);
#endif

    Service1 service1;
    if (!QDBusConnection::connectToBus(QDBusConnection::SessionBus, "org.services.demo1")
                 .registerObject("/org/services/demo",
                                 &service1,
                                 QDBusConnection::ExportAllSlots)) {
        return -1;
    }
    if (!QDBusConnection::connectToBus(QDBusConnection::SessionBus, "org.services.demo1")
                 .registerService("org.services.demo1")) {
        return -1;
    }
    Service2 service2;
    if (!QDBusConnection::connectToBus(QDBusConnection::SessionBus, "org.services.demo2")
                 .registerObject("/org/services/demo",
                                 &service2,
                                 QDBusConnection::ExportAllSlots)) {
        return -1;
    }
    if (!QDBusConnection::connectToBus(QDBusConnection::SessionBus, "org.services.demo2")
                 .registerService("org.services.demo2")) {
        return -1;
    }
    qInfo() << QDBusConnection::connectToBus(QDBusConnection::SessionBus, "org.services.demo1")
                       .baseService();
    qInfo() << QDBusConnection::connectToBus(QDBusConnection::SessionBus, "org.services.demo2")
                       .baseService();
    return a.exec();
}