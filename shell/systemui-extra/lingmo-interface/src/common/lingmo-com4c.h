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

#ifndef __LINGMOCOMM4C_H__
#define __LINGMOCOMM4C_H__

#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @fn kdk_get_lsbrelease
 * @brief 根据lsbrelease信息的键获取值
 * @param[in] key lsbrelease信息的键
 * @param[out] value lsbrelease信息的值
 * @param[in] value_max_len 值缓存区的大小
 * @retval int > 0 成功 否则失败
 */
int kdk_get_lsbrelease(const char *key, char *value, int value_max_len);

/**
 * @fn kdk_get_osrelease
 * @brief 根据osrelease信息的键获取值
 * @param[in] key osrelease信息的键
 * @param[out] value osrelease信息的值
 * @param[in] value_max_len 值缓存区的大小
 * @retval int > 0 成功 否则失败
 */
int kdk_get_osrelease(const char *key, char *value, int value_max_len);

/**
 * @fn kdk_get_kyinfo
 * @brief 根据kyinfo的键获取值
 * @param[in] session kyinfo的session值
 * @param[in] key kyinfo的键
 * @param[out] value kyinfo的值
 * @param[in] value_max_len 值缓存区的大小
 * @retval int >= 0 成功 否则失败
 */
int kdk_get_kyinfo(const char *session, const char *key, char *value, int value_max_len);

/**
 * @fn kdk_get_prjcodename
 * @brief 根据PROJECT_CODENAME字段的值
 * @param[out] value PROJECT_CODENAME字段的值
 * @param[in] value_max_len 值缓存区的大小
 * @retval int > 0 成功 否则失败
 */
int kdk_get_prjcodename(char *value, int value_max_len);

/**
 * @fn kdk_get_cpumodelname
 * @brief 获取CPU型号
 * @param[out] modelName CPU型号信息
 * @param[in] max_len CPU型号缓存区的大小
 * @retval int > 0 成功 否则失败
 */
int kdk_get_cpumodelname(char *modelName, int max_len);

/**
 * @fn kdk_get_spechdplatform
 * @brief 获取特定硬件平台信息
 * @param[out] platformName 特定硬件平台信息
 * @param[in] max_len 特定硬件平台缓存区的大小
 * @retval int > 0 成功 否则失败
 */
int kdk_get_spechdplatform(char *platformName, int max_len);

#ifdef  __cplusplus
}
#endif

#endif // __LINGMOCOMM4C_H__