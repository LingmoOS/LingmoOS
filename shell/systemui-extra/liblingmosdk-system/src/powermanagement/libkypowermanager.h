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

#ifndef LIBKYPOWERMANAGER_H
#define LIBKYPOWERMANAGER_H

/**
 * @file libkybattery.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 电源信息
 * @version 0.1
 * @date 2024-4-11
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-powermanagement
 * @{
 * 
 */

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 检测当前设备经过多长时间后息屏
 * 
 * @return int 设备经过多长时间后息屏，单位秒。
 */
extern int kdk_power_get_screenidle_timeout();

/**
 * @brief 检测当前设备是否处于活动状态
 * 
 * @return int 设备是否处于活动状态，0处于活动状态，其他不处于。
 */
extern int kdk_power_is_active();

/**
 * @brief 获取当前设备的电源模式
 * 
 * @return int 当前设备的电源模式, 0-最佳性能，1-平衡，2-最佳能效
 */
extern int kdk_power_get_mode();

/**
 * @brief 获取电源支持的休眠方式
 * 
 * @return char* 成功返回电源支持的休眠方式，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_power_is_hibernate();

/**
 * @brief 获取控制休眠的操作模式（挂起到磁盘）
 * 
 * @return char* 成功返回控制休眠的操作模式，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_power_get_control_disk_status();

#ifdef __cplusplus
}
#endif

#endif