/*
 * liblingmosdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#ifndef DBUS_H_
#define DBUS_H_

#include <QObject>
#include <QList>
#include <QVariant>

namespace kdk
{
namespace kabase
{

class DBus
{
public:
    DBus();
    ~DBus();

    /**
     * @brief 注册服务
     *
     * @param serviceName 服务名
     *
     * @retval true 成功
     * @retval false 失败
     */
    static bool registerService(QString serviceName);

    /**
     * @brief 注册对象
     *
     * @param objectPath 对象路径
     * @param object 要注册的对象指针
     *
     * @retval true 成功
     * @retval false 失败
     */
    static bool registerObject(QString objectPath, QObject *object);

    /**
     * @brief 函数调用
     *
     * @param serviceName 服务名
     * @param objectPath 对象路径
     * @param interfaceName 接口名
     * @param methodName 函数名
     * @param args 参数列表
     *
     * @return 函数返回值
     */
    static QList<QVariant> callMethod(QString serviceName, QString objectPath, QString interfaceName,
                                      QString methodName, QList<QVariant> args = QList<QVariant>());
};

} /* namespace kabase */
} /* namespace kdk */

#endif
