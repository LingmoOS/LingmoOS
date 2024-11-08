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


#include <stdio.h>
#include <string.h>
#include <sys/timerfd.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "libkytimer.h"
#include "sdkmarcos.h"

static void* timerCoreThread(void* data);		//定时器循环核心
static KTimerNode* findNodeByFD(unsigned int fd);	//根据fd查找节点
static KTimerNode* newTimerNode();
static void freeTimerNode(KTimerNode* node);
static int	testNodeInList(KTimerNode* node);
static void	insertIntoList(KTimerNode* node);	//插入节点，按照间隔时间的顺序从小到大排序
static void	deleteFromList(KTimerNode* node , int locked);		//从链表中删除节点
static pthread_t	g_coreThreadID;		//核心循环线程
static KTimerNode*	g_list	= NULL;
static int	epollfd;	//事件池句柄
static int	curTimerCounts;	//当前已注册定时器数量

int	kdk_timer_init();	//初始化定时器核心
size_t 	kdk_timer_start(unsigned int intervalms, time_handler callback, KTimerAttribute attr, KTimerType type, void* userdata, int freeOnDelete);	//注册并开始一个定时器，返回定时器句柄号
void	kdk_timer_stop(size_t timerfd);	//停止一个定时器
void	kdk_timer_reset(size_t timerfd , unsigned int intervalms);
void	kdk_timer_destroy() DESTRUCTOR;		//销毁定时器核心

#ifndef	TFD_TIMER_CANCEL_ON_SET
#define	TFD_TIMER_CANCEL_ON_SET	(1 << 1)
#endif

int kdk_timer_init()
{
	if (g_coreThreadID > 0)
		return 0;

	epollfd	= epoll_create1(EPOLL_CLOEXEC);
	if (epollfd <= 0)
	{
		printf("Epoll事件池创建失败！%s\n" , strerror(errno));
		return errno;
	}

	if (pthread_create(&g_coreThreadID , NULL , timerCoreThread , NULL))
	{
		printf("kyTimer 定时器核心初始化失败：%s\n" , strerror(errno));
		return errno;
	}
	return 0;
}

size_t kdk_timer_start(unsigned int intervalms, time_handler callback, KTimerAttribute attr, KTimerType type, void* userdata, int freeOnDelete)
{
	if (g_coreThreadID <= 0 && kdk_timer_init())
	{
		printf("定时器注册失败：定时器全局句柄初始化失败！\n");
		return 0;
	}
	if (curTimerCounts >= KTIMER_MAXTIMERFD)
	{
		printf("定时器注册失败：定时器注册数量已超限！\n");
		return 0;
	}

	KTimerNode* node	= newTimerNode();
	if (!node)
	{
		printf("定时器注册失败：无法分配内存，%s\n" , strerror(errno));
		return 0;
	}

	node->callback	= callback;
	node->userdata	= userdata;
	node->intervalms	= intervalms;
	node->attr		= attr;
	node->type		= type;
	node->freeOnDelete	= freeOnDelete;

	if (type == KTIMER_ABSOLUTE)
		node->fd		= timerfd_create(CLOCK_MONOTONIC , TFD_CLOEXEC|TFD_NONBLOCK);
	else
		node->fd		= timerfd_create(CLOCK_REALTIME , TFD_CLOEXEC|TFD_NONBLOCK);

	if ((int)node->fd <= 0)
	{
		printf("定时器注册失败：timerfd_create失败，%s\n" , strerror(errno));
		freeTimerNode(node);
		return 0;
	}
	struct itimerspec tval;
	tval.it_value.tv_sec	= intervalms / 1000;
	tval.it_value.tv_nsec	= (intervalms % 1000) * 1000000;

	if (attr == KTIMER_PERIODIC)	//多次触发时设置interval项
	{
		tval.it_interval.tv_sec	= tval.it_value.tv_sec;
		tval.it_interval.tv_nsec	= tval.it_value.tv_nsec;
	}
	else
	{
		tval.it_interval.tv_sec	= 0;
		tval.it_interval.tv_nsec	= 0;
	}

	timerfd_settime(node->fd, 0, &tval, NULL);

	//注册到Epoll池中
	struct epoll_event ev;
	memset(&ev , 0 , sizeof(struct epoll_event));
	if (attr == KTIMER_SINGLESHOT)
		ev.events	= EPOLLIN | EPOLLONESHOT;
	else
		ev.events	= EPOLLIN;
	ev.data.ptr	= node;
	if (epoll_ctl(epollfd , EPOLL_CTL_ADD , node->fd , &ev))
	{
		printf("定时器注册失败：epoll_ctl错误，%s\n" , strerror(errno));
		freeTimerNode(node);
		return 0;
	}

	insertIntoList(node);

	return node->fd;
}

//停止一个定时器，并从列表中移除
void kdk_timer_stop(size_t timerfd)
{
	if (timerfd <= 0)
		return;
	KTimerNode* node	= findNodeByFD(timerfd);
	if (node)
	{
		if (epoll_ctl(epollfd , EPOLL_CTL_DEL , timerfd , NULL))
		{
			printf("无法注销文件句柄%zd : %s\n" , timerfd , strerror(errno));
			return;
		}
		deleteFromList(node , 0);
	}
	return;
}

//定时器核心线程，负责循环处理已到期的定时器
static void* timerCoreThread(void* data)
{
	pthread_detach(pthread_self());

	KTimerNode* knode	= NULL;
	int read_fds = 0;

	while (1)
	{
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE , NULL);
		pthread_testcancel();
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE , NULL);

		struct epoll_event activeEvs[KTIMER_MAXTIMERFD];
		read_fds	= epoll_wait(epollfd , activeEvs , KTIMER_MAXTIMERFD , -1);
		if (read_fds < 0)
		{
			//printf("epoll wait error , %s\n" , strerror(errno));
			continue;
		}
		for (int i = 0 ; i < read_fds ; i ++)
		{
			knode	= activeEvs[i].data.ptr;
			pthread_mutex_lock(&knode->lock);
			if (!testNodeInList(knode))
				continue;
			uint64_t dep;
			size_t rd = read(knode->fd , &dep , sizeof(uint64_t));
			if (rd <= 0)
				continue;

			if (knode && knode->attr != KTIMER_NEVER && knode->callback)
			{
#if 1
				pthread_attr_t attr;
				pthread_t tid;
				pthread_attr_init(&attr);
				pthread_attr_setdetachstate(&attr , PTHREAD_CREATE_DETACHED);
				pthread_create(&tid , &attr , knode->callback , knode->userdata);

				knode->freeOnDelete	= 0;
#else
				knode->callback(knode->userdata);
#endif
			}

			if (knode->attr == KTIMER_SINGLESHOT)	//单次触发的在触发后就从链表中移除
			{
				// FixMe: 如果设置了freeOnDelete，并且采用上面的线程执行模式，
				// 		  则会发生段错误。因此只能在启用线程执行模式的时候强制禁止
				// 		  freeOnDelete。
				deleteFromList(knode , 1);
			}
			if (testNodeInList(knode))
				pthread_mutex_unlock(&knode->lock);
		}
	}

	return NULL;
}

static KTimerNode* findNodeByFD(unsigned int fd)
{
	KTimerNode* tmp	= g_list;
	while (tmp)
	{
		if (tmp->fd == fd)
			return tmp;
		tmp	= tmp->next;
	}
	return NULL;
}

static int testNodeInList(KTimerNode* node)
{
	KTimerNode* pos	= g_list;
	while (pos)
	{
		if (pos == node)
			return 1;
		pos	= pos->next;
	}
	return 0;
}

static void	insertIntoList(KTimerNode* node)
{
	curTimerCounts ++;
	if (!g_list)
	{
		g_list	= node;
	}
	else
	{
		if (g_list->intervalms > node->intervalms)
		{
			node->next	= g_list;
			g_list	= node;
			return;
		}

		KTimerNode* cur		= g_list->next;
		KTimerNode* prev	= g_list;

		while (cur)
		{
			if (cur->intervalms > node->intervalms)
			{
				node->next	= cur;
				prev->next	= node;
				return;
			}
			prev	= cur;
			cur	= cur->next;
		}

		prev->next	= node;
		node->next	= NULL;
	}
	return;
}

static void deleteFromList(KTimerNode* node , int locked)
{
	if (!g_list)
		return;
	curTimerCounts --;
	if (!locked)
		pthread_mutex_lock(&node->lock);
	if (node == g_list)
	{
		g_list	= g_list->next;
		freeTimerNode(node);
		return;
	}

	KTimerNode* pos	= g_list;
	while (pos)
	{
		if (pos->next && pos->next == node)
		{
			pos->next	= pos->next->next;
			freeTimerNode(node);
			return;
		}
		pos	= pos->next;
	}
	return;
}

static KTimerNode* newTimerNode()
{
	KTimerNode *node = (KTimerNode*)calloc(1 , sizeof(KTimerNode));
	if (!node)
		return NULL;
	pthread_mutex_init(&node->lock , NULL);
	return node;
}

static void freeTimerNode(KTimerNode *node)
{
	if (! node)
		return;
	if (node->fd > 0)
		close(node->fd);
	if (node->userdata && node->freeOnDelete)
		free(node->userdata);
	free(node);
}

void kdk_timer_reset(size_t timerfd , unsigned int intervalms)
{
	KTimerNode* node	= findNodeByFD(timerfd);
	if (!node)
		return;
	node->intervalms	= intervalms;
	struct itimerspec tval;
	tval.it_value.tv_sec	= intervalms / 1000;
	tval.it_value.tv_nsec	= (intervalms % 1000) * 1000000;

	if (node->attr == KTIMER_PERIODIC)	//多次触发时设置interval项
	{
		tval.it_interval.tv_sec	= tval.it_value.tv_sec;
		tval.it_interval.tv_nsec	= tval.it_value.tv_nsec;
	}
	else
	{
		tval.it_interval.tv_sec	= 0;
		tval.it_interval.tv_nsec	= 0;
	}
	timerfd_settime(timerfd , 0 , &tval , NULL);
	return;
}

void kdk_timer_destroy()	//停止所有定时器，并销毁定时器核心
{
	//终止线程运行
	if (g_coreThreadID > 0)
		pthread_cancel(g_coreThreadID);

	//销毁链表
	KTimerNode* node	= g_list;
	while (node)
	{
		deleteFromList(node , 0);
		node	= g_list;
	}

	//关闭epoll池
	if (epollfd > 0)
		close(epollfd);
	return;
}
