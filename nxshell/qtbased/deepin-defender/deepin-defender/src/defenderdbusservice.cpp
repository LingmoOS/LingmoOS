// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defenderdbusservice.h"
#include "window/mainwindow.h"

const QString service = "com.deepin.defender.hmiscreen";
const QString path = "/com/deepin/defender/hmiscreen";

DefenderDBusService::DefenderDBusService(QObject *parent)
    : QObject(parent)
{
    m_adaptor = new HmiscreenAdaptor(this);

    if (!QDBusConnection::sessionBus().registerService(service) || !QDBusConnection::sessionBus().registerObject(path, this)) {
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

// 显示设置弹窗
void DefenderDBusService::showSettingDialog(const QString &sGroupKey)
{
    parent()->showSettingDialog(sGroupKey);
}

// 联网管控弹框
void DefenderDBusService::showFuncConnectNetControl(QString sPkgName)
{
    parent()->showFuncConnectNetControl(sPkgName);
}

// 远程访问弹框
void DefenderDBusService::showFuncConnectRemControl(QString sPkgName)
{
    parent()->showFuncConnectRemControl(sPkgName);
}

// 流量详情弹框
void DefenderDBusService::showFuncConnectDataUsage()
{
    parent()->showFuncConnectDataUsage();
}
