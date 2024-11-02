/*
 * liblingmosdk-ukenv's Library
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

#ifndef LINGMOSDK_UKENV_USERMANUAL_H_
#define LINGMOSDK_UKENV_USERMANUAL_H_

#include <QString>

namespace kdk
{

class UserManual
{
public:
    UserManual();
    ~UserManual();

    /**
     * @brief 调用用户手册
     *
     * @param appName 应用名
     *
     * @retval true 成功
     * @retval false 失败
     */
    bool callUserManual(QString appName);
};

} // namespace kdk

#endif
