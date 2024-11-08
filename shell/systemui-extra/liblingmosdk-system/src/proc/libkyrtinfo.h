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

#ifndef KDK_SYSTEM_PROC_H__
#define KDK_SYSTEM_PROC_H__

/**
 * @file libkyrtinfo.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 当前系统资源占用信息
 * @version 0.1
 * @date 2021-11-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-proc
 * @{
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取系统中物理内存总大小
 * 
 * @return unsigned long 物理内存大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_res_total_KiB();

/**
 * @brief 获取物理内存使用率
 * 
 * @return float 物理内存使用率
 */
extern float kdk_rti_get_mem_res_usage_percent();

/**
 * @brief 获取物理内存使用大小，注意Buffer/Cache被计算为已使用内存
 * 
 * @return unsigned long 物理内存使用大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_res_usage_KiB();

/**
 * @brief 获取实际可用的物理内存大小，该数值约等于Free + Buffer + Cache
 * 
 * @return unsigned long 可用物理内存大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_res_available_KiB();

/**
 * @brief 获取实际空闲的物理内存大小，注意Buffer/Cache被计算为已使用内存
 * 
 * @return unsigned long 空闲的物理内存大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_res_free_KiB();

/**
 * @brief 获取所有应用申请的虚拟内存总量
 * 
 * @return unsigned long 虚拟内存总申请量，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_virt_alloc_KiB();

/**
 * @brief 获取系统中Swap分区总大小
 * 
 * @return unsigned long Swap分区大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_swap_total_KiB();

/**
 * @brief 获取Swap分区使用率
 * 
 * @return float Swap分区使用率
 */
extern float kdk_rti_get_mem_swap_usage_percent();

/**
 * @brief 获取Swap分区使用量
 * 
 * @return unsigned long Swap分区使用量，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_swap_usage_KiB();

/**
 * @brief 获取Swap分区空闲大小
 * 
 * @return unsigned long Swap分区空闲大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_swap_free_KiB();

/**
 * @brief 获取CPU瞬时使用率
 * 
 * @return float CPU瞬时使用率，该值 < 1.00
 */
extern float kdk_rti_get_cpu_current_usage();

// /**
//  * @brief 判断CPU当前负载压力是否正常
//  * 
//  * @return int 0表示正常，1表示压力过大
//  */
// extern int kdk_rti_test_load_isok();

/**
 * @brief 获取操作系统开机时长
 * 
 * @param day 开机天数
 * @param hour 小时数，该数值一定 < 24
 * @param min 分钟数，该数值一定 < 60
 * @param sec 秒数，该数值一定 < 60
 * @return int 返回0表示成功，返回非零表示失败
 */
extern int kdk_rti_get_uptime(unsigned int *day, unsigned int *hour, unsigned int *min, unsigned int *sec);

/**
 * @brief 获取共享内存大小
 * 
 * @return unsigned long 共享内存大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_shared_KiB();

/**
 * @brief 获取高速缓存大小
 * 
 * @return unsigned long 高速缓存大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_cached_KiB();

/**
 * @brief 获取数据缓存大小
 * 
 * @return unsigned long 数据缓存大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_buffers_KiB();

/**
 * @brief 获取交换缓存区大小
 * 
 * @return unsigned long 交换缓存区大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_swap_cached_KiB();

/**
 * @brief 获取活跃的缓冲文件大小
 * 
 * @return unsigned long 活跃的缓冲文件大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_active_KiB();

/**
 * @brief 获取不活跃的缓冲文件大小
 * 
 * @return unsigned long 不活跃的缓冲文件大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_inactive_KiB();

/**
 * @brief 获取脏页大小
 * 
 * @return unsigned long 脏页大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_dirty_KiB();

/**
 * @brief 获取映射大小
 * 
 * @return unsigned long 映射大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_map_KiB();

/**
 * @brief 获取内核数据结构缓存大小
 * 
 * @return unsigned long 内核数据结构缓存大小，KiB为单位
 */
extern unsigned long kdk_rti_get_mem_slab_KiB();

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/proc/test/kyrtinfo-test.c
 * 
 */

/**
  * @}
  */
