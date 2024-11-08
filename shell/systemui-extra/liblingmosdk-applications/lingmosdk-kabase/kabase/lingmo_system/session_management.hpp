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

#ifndef LINGMO_SYSTEM_SESSION_MANAGEMENT_H_
#define LINGMO_SYSTEM_SESSION_MANAGEMENT_H_

#include <QDBusInterface>
#include <QString>

#include "currency.hpp"

namespace kdk
{
namespace kabase
{

class SessionManagement
{
public:
    SessionManagement();
    ~SessionManagement();

    /**
     * @brief 设置禁止锁屏
     *
     * @param appName 应用名
     * @param reason 原因
     *
     * @retval 非0正整数 成功
     * @retval 0 失败
     */
    quint32 setInhibitLockScreen(AppName appName, QString reason);

    /**
     * @brief 取消禁止锁屏
     *
     * @param flag setNoLockScreen函数 返回值
     *
     * @retval true 成功
     * @retval false 失败
     */
    bool unInhibitLockScreen(quint32 flag);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
