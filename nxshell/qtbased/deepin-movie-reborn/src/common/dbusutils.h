// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DBUSUTILS_H
#define DBUSUTILS_H
#include <QVariant>

/**
 * @file dbus接口读取工具
 */
class DBusUtils
{
public:
    DBusUtils();
    /**
     * @brief 读取d-bus接口参数值
     * @param dbus服务名
     * @param dbus路径
     * @param dbus接口名
     * @param 获取的参数名
     * @return 获取参数的值
     */
    static QVariant redDBusProperty(const QString &sService, const QString &sPath, const QString &sInterface = QString(), const char *pPropert = "");
    /**
     * @brief 调用d-bus方法
     * @param d-bus服务名
     * @param d-bus路径
     * @param d-bus接口名
     * @param d-bus的方法
     * @return 方法的返回值
     */
    static QVariant redDBusMethod(const QString &sService, const QString &sPath, const QString &sInterface, const char *pMethod);
};

#endif // DBUSUTILS_H
