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


#ifndef	__KLOG_CORE_H__
#define	__KLOG_CORE_H__
#include <stdio.h>
#include <pthread.h>
#include "libkylog.h"

#define	KLOG_DEFAULT_CONFPATH	"/etc/kylog.conf"

#define	KLOG_DEFAULT_MSGFLUSHINTERVAL	100
#define	KLOG_DEFAULT_MSGGROWTH			4096	//消息存储超阈值后单次增长率
#define	KLOG_DEFAULT_MSGGROWTHRESHOLD	204800	//消息增长阈值

#define	MLOCK(x)	(pthread_mutex_lock(x))
#define	MTRYLOCK(x)	(pthread_mutex_trylock(x))
#define	MUNLOCK(x)	(pthread_mutex_unlock(x))

// #ifndef RELEASE
// #define	OUTPUT(fmt , ...)	printf("[%s:%d]"fmt , __FUNCTION__ , __LINE__ , ##__VA_ARGS__)
// #else
// #define OUTPUT(fmt, ...)
// #endif

#define	KLOG_MAXPATHLEN	1024		//文件路径最大长度
#define	KLOG_MAXMSGSIZE	2048		//每条消息最大长度
#define	KLOG_MAXPRIVALSIZE	128		//自定义内容最大长度
#define	KLOG_PROCESSNAME_LEN	128		//可执行文件名最大长度
#define	KLOG_MAXDATELEN	64			//日期最大长度
#define	KLOG_CONF_GROUPSIZE	64		//配置文件Group最大长度
#define	KLOG_CONF_KEYSIZE	64		//配置文件Key最大长度
#define	KLOG_CONF_VALUESIZE	KLOG_MAXPATHLEN	//配置文件value最大长度

#define	KLOG_OPT_FORMAT		0x0001
#define	KLOG_OPT_LOGTYPE	0x0002
#define	KLOG_OPT_SYNC		0x0003
#define	KLOG_OPT_OUTPUT		0x0004
#define	KLOG_OPT_SPECFILE	0x0005
#define	KLOG_OPT_SPECCONTENT	0x0006

#ifndef CORE_DEFINE
#define	CORE_DEFINE
enum logIdentifier{
	LT_USER = 0,
	LT_LOCAL3 = 1,
	LT_SYSLOG = 2,
	LT_SPEC,
	LTENUMMAX
};	//日志类别

enum syncType{
	ASYNC=1,	// 异步模式，会较大程度提高日志写入速度，减小日志写入带来的IO等待时间，但有可能出现程序崩溃时，日志尚未写入的情况
	SYNC,		// 同步模式，每次的写入都会等待写入事件完成
	STENUMMAX
};	//记录方式

enum outputType{
	OUT_SYSLOG=0,
	OUT_SPECFILE,
	OUT_STDOUT,
	OTENUMMAX
};	//日志输出位置
#endif

typedef struct KLogger
{
	union
	{
		FILE*	commonfp;
		FILE*	classfiedfp[8];
	}fp;
	enum logIdentifier	identer;	// 日志输出的标识符
	enum syncType	stype;	// 标明在spec模式下，刷入日志的模式，异步/同步
	enum outputType	otype;	// 标明日志输出的位置，可以输出到syslog/指定文件/标准输出；目前syslog尚未验证
	int			levelBasedStorage;	//按照等级分类存储
	int			levelBasedContainHigherLevel;	//按等级分类存储时，低优先级日志是否需要包含高优先级日志（数字越小优先级越高）
	int			level;		//日志记录等级
	int			autowrap;	// 自动换行
	pid_t		pid;
	char		stringPID[16];
	char		rootPath[KLOG_MAXPATHLEN + 1];
	char		specName[KLOG_MAXPATHLEN + 1];
	union
	{
		char		commonlogfileName[KLOG_MAXPATHLEN + 1];	//自定义日志文件路径
		char		classfiedfileName[8][KLOG_MAXPATHLEN + 1];
	}logfileName;
	char		processName[KLOG_PROCESSNAME_LEN + 1];		//可执行文件名称
	char		specLogType[KLOG_MAXPRIVALSIZE + 1];	//自定义记录类型

	pthread_mutex_t*	mlock;
}KLogger;

#define	DEFAULT_LOGTYPE		LT_LOCAL3
#define	DEFAULT_OUTPUTTYPE	OUT_SPECFILE
#define	DEFAULT_SYNCTYPE	SYNC
#define	DEFAULT_LOGLEVEL	KLOG_DEBUG

extern int	initKLogger(int cid);
extern int setRootDir(const char* dpath) NOTNULL();
extern void	destroyKLogger();
extern void set_autowrap(int autowrap);
extern void append_wrap(char *message);

extern KLogger* logger;

#endif
