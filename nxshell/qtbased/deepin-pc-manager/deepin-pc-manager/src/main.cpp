// Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderdbusservice.h"
#include "window/mainwindow.h"

#include <polkit-qt5-1/PolkitQt1/Authority>

#include <DApplication>
#include <DApplicationSettings>
#include <DLog>
#include <DWidgetUtil>

#include <QAccessible>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QObject>

DEF_USING_NAMESPACE
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
using namespace PolkitQt1;

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps); // 高清图
    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-pc-manager");
    a.setApplicationVersion(DApplication::buildVersion((QMAKE_VERSION))); // pro 文件中的版本
    a.loadTranslator();
    a.setApplicationDisplayName(QObject::tr("PC Manager"));
    a.setStyle("chameleon");
    a.setWindowIcon(QIcon::fromTheme("deepin-pc-manager"));
    a.setProductIcon(QIcon::fromTheme("deepin-pc-manager"));
    a.setApplicationDescription(
        QApplication::translate("main",
                                "Deepin PC Manager is an application tool designed to help users "
                                "quickly manage, maintain, and optimize computer systems."));
    a.setQuitOnLastWindowClosed(false);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    MainWindow w;
    Dtk::Widget::moveToCenter(&w);

    // regist dbus service
    DefenderDBusService dbus(&w);

    w.show();
    return a.exec();
}
