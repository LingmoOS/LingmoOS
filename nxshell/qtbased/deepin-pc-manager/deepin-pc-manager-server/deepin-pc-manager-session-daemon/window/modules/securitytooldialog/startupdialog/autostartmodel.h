// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class DBusInvokerInterface;
class SettingsInvokerInterface;

namespace def {
class SystemSettings;
}

using namespace def;

class AutoStartModel : public QObject
{
    Q_OBJECT
public:
    explicit AutoStartModel(QObject *parent = nullptr);
    ~AutoStartModel();

    // 更新所有应用自启动状态
    bool updateAppsAutoStartStatus() const;
    // 获取所有自启动应用的信息字符串
    QString getAllAutoStartAppsInfoStr() const;
    // 获取所有非自启动应用的信息字符串
    QString getAllNonautoStartAppsInfoStr() const;
    // 应用是否自启动
    bool isAppAutoStart(const QString &path) const;
    // 设置应用自启动
    bool setAppAutoStart(int status, const QString &path) const;
    // 添加日志
    void addSecurityLog(int type, const QString &log) const;

Q_SIGNALS:
    void itemStatusChanged(bool isAuto, const QString &id);

private:
    void init();
    // 初始化信号
    void initSignalSLot();

private:
    DBusInvokerInterface *m_dataInterfaceInvoker; // 安全中心数据服务接口
};
