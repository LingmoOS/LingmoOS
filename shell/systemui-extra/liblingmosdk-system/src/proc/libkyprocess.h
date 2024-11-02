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

#ifndef LIBKYPROCESS_H
#define LIBKYPROCESS_H

#endif

/**
 * @file libkyprocess.h
 * @author shaozhimin (shaozhimin@kylinos.cn)
 * @brief 某一进程的瞬时详细信息
 * @version 0.1
 * @date 2022-09-13
 * 
 * @copyright Copyright (c) 2022
 * @defgroup 某一进程的瞬时详细信息
 * @{
 * 
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 获取某一进程的名称
 * 
 * @param proc_num 进程号
 * @return char* 成功返回进程的名称，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_get_process_name(int proc_num);

/**
 * @brief 获取某一进程的id
 * 
 * @param proc_name 进程的名称
 * @return int 成功返回进程号（id），失败返回0.
 */
extern int kdk_get_process_id(char *proc_name);

/**
 * @brief 获取某一进程的CPU使用率
 * 
 * @param proc_num 进程号
 * @return float 成功返回CPU使用率，失败返回0.0
 */
extern float kdk_get_process_cpu_usage_percent(int proc_num);

/**
 * @brief 获取某一进程的内存占用率
 * 
 * @param proc_num 进程号
 * @return float 成功返回内存占用率，失败返回0.00
 */
extern float kdk_get_process_mem_usage_percent(int proc_num);

/**
 * @brief 获取某一进程的进程状态
 * 
 * @param proc_num 进程号
 * @return char* 成功返回进程状态，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_get_process_status(int proc_num);

/**
 * @brief 获取某一进程的端口号占用
 * 
 * @param proc_num 进程号
 * @return int 成功返回使用的端口号，失败返回0。
 */
extern int kdk_get_process_port(int proc_num);

/**
 * @brief 获取某一进程的端口号占用
 * 
 * @param proc_num 进程号
 * @return int* 成功返回使用的端口号，失败返回0。
 */
extern int* kdk_get_process_port_nums(int proc_num);

/**
 * @brief 获取某一进程的启动时间
 * 
 * @param proc_num 进程号
 * @return char* 成功返回启动时间，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_get_process_start_time(int proc_num);

/**
 * @brief 获取某一进程的运行时间
 * 
 * @param proc_num 进程号
 * @return char* 成功返回运行时间，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_get_process_running_time(int proc_num);

/**
 * @brief 获取某一进程的cpu时间
 * 
 * @param proc_num 进程号
 * @return char* 成功返回cpu时间，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_get_process_cpu_time(int proc_num);

/**
 * @brief 获取某一进程的Command
 * 
 * @param proc_num 进程号
 * @return char* 成功返回Command，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_get_process_command(int proc_num);

/**
 * @brief 获取某一进程的属主
 * 
 * @param proc_num 进程号
 * @return char* 成功返回属主，失败返回NULL。返回的字符串需要被 free 释放。
 */
extern char* kdk_get_process_user(int proc_num);


/**
 * @brief 获取某一进程的信息
 * 
 * @param proc_name 进程名
 * @return char** 成功返回某一进程的信息，以NULL表示结尾，由alloc生成，需要被kdk_proc_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_procname_get_process_infomation(char *proc_name);

/**
 * @brief 获取系统中所有进程的信息
 *
 * @return char** 成功返回进程所有信息，以NULL表示结尾，由alloc生成，需要被kdk_proc_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_get_process_all_information();

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 */
extern inline void kdk_proc_freeall(char **ptr);

/**
 * @brief 用于回收进程id资源
 *
 * @param list 进程id列表
 */
extern inline void kdk_proc_freeid(int *list);

#ifdef __cplusplus
}

#endif

/**
 * \example lingmosdk-system/src/proc/test/kyprocess-test.c
 * 
 */

/**
  * @}
  */
