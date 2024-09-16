// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSMANAGER_H
#define DBUSMANAGER_H

#include <QObject>
#include <QVariantMap>
/**
 * @brief The DBusManager class  dbus接口获取工具类
 */
class DBusManager : public QObject
{
    Q_OBJECT
public:
    explicit DBusManager(QObject *parent = nullptr);
    // 是否开启等保四
    static bool isSEOpen();
    // 开启等保四情况下，判断是否为审计管理员身份
    static bool isAuditAdmin();
    // 利用freedeskop获取当前用户家目录路径
    static QString getHomePathByFreeDesktop();
signals:

public slots:
};

#endif // DBUSMANAGER_H
