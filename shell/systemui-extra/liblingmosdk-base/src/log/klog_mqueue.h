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


#ifndef __KLOG_MQ_H__
#define	__KLOG_MQ_H__
#include "core.h"
#include <pthread.h>
#include <stdbool.h>

typedef struct _KL_MessageNode{
	int	lvl;
	unsigned int bufSize;
	char buf[KLOG_MAXMSGSIZE];
}KL_MessageNode;

typedef struct _KL_MessageQueue{
	pthread_mutex_t* mlock;		//用于在操作队列时锁定
	int				autoIncrementQueueSize;	//是否自动扩充消息队列
	unsigned long	maxmessage;	//当前最大可接受消息数量，该数量超限后会以每次KLOG_DEFAULT_MSGGROWTH的数量扩充
	unsigned long	mnum;	//队列中消息的数量
	pthread_t thread_id;	//后台刷新线程ID
	int interval;			//刷新间隔
	KL_MessageNode**	message;	//消息缓冲
}KLMessageQueue;

extern int	initMessageQueue(int flushInterval , int autoIncrement);
extern int	insertMessage(int lvl , const char *message);
extern int	flushMessageQueue(int locked);
extern void	emptyMessageQueue();
extern void	destroyMessageQueue();

#endif
