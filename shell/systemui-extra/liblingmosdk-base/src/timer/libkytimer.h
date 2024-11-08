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


#ifndef	LINGMOSDK_BASE_TIMER_H__
#define	LINGMOSDK_BASE_TIMER_H__


/** @defgroup 定时器模块
  * @{
  */


/**
 * @file libkytimer.h
 * @author liuyunhe (liuyunhe@kylinos.cn)
 * @brief LINGMOSDK C语言定时器模块
 * @version 0.1
 * @date 2021-10-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdlib.h>
#include <pthread.h>

typedef enum
{
	KTIMER_SINGLESHOT = 0,
	KTIMER_PERIODIC = 1,
	KTIMER_NEVER = 2
}KTimerAttribute;

typedef enum
{
	KTIMER_ABSOLUTE = 0,
	KTIMER_RELATIVE
}KTimerType;

#define	KTIMER_MAXTIMERFD	1000		//最多支持注册1000个定时器

typedef void* (*time_handler)(void* user_data);	//回调函数规则

typedef struct _KTimerNode{
	pthread_mutex_t	lock;
	size_t			fd;			//序列号
	time_handler	callback;	//到期后执行的函数
	int				freeOnDelete;	//删除定时器时自动释放userdata
	void*			userdata;	//callback使用的参数，外部使用alloc分配内存，在delete定时器的时候会根据配置自动释放
	unsigned int	intervalms;	//定时器间隔时间，单位毫秒
	KTimerAttribute	attr;		//触发类型，单次触发或多次触发
	KTimerType		type;		//时钟类型，绝对时间还是相对时间
	struct _KTimerNode*	next;
}KTimerNode;

#ifdef	__cplusplus
extern "C"
{
#endif
/**
 * @brief 初始化定时器核心组件
 * 
 * @return int 成功返回0，失败返回错误码
 */
extern int	kdk_timer_init();

/**
 * @brief 启动一个定时器
 * 
 * @param intervalms 定时器时间，以毫秒为单位
 * @param callback 定时器到期后触发的回调函数指针
 * @param attr 定时器属性，KTIMER_SINGLESHOT表示一次性定时器；KTIMER_PERIODIC表示周期性定时器；KTIMER_NEVER表示不会被触发的定时器
 * @param type 定时器类型，KTIMER_ABSOLUTE表示绝对时间定时器，修改系统时间不会影响定时器的时间；KTIMER_RELATIVE表示相对时间定时器，修改系统时间会影响定时器时间
 * @param userdata 指向用户数据的指针
 * @param freeOnDelete [未启用]
 * @return size_t 定时器的ID
 */
extern size_t 	kdk_timer_start(unsigned int intervalms, time_handler callback, KTimerAttribute attr, KTimerType type, void* userdata, int freeOnDelete);

/**
 * @brief 停止给定的定时器
 * 
 * @param timerfd 由kdk_timer_start返回的定时器ID
 */
extern void	kdk_timer_stop(size_t timerfd);

/**
 * @brief 重置定时器时间至intervalms，以毫秒为单位
 * 
 * @param timerfd 由kdk_timer_start返回的定时器ID
 * @param intervalms 需要调整的时间间隔，以ms为单位
 */
extern void	kdk_timer_reset(size_t timerfd , unsigned int intervalms);

/**
 * @brief 销毁定时器核心
 * 
 */
extern void	kdk_timer_destroy();
#ifdef	__cplusplus
}
#endif

#endif	//LINGMOSDK_BASE_TIMER_H__


/**
 * \example lingmosdk-base/src/timer/test/test-kytimer.c
 * 
 */

/**
  * @}
  */
