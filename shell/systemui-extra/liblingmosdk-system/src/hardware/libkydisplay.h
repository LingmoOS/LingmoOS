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

#ifndef LIBKYDISPLAY_H
#define LIBKYDISPLAY_H

/**
 * @file libkydisplay.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 显卡信息
 * @version 0.1
 * @date 2023-6-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-hardware
 * @{
 * 
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 获取显卡的制造商
 * 
 * @return char* 成功返回显卡的制造商，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_vendor();

/**
 * @brief 获取显卡的型号
 * 
 * @return char* 成功返回显卡的型号，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_product();

/**
 * @brief 获取显卡的说明
 * 
 * @return char* 成功返回显卡的说明，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_description();

/**
 * @brief 获取显卡的物理id
 * 
 * @return char* 成功返回显卡的物理id，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_physical_id();

/**
 * @brief 获取显卡的总线地址
 * 
 * @return char* 成功返回显卡的总线地址，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_bus_info();

/**
 * @brief 获取显卡的设备版本
 * 
 * @return char* 成功返回显卡的设备版本，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_version();

/**
 * @brief 获取显卡的数据宽度
 * 
 * @return char* 成功返回显卡的数据宽度，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_width();

/**
 * @brief 获取显卡的频率
 * 
 * @return char* 成功返回显卡的频率，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_clock();

/**
 * @brief 获取显卡的功能
 * 
 * @return char* 成功返回显卡的功能，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_capabilities();

/**
 * @brief 获取显卡的配置
 * 
 * @return char* 成功返回显卡的配置，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_configuration();

/**
 * @brief 获取显卡的资源
 * 
 * @return char* 成功返回显卡的资源，失败返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_display_get_resources();

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/hardware/test/kydisplay-test.c
 * 
 */
