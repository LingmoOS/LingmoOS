// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appitemmodel.h"
#include "appgroupmanager.h"

namespace apps {
AppItemModel::AppItemModel(QObject *parent)
    : QStandardItemModel(parent)
{
}

QHash<int, QByteArray> AppItemModel::roleNames() const
{
    return {{AppItemModel::DesktopIdRole, QByteArrayLiteral("desktopId")},
            {AppItemModel::NameRole, QByteArrayLiteral("name")},
            {AppItemModel::IconNameRole, QByteArrayLiteral("iconName")},
            {AppItemModel::StartUpWMClassRole, QByteArrayLiteral("startupWMClass")},
            {AppItemModel::NoDisplayRole, QByteArrayLiteral("noDisplay")},
            {AppItemModel::ActionsRole, QByteArrayLiteral("actions")},
            {AppItemModel::OCEANCategoryRole, QByteArrayLiteral("oceanCategory")},
            {AppItemModel::InstalledTimeRole, QByteArrayLiteral("installedTime")},
            {AppItemModel::LastLaunchedTimeRole, QByteArrayLiteral("lastLaunchedTime")},
            {AppItemModel::LaunchedTimesRole, QByteArrayLiteral("launchedTimes")},
            {AppItemModel::DockedRole, QByteArrayLiteral("docked")},
            {AppItemModel::OnDesktopRole, QByteArrayLiteral("onDesktop")},
            {AppItemModel::AutoStartRole, QByteArrayLiteral("autoStart")},
            {AppItemModel::AppTypeRole, QByteArrayLiteral("appType")}};
}
}
