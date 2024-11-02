/*
 * liblingmosdk-system's Library
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include <stdint.h>
#include <string>

namespace kdk
{

class LockScreen
{
public:
    LockScreen();
    ~LockScreen();

    /**
     * @brief 设置禁止锁屏
     * 
     * @param appName : 应用名
     * @param reason : 原因描述 
     * 
     * @return 标记
     */
    uint32_t setInhibitLockScreen(std::string appName , std::string reason);

    /**
     * @brief 取消禁止锁屏
     * 
     * @param flag : 禁止锁屏接口返回来的标记
     * 
     * @return true : 成功 , false : 失败
     */
    bool unInhibitLockScreen(uint32_t flag);
};

}
