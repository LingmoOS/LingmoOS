// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "writedbusdata.h"
#include "netcheck/netcheckdbusinter.h"

#include <DLog>

#include <QCoreApplication>
#include <QThreadPool>

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DLogManager::setlogFilePath("/var/log/deepin-pc-manager-system-daemon.log");
    QCoreApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-pc-manager-system-daemon");

    // 设置异步线程退出时间，默认为30秒
    QThreadPool::globalInstance()->setExpiryTimeout(100);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    WriteDBusData dbusData;
    NetCheckDBusInter netCheckDBusInter;
    return a.exec();
}
