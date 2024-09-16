// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "src/window/modules/common/common.h"
#include "window/namespace.h"

#include <QDBusInterface>

class DBusInvokerInterface;

DEF_NAMESPACE_BEGIN

class SysUpdaterModel : public QObject
{
    Q_OBJECT
public:
    explicit SysUpdaterModel(QObject *parent = nullptr);
    ~SysUpdaterModel();

    // 检查是否有更新
    void CheckForUpdates();

Q_SIGNALS:
    // 发送是否存在系统更新结果
    void SendHaveUpdates(bool have);

public Q_SLOTS:
    // 设置检查系统更新任务
    void SetCheckUpdatesJob(const QString &jobPath);
    // 停止检查系统版本
    void stopCheckingSysVer();
    // 检查更新任务状态
    void onCheckJobStatusChanged(const QString &status);

private:
    // 系统更新管理器dbus服务
    DBusInvokerInterface *m_managerInvokerInter;
    // 系统更新dbus服务
    DBusInvokerInterface *m_updaterInvokerInter;
    // 检查更新任务
    DBusInvokerInterface *m_checkUpdateJobInvokerInter;
    QTimer *m_checkUpdateJobTimer;
};

DEF_NAMESPACE_END
