// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controlinterface.h"
#include "DDLog.h"

#include <QCoreApplication>
#include <QLoggingCategory>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <DLog>

using namespace DDLog;
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    #if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
        Dtk::Core::DLogManager::registerJournalAppender();
    #else
        Dtk::Core::DLogManager::registerFileAppender();
    #endif
    #ifdef QT_DEBUG
        Dtk::Core::DLogManager::registerConsoleAppender();
    #endif
    QCoreApplication a(argc, argv);

    ControlInterface controlInterface;
    QDBusConnection connection = controlInterface.qDbusConnection();
    if (!connection.registerObject("/org/deepin/DeviceControl", &controlInterface,
                                   QDBusConnection::ExportScriptableContents)) {
        qCWarning(appLog) << "failed to register dbus object" << connection.lastError().message();
    }
    if (!connection.registerService("org.deepin.DeviceControl")) {
        qCWarning(appLog) << "failed to register dbus object" << connection.lastError().message();
    }
    return a.exec();
}
