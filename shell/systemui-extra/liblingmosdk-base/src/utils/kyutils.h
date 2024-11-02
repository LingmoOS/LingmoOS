/*
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


/**
 * @file kyutils.h
 * @brief 
 * @author liuyang <liuyang@kylinos.cn>
 * @version 1.0
 * @date 2021-09-01
 * @example lingmosdk-base/src/utils/sample/kyutils_sample.c
 * 
 * @copyright Copyright: 2021,KylinSoft Co.,Ltd.
 * 
 */

#ifndef LINGMOSDK_UTILS_H__
#define LINGMOSDK_UTILS_H__

#include <stdint.h>
#include <ctype.h>
#include <inttypes.h>
#include <bits/types.h>
#include <locale.h>
#include <errno.h>
#include <stddef.h>
#include <sys/syslog.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup 单位进制转换组件
 * @{ 
 */


/** KDKUtilsErrCode */
typedef enum{
    KDK_NOERR,                      /**< 无异常 */
    KDK_INVAILD_BASE,               /**< 非法进制 */
    KDK_INVAILD_ARGUMENT,           /**< 非法参数 */
    KDK_INVAILD_DATA_FORMAT,        /**< 非法数据格式 */
    KDK_SYSTEM_UNKOWNERR,           /**< 系统运行异常引发的未知错误 */
}KDKUtilsErrCode;

/** KDKVolumeBaseType */
typedef enum{
    KDK_KILOBYTE = 1,               /**< KB */
    KDK_MEGABYTE,                   /**< MB */
    KDK_GIGABYTE,                   /**< GB */
    KDK_TERABYTE,                   /**< TB */
    KDK_PETABYTE,                   /**< PB */
    KDK_EXABYTE,                    /**< EB */
    KDK_KILO,                       /**< K */
    KDK_MEGA,                       /**< M */
    KDK_GIGA,                       /**< G */
    KDK_TERA,                       /**< T */
    KDK_PETA,                       /**< P */
    KDK_EXA,                        /**< E */
}KDKVolumeBaseType;


/**
 * @brief 字符格式单位进制转换
 * @param[in]   origin_data      原始字符类型的数据，以具体进制单位结束，如果没有，缺省为‘B’
 * @param[in]   result_base      期望的结果进制单位
 * @param[out]  result_data      转化进制后的字符数据，带进制单位
 * @return int
 * 
 */

extern int kdkVolumeBaseCharacterConvert(const char* origin_data, KDKVolumeBaseType result_base, char* result_data);


/**
 * @brief 数字格式单位进制转换
 * @param[in]   origin_numerical 原始数字类型数据
 * @param[in]   origin_base      原始的进制单位
 * @param[in]   result_base      期望的进制单位
 * @param[out]  result_numerical 期望进制下的数字类型数据
 * @return int 
 */

extern int kdkVolumeBaseNumericalConvert(double origin_numerical, KDKVolumeBaseType origin_base, KDKVolumeBaseType result_base, double* result_numerical);

/** 
 * @} 
 */


#ifdef __cplusplus
}
#endif

#endif
