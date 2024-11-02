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

#ifndef LINGMO_SYSTEM_USER_MANUAL_H_
#define LINGMO_SYSTEM_USER_MANUAL_H_

#include <QDBusInterface>

#include "currency.hpp"

namespace kdk
{
namespace kabase
{

class UserManualManagement
{
public:
    UserManualManagement();
    ~UserManualManagement();

    /**
     * @brief 调用用户手册
     *
     * @param appName 应用名
     *
     * @retval true 成功
     * @retval false 失败
     * 
     * @note AppName 只适用与组内应用，其他应用调用需要维护 AppName 枚举
     *       由于调用用户手册接口需要普遍调用, 改用重载的 QString 类型参数 , 该接口不在维护
     *       -jishengjie 2022-04-15
     */
    bool callUserManual(AppName appName);

    /**
     * @brief 调用用户手册
     * 
     * @param appName 应用名
     * 
     * @retval true 成功
     * @retval false 失败
     */
    bool callUserManual(QString appName);

private:
    QString adapterAppName(QString appName);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
