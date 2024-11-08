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

#ifndef LIBKYFAN_H
#define LIBKYFAN_H

/**
 * @file libkyfan.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 风扇信息
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
 * @brief 获取风扇的信息
 * 
 * @return char** 风扇的信息（名称，转速）
 */
extern char** kdk_fan_get_information();

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 */
extern inline void kdk_fan_freeall(char **ptr);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/hardware/test/kyfan-test.c
 * 
 */
