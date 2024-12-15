// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QTimer>
#include <DConfig>
#include <DLog>

#include "dtkcore_global.h"
#include "dbusserver.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    MLogger();   // 日志处理要放在app之前，否则QApplication
            // 内部可能进行了日志打印，导致环境变量设置不生效
// 为了兼容性
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
    Dtk::Core::DLogManager::registerJournalAppender();
#else
    Dtk::Core::DLogManager::registerFileAppender();
#endif
#ifdef QT_DEBUG
    Dtk::Core::DLogManager::registerConsoleAppender();
#endif

    QCoreApplication a(argc, argv);

    DBusServer dbusServer;
    dbusServer.exitDBusServer(10000);

    return a.exec();
}
