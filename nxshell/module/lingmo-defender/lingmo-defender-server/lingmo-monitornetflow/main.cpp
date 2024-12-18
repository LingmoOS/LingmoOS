// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "writedbusdata.h"

#include <DLog>

#include <QCoreApplication>

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DLogManager::setlogFilePath("/var/log/lingmo-monitornetflow.log");
    QCoreApplication a(argc, argv);
    a.setOrganizationName("lingmo");
    a.setApplicationName("lingmo-defender-MonitorNetFlow");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    WriteDBusData dbusData;

    return a.exec();
}
