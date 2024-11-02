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

#ifndef LIBKYDATE_H
#define LIBKYDATE_H

/**
 * @file libkydate.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief 日期和时间格式信息
 * @version 0.1
 * @date 2023-2-17
 * 
 * @copyright Copyright (c) 2021
 * @defgroup liblingmosdk-systime
 * @{
 * 
 */

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _kdk_dateinfo{
    char *date;     // 日期
    char *time;   // 时间
    char *timesec;  //带秒钟数时间
}kdk_dateinfo;

typedef struct _kdk_logn_dateinfo{
    char *date;     // 日期
    char *time;   // 时间
    char *week;
}kdk_logn_dateinfo;

typedef struct _kdk_timeinfo{
    char *time;   // 时间
    char *timesec;  //带秒钟数时间
}kdk_timeinfo;

/**
 * @brief 获取系统支持的日期格式
 * 
 * @return char** 当前日期支持的日期格式，如"2020/08/01"；以NULL表示结尾，由alloc生成，需要被kdk_net_freeall回收；若获取出错，返回NULL；
 */
extern char** kdk_system_get_dateformat();

/**
 * @brief 设置日期格式
 * 
 * @param date 系统当前支持的日期格式
 * @return int 设置成功返回0，返回其他失败
 */
extern int kdk_system_set_dateformat(char *date);

/**
 * @brief 设置24小时制格式接口
 * 
 * @return int 设置成功返回0，返回其他失败
 */
extern int kdk_system_set_24_timeformat();

/**
 * @brief 设置12小时制格式接口
 * 
 * @return int 设置成功返回0，返回其他失败
 */
extern int kdk_system_set_12_timeformat();

/**
 * @brief 获取当前的日期格式
 * 
 * @return char* 用户的当前日期格式，返回的字符串需要被 free 释放
 */
extern char* kdk_system_get_now_dateformat();

/**
 * @brief 获取当前的时间格式
 * 
 * @return char* 用户的当前时间格式，返回的字符串需要被 free 释放
 */
extern char* kdk_system_get_now_timeformat();

/**
 * @brief 时间格式转换接口
 * 
 * @param struct tm *ptr 用户输入的日期和时间
 * @return kdk_dateinfo* 用户配置文件中的格式的日期，具体信息自取
 */
extern kdk_dateinfo *kdk_system_tran_dateformat(struct tm *ptr);

/**
 * @brief 获取当前时间接口
 * 
 * @return char* 与用户设置时间格式相同的当前时间，返回的字符串需要被 free 释放
 */
extern char* kdk_system_nowtime();

/**
 * @brief 获取当前日期接口
 * 
 * @return char* 与用户设置时间格式相同的当前时间，返回的字符串需要被 free 释放
 */
extern char* kdk_system_nowdate();

/**
 * @brief 获取当前星期接口
 * 
 * @return char* 用户的当前星期，返回的字符串需要被 free 释放
 */
extern char* kdk_system_shortweek();

/**
 * @brief 获取当前星期接口
 * 
 * @return char* 用户的当前星期，返回的字符串需要被 free 释放
 */
extern char* kdk_system_longweek();

/**
 * @brief 获取当前秒钟接口
 * 
 * @return char* 用户的当前的带秒钟数的时间，返回的字符串需要被 free 释放
 */
extern char* kdk_system_second();

/**
 * @brief 获取未登录的时间，星期，月份
 * 
 * @param char * 用户名
 * @return kdk_logn_dateinfo* 登录配置文件中的格式的日期，具体信息自取
 */
extern kdk_logn_dateinfo *kdk_system_logn_dateinfo(char *user);

/**
 * @brief 转换工具箱时间接口
 * 
 * @param char * 工具箱日期
 * @return char* 配置文件中的格式的日期，返回的字符串需要被 free 释放
 */
extern char* kdk_system_gjx_time(char *date);

/**
 * @brief 用于回收字符串列表
 * 
 * @param ptr 字符串列表
 */
extern inline void kdk_date_freeall(char **ptr);

/**
 * @brief 释放由kdk_system_tran_dateformat返回的日期信息结构体
 * 
 * @param date 由kdk_system_tran_dateformat返回的结构体指针
 */
extern void kdk_free_dateinfo(kdk_dateinfo *date);

/**
 * @brief 释放由kdk_system_logn_dateinfo返回的日期信息结构体
 * 
 * @param date 由kdk_system_logn_dateinfo返回的结构体指针
 */
extern void kdk_free_logn_dateinfo(kdk_logn_dateinfo *date);

/**
 * @brief 设置日期长格式
 * 
 * @param format 长格式的一种
 * @return int 设置成功返回0；其它值，失败。
 */
extern int kdk_system_set_long_dateformat(char *format);

/**
 * @brief 设置日期短格式
 * 
 * @param format 短格式中的一种
 * @return int 设置成功返回0；其它值，失败。
 */
extern int kdk_system_set_short_dateformat(char *format);

/**
 * @brief 获取当前的长格式日期
 * 
 * @return char* 用户的当前长格式的日期，返回的字符串需要被 free 释放
 */
extern char* kdk_system_get_longformat_date();

/**
 * @brief 获取当前的短格式日期
 * 
 * @return char* 用户的当前短格式的日期，返回的字符串需要被 free 释放
 */
extern char* kdk_system_get_shortformat_date();

/**
 * @brief 获取长格式
 * 
 * @return char*,用户设置的长格式,长格式有{yyyy MM dd,yy M d},默认yyyy MM dd 格式，返回的字符串需要被 free 释放
 */
extern char* kdk_system_get_longformat();

/**
 * @brief 获取短格式
 * 
 * @return char*,用户设置的短格式,短格式有{yyyy/MM/dd,yy/M/d,
 *          yyyy-MM-dd,yy-M-d,yyyy.MM.dd,yy.M.d},默认 yyyy/MM/dd 格式，返回的字符串需要被 free 释放
 */
extern char* kdk_system_get_shortformat();

/**
 * @brief 长格式转化
 * 
 * @param struct tm *,需要转化的时间。
 * @return char*,用户设置的长格式形式输出转化的日期，返回的字符串需要被 free 释放
 */
extern char* kdk_system_longformat_transform(struct tm *ptr);

/**
 * @brief 短格式转化
 * 
 * @param struct tm *,需要转化的时间。
 * @return char*,用户设置的短格式形式输出转化的日期。如 2023/06/09，返回的字符串需要被 free 释放
 */
extern char* kdk_system_shortformat_transform(struct tm *ptr);

/**
 * @brief 获取登录锁屏的时间,星期,长格式日期
 * 
 * @param user 用户名
 * @return kdk_logn_dateinfo,成员:date(char*);描述:登录配置文件中的长格式的日期;
 *          成员:time(char*);描述:登录配置文件中的格式的时间;成员:week(char*);描述:当前星期。
 */
extern kdk_logn_dateinfo *kdk_system_login_lock_dateinfo(char *user);

/**
 * @brief 时间格式转化
 * 
 * @param struct tm *,需要转化的时间。
 * @return char*,用户设置的时间形式输出转化的时间。
 */
extern kdk_timeinfo *kdk_system_timeformat_transform(struct tm *ptr);

/**
 * @brief 释放由kdk_system_timeformat_transform返回的时间信息结构体
 * 
 * @param date 由kdk_system_timeformat_transform返回的结构体指针
 */
extern void kdk_free_timeinfo(kdk_timeinfo *time);

/**
 * @brief 相对日期转换
 * @param struct tm 日期
 * 
 * @return char* 成功返回字符串{今天，明天}，失败返回NULL；返回的字符串需要free释放。
 */
extern char* kdk_system_tran_absolute_date(struct tm *ptr);

#ifdef __cplusplus
}
#endif

#endif

/**
 * \example lingmosdk-system/src/systemtime/test/kydate_test.c
 * 
 */

/**
  * @}
  */
