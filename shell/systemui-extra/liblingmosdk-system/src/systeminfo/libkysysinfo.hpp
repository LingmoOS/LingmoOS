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

#ifndef LINGMOSDK_SYSTEM_SYSINFO_H__
#define LINGMOSDK_SYSTEM_SYSINFO_H__

#include <string>
#include <stdbool.h>

/**
 * @file systeminfo.hpp（停止维护，请使用C语言接口）
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 操作系统基础信息
 * @version 0.1
 * @date 2021-11-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

namespace KDK_SYSTEM {

/**
 * @brief 获取架构类型
 * 
 * @return std::string 
 */
extern std::string getSystemArchitecture();

/**
* @brief 获取操作系统名称
* 
* @return std::string 
*/
extern std::string getSystemName();

/**
 * @brief 获取操作系统版本号
 * 
 * @param verbose 
 * @return std::string 
 */
extern std::string getSystemVersion(bool verbose = false);

/**
 * @brief 获取操作系统激活状态
 * 
 * @return true 
 * @return false 
 */
extern bool getSystemActivationStatus();

/**
 * @brief 获取操作系统服务序列号
 * 
 * @return std::string 
 */
extern std::string getSystemSerialNumber();

/**
 * @brief 获取内核版本号
 * 
 * @return std::string 
 */
extern std::string getKernelVersion();
}

#endif  // LINGMOSDK_SYSTEM_SYSINFO_H__
