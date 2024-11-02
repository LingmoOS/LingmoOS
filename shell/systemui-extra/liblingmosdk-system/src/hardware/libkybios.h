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

#ifndef LIBKYBIOSINFO_H
#define LIBKYBIOSINFO_H

/**
 * @file libkybios.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief bios信息
 * @version 0.1
 * @date 2023-2-17
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
 * @brief 获取bios厂商
 *
 * @return const char* 厂商名，返回的字符串需要被 free 释放
 */
extern const char *kdk_bios_get_vendor();

/**
 * @brief 获取bios版本
 * 
 * @return const char* 版本号，返回的字符串需要被 free 释放
 */
extern const char *kdk_bios_get_version();

/**
 * @brief 释放内存
 * 
 * @param info 本模块接口返回的字符串
 */
extern void kdk_bios_free(char *info);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/hardware/test/kybios-test.c
 * 
 */
