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

#ifndef KAPPWIDGETPROVIDER_H
#define KAPPWIDGETPROVIDER_H

#include "liblingmo-appwidget-provider_global.h"

#include <QObject>

class QDBusVariant;
namespace AppWidget
{
/** @defgroup 小插件提供端模块
  * @{
  */

class LIBLINGMOAPPWIDGETPROVIDER_EXPORT KAppWidgetProvider : public QObject
{
    Q_OBJECT
public:
    KAppWidgetProvider(QString providername, QObject *parent = nullptr) ;
public slots:
    /**
     * @brief 小插件自定义事件
     */
    virtual void appWidgetRecevie(const QString &eventname, const QString &widgetname, const QDBusVariant &value);
    /**
     * @brief 小插件更新函数
     */
    virtual void appWidgetUpdate();
    /**
     * @brief 小插件禁用函数
     */
    virtual void appWidgetDisable();
    /**
     * @brief 小插件删除函数
     */
    virtual void appWidgetDeleted();
    /**
     * @brief 小插件启用函数
     */
    virtual void appWidgetEnable();
    /**
     * @brief 小插件点击函数
     */
    virtual void appWidgetClicked(const QString &widgetname, const QDBusVariant &value);
};
}// namespace AppWidget


#endif // KAPPWIDGETPROVIDER_H
