// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DApplication>
#include <DMainWindow>
#include <DWidgetUtil>
#include <DLog>

#include "servicedbus.h"

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.loadTranslator();
    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-defender-daemon");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    ServiceDBus dbus;

    return a.exec();
}
