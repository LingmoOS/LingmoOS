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


#ifndef	LINGMOSDK_BASE_LOG_H__
#define	LINGMOSDK_BASE_LOG_H__


/** @defgroup 日志模块
  * @{
  */


/**
 * @file libkylog.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief LINGMOSDK日志模块
 * @version 0.1
 * @date 2021-10-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "sdkmarcos.h"

#define	KLOG_TRACE		8
#define	KLOG_DEBUG		7
#define	KLOG_INFO		6
#define	KLOG_NOTICE		5
#define	KLOG_WARNING	4
#define	KLOG_ERROR		3
#define	KLOG_CRIT		2
#define	KLOG_ALERT		1
#define	KLOG_EMERG		0

#ifdef	__cplusplus
extern "C"
{
#endif

/**
 * @brief 在调用的位置自动输出一串日志，表明主调方函数被调用
 * 
 */
#define	klog_trace(fmt , ...)	(klog(KLOG_TRACE , fmt , ##__VA_ARGS__))

/**
 * @brief 输出debug级别日志，不会自动添加换行符
 * 
 */
#define	klog_debug(fmt , ...)	(klog(KLOG_DEBUG , fmt , ##__VA_ARGS__))

/**
 * @brief 输出info级别日志，不会自动添加换行符
 * 
 */
#define	klog_info(fmt , ...)	(klog(KLOG_INFO , fmt , ##__VA_ARGS__))

/**
 * @brief 输出notice级别日志，不会自动添加换行符
 * 
 */
#define	klog_notice(fmt , ...)	(klog(KLOG_NOTICE , fmt , ##__VA_ARGS__))

/**
 * @brief 输出warning级别日志，不会自动添加换行符
 * 
 */
#define	klog_warning(fmt , ...)	(klog(KLOG_WARNING , fmt , ##__VA_ARGS__))

/**
 * @brief 输出error级别日志，不会自动添加换行符。
 * 
 */
#define	klog_err(fmt , ...)		(klog(KLOG_ERROR , fmt , ##__VA_ARGS__))

/**
 * @brief 输出crit级别日志，不会自动添加换行符。该等级日志说明程序发生了重大问题，需要立即修复。
 * 
 */
#define	klog_crit(fmt , ...)	(klog(KLOG_CRIT , fmt , ##__VA_ARGS__))

/**
 * @brief 输出alert级别日志，不会自动添加换行符。该等级日志说明系统发生了极为严重的错误，需要立即修复。
 * 
 */
#define	klog_alert(fmt , ...)	(klog(KLOG_ALERT , fmt , ##__VA_ARGS__))

/**
 * @brief 输出emerg级别日志，不会自动添加换行符。该等级日志说明系统发生了无法挽回的故障，必须立即停止。
 * 
 */
#define	klog_emerg(fmt , ...)	(klog(KLOG_EMERG , fmt , ##__VA_ARGS__))

#ifndef NOCALLTRACE
	#define	klog_calltrace()		(klog_trace("[CALLTRACE]Enter %s\n" , __FUNCTION__))
#else
	#define	klog_calltrace()
#endif

/**
 * @brief 内部使用函数，不应在外部被调用
 * 
 */
#define	klog(lvl , fmt , ...)	kdk_logger_write(lvl , __FILE__ , __FUNCTION__ , __LINE__ , fmt , ##__VA_ARGS__)

/**
 * @brief 初始化日志记录，也可以不调用该函数直接使用上方日志记录的宏，若以此方式运行，则程序会使用默认的日志配置文件
 * 
 * @param ini：日志配置文件路径，若传入NULL则会使用默认的日志配置文件
 * @return int，0表示成功，非0表示失败
 */
extern int	kdk_logger_init(const char *ini);

/**
 * @brief 在异步写入的方式下，可以调用该函数手动将缓存区中的日志写入文件中
 * 
 */
extern void	kdk_logger_flush() DESTRUCTOR;	//当使用异步日志记录方式时，该函数可以手动将日志刷新到文件中

/**
 * @brief 内部使用函数，不应在外部被调用
 * 
 */
extern int	kdk_logger_write(int lvl , const char *filename , const char *func , int linenum , const char *fmt , ...) NOTNULL() CHECK_FMT(5 , 6);

/**
 * @brief 设置日志的存储目录，若不设置，非root程序会被记录在~/.log下，root程序会被记录在/var/log下
 * 
 * @param dpath 
 * @return int 
 */
extern int kdk_logger_setdir(const char* dpath);

/**
 * @brief 设置日志输出自动换行
 * 
 * @param autowarp 1表示启用启动换行，0表示禁止自动换行
 */
extern void kdk_logger_set_autowrap(int autowarp);
#ifdef	__cplusplus
}
#endif

#endif


/**
 * \example lingmosdk-base/src/log/test/test-log.c
 * \example lingmosdk-base/src/log/test/test-pressure.c
 * \example lingmosdk-base/src/log/test/test-setdir.c
 */

/**
  * @}
  */
