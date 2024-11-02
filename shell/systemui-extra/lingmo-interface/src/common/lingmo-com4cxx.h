/*
 * Copyright (C) 2021 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef __LINGMOCOMM4CXX_H__
#define __LINGMOCOMM4CXX_H__

#include <string>
using namespace std;

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @fn KDKGetLsbRelease
 * @brief 根据lsbrelease信息的键获取值
 * @param[in] key lsbrelease信息的键
 * @param[out] value lsbrelease信息的值
 * @param[in] value_max_len 值缓存区的大小
 * @retval string lsbrelease信息的值,empty 失败，否则成功
 */
string KDKGetLSBRelease(const string key);

/**
 * @fn KDKGetOSRelease
 * @brief 根据osrelease信息的键获取值
 * @param[in] key osrelease信息的键
 * @retval string osrelease信息的值,empty 失败，否则成功
 */
string KDKGetOSRelease(const string key);

/**
 * @fn KDKGetKYInfo
 * @brief 根据kyinfo的键获取值
 * @param[in] session kyinfo的session值
 * @param[in] key kyinfo的键
 * @retval string kyinfo信息的键值,empty 失败，否则成功
 */
string KDKGetKYInfo(const string session, const string key);

/**
 * @fn KDKGetPrjCodeName
 * @brief 根据PROJECT_CODENAME字段的值
 * @param None 无参
 * @retval string PROJECT_CODENAME字段的值,empty 失败，否则成功
 */
string KDKGetPrjCodeName();

/**
 * @fn KDKGetCpuModelName
 * @brief 获取CPU型号
 * @param None 无参
 * @retval string CPU型号信息,empty 失败，否则成功
 */
string KDKGetCpuModelName();

/**
 * @fn KDKGetSpecHDPlatform
 * @brief 获取特定硬件平台信息
 * @param None 无参
 * @retval string 特定硬件平台信息,empty 失败，否则成功
 */
string KDKGetSpecHDPlatform();

#ifdef  __cplusplus
}
#endif

#endif // __LINGMOCOMM4C_H__