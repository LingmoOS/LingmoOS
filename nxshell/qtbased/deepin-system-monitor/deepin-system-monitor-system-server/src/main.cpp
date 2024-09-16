// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>

#include "systemdbusserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SystemDBusServer dbusServer;
    dbusServer.exitDBusServer(10000);

    return a.exec();
}
