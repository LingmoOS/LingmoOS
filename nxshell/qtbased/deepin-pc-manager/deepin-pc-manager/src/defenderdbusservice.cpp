// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderdbusservice.h"

#include "window/mainwindow.h"

#include <QDBusConnection>

const QString service = "com.deepin.pc.manager";
const QString path = "/com/deepin/pc/manager";

DefenderDBusService::DefenderDBusService(QObject *parent)
    : QObject(parent)
{
    m_managerAdaptor = new ManagerAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService(service)
        || !QDBusConnection::sessionBus().registerObject(path, this)) {
        qDebug() << "dbus service already registered! " << service;

        exit(0);
    }
}

void DefenderDBusService::Show()
{
    parent()->raise();
    if (parent()->isMinimized() || !parent()->isVisible())
        parent()->showNormal();

    parent()->activateWindow();
}

MainWindow *DefenderDBusService::parent()
{
    return qobject_cast<MainWindow *>(QObject::parent());
}

void DefenderDBusService::ShowModule(const QString &module)
{
    ShowPage(module, "");
    Show();
}

void DefenderDBusService::ShowPage(const QString &module, const QString &page)
{
    if (parent()->getModuleCount() == 0)
        parent()->initAllModule();
    parent()->showModulePage(module, page);
}

void DefenderDBusService::ExitApp()
{
    qApp->quit();
}
