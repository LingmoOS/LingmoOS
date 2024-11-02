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

#ifndef POWERMANAGEMENT_LIBKYLOCKSCREEN_H_
#define POWERMANAGEMENT_LIBKYLOCKSCREEN_H_

/**
 * @file libkylockscreen.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 提供锁屏设置能力
 * @version 0.1
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-powermanagement
 * @{
 * 
 * 
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 设置禁止锁屏 
 * 
 * @param appName : 应用名
 * @param reason : 禁止锁屏原因
 * 
 * @return uint32_t 0 : 失败 , >0 : 成功
 */
extern uint32_t kdk_set_inhibit_lockscreen(const char *appName , const char *reason);

/**
 * @brief 取消禁止锁屏 
 * 
 * @param flag : 禁止锁屏接口的返回值
 * 
 * @retval 0 : 成功
 * @retval -1 : 失败
 */
extern int kdk_un_inhibit_lockscreen(uint32_t flag);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/powermanagement/test/testlockscreen/main.c
 * 
 */
