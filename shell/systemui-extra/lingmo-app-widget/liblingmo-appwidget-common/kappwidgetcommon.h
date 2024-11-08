/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#ifndef KAPPWIDGETCOMMON_H
#define KAPPWIDGETCOMMON_H

#include "liblingmo-appwidget-common_global.h"
#include <QObject>

namespace AppWidget
{

/**
 * @defgroup 公共接口模块
 */

class LIBLINGMOAPPWIDGETCOMMON_EXPORT KAppWidgetCommon : public QObject
{
    Q_OBJECT
public:
    explicit KAppWidgetCommon(QObject *parent = nullptr);

    /**
     * @brief 获取Providername的提供端Dbus Object name
     * @param providername
     * @return Provider dbus object name
     */
    static QString getProviderDbusObject(const QString &providername);
    /**
     * @brief 获取Providername的提供端Dbus Path
     * @param providername
     * @return Provider dbus path name
     */
    static QString getProviderDbusPath(const QString &providername);
    /**
     * @brief 获取Providername的提供端Dbus Interface
     * @param providername
     * @return Provider dbus interface name
     */
    static QString getProviderDbusInterface(const QString &providername);
    /**
     * @brief 获取Manager的提供端Dbus Object name
     * @return Manager dbus object name
     */
    static QString getManagerDbusObject();
    /**
     * @brief 获取Manager的提供端Dbus Path
     * @return Manager dbus path name
     */
    static QString getManagerDbusPath();
    /**
     * @brief 获取Manager的提供端Dbus Interface
     * @return Manager dbus interface name
     */
    static QString getManagerDbusInterface();
    /**
     * @brief 获取AppWidget的提供端Dbus Object name
     * @param username appwidget`id
     * @return Widget dbus object name
     */
    static QString getAppWidgetDbusObject(const QString &username, const int &id);
    /**
     * @brief 获取AppWidget的提供端Dbus Object name
     * @param username appwidget`id
     * @return Widget dbus path name
     */
    static QString getAppWidgetDbusPath(const QString &username, const int &id);
    /**
     * @brief 获取AppWidget的提供端Dbus Object name
     * @param username appwidget`id
     * @return Widget dbus interface name
     */
    static QString getAppWidgetDbusInterface(const QString &username, const int &id);

};
}// namespace AppWidget

#endif // KAPPWIDGETCOMMON_H
