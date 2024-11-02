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

#ifndef APPLICATION_ACCESS_H_
#define APPLICATION_ACCESS_H_

#include <QObject>
#include <QList>
#include <QVariant>

#include "currency.hpp"

namespace kdk
{
namespace kabase
{

class ApplicationAccess
{
public:
    ApplicationAccess();
    ~ApplicationAccess();

    /**
     * @brief 导出函数
     *
     * @param appname 应用名
     * @param object 要导出函数的对象指针
     *
     * @retval true 成功
     * @retval false 失败
     */
    bool exportFunc(AppName appName, QObject *object);

    /**
     * @brief 调用函数
     *
     * @param appName 应用名
     * @param methodName 函数名
     * @param args 函数参数列表
     *
     * @return 函数返回结果
     */
    QList<QVariant> callMethod(AppName appName, QString methodName, QList<QVariant> args = QList<QVariant>());

private:
    QString getServiceName(AppName appName);
    QString getObjectPath(AppName appName);
    QString getInterfaceName(AppName AppName);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
