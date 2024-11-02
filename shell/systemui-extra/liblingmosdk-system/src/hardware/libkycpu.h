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

#ifndef KDK_SYSTEM_HW_CPU_H__
#define KDK_SYSTEM_HW_CPU_H__
/**
 * @file libkycpu.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief CPU信息
 * @version 0.1
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-hardware
 * @{
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取CPU架构
 * 
 * @return const char* 架构信息，如"x86_64"；返回的是const char*，不要free
 */
extern const char* kdk_cpu_get_arch();

/**
 * @brief 获取CPU制造厂商
 * 
 * @return const char* 制造厂商字符串，如“GenuineIntel”；返回的是const char*，不要free
 */
extern const char* kdk_cpu_get_vendor();

/**
 * @brief 获取CPU型号
 * 
 * @return const char* CPU型号名称，如“Intel(R) Core(TM) i7-4790K CPU @ 4.00GHz”；返回的是const char*，不要free
 */
extern const char* kdk_cpu_get_model();

/**
 * @brief 获取CPU额定主频
 * 
 * @return const char* 额定主频赫兹数，如“1794.742”，单位是MHz；返回的是const char*，不要free
 */
extern const char* kdk_cpu_get_freq_MHz();

/**
 * @brief 获取CPU核心数量
 * 
 * @return unsigned int 所有可用的CPU核心数量
 */
extern unsigned int kdk_cpu_get_corenums();

/**
 * @brief 获取CPU对虚拟化的支持
 * 
 * @return const char* 若CPU支持虚拟化，则返回虚拟化技术，如“vmx”；若不支持，返回NULL；返回的是const char*，不要free
 */
extern const char* kdk_cpu_get_virt();

/**
 * @brief 获取CPU线程数/逻辑处理器数
 * 
 * @return unsigned int 返回cpu支持的线程数/逻辑处理器数
 */
extern unsigned int kdk_cpu_get_process();

/**
 * @brief 获取CPU最大频率
 * 
 * @return float 返回cpu频率最大频率，如“2600.0000”，单位为MHz。
 */
extern float kdk_cpu_get_max_freq_MHz();

/**
 * @brief 获取CPU最小频率
 * 
 * @return float 返回cpu频率最小频率，如“1900.0000”，单位为MHz。
 */
extern float kdk_cpu_get_min_freq_MHz();

/**
 * @brief 获取CPU运行时间
 * 
 * @return char* 返回cpu运行时间，如“xx天xx小时xx分钟”；若失败，返回NULL；返回的字符串需要被 free 释放
 */
extern char* kdk_cpu_get_running_time();

/**
 * @brief 获取CPU插槽
 * 
 * @return unsigned int 返回cpu插槽数量; 失败返回-1。
 */
extern unsigned int kdk_cpu_get_sockets();

/**
 * @brief 获取CPU L1缓存（数据）
 * 
 * @return unsigned int 返回cpu L1缓存（数据）; 失败返回-1。
 */
extern unsigned int kdk_cpu_get_L1d_cache();

/**
 * @brief 获取CPU L1缓存（指令）
 * 
 * @return unsigned int 返回cpu L1缓存（指令）; 失败返回-1。
 */
extern unsigned int kdk_cpu_get_L1i_cache();

/**
 * @brief 获取CPU L2缓存
 * 
 * @return unsigned int 返回cpu L2缓存; 失败返回-1。
 */
extern unsigned int kdk_cpu_get_L2_cache();

/**
 * @brief 获取CPU L3缓存
 * 
 * @return unsigned int 返回cpu L3缓存; 失败返回-1。
 */
extern unsigned int kdk_cpu_get_L3_cache();

#ifdef __cplusplus
}
#endif

#endif
/**
  * @}
  */
