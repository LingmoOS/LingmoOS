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


#include "klog_mqueue.h"
#include "writeFile.h"
#include "core.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

int	initMessageQueue(int flushInterval , int autoIncrement);
int	insertMessage(int lvl , const char *message);
int	flushMessageQueue(int locked);
void	emptyMessageQueue();
void	destroyMessageQueue();
void*	startMQDaemon(void* msec);
void	recycle() DESTRUCTOR;

static KLMessageQueue* pMQ;

int initMessageQueue(int flushInterval , int autoIncrement)
{
	if (pMQ)
		return 0;
	pMQ	= (KLMessageQueue*)calloc(1 , sizeof(KLMessageQueue));
	if (!pMQ)
	{
		return errno;
	}
	pMQ->interval	= flushInterval > 0 ? flushInterval : KLOG_DEFAULT_MSGFLUSHINTERVAL;
	pMQ->autoIncrementQueueSize	= autoIncrement != 0 ? 1 : 0;

	pMQ->mnum	= 0;
	pMQ->mlock	= (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if (!pMQ->mlock)
	{
		printf("消息队列锁分配失败！%s" , strerror(errno));
		return errno;
	}
	pthread_mutex_init(pMQ->mlock , NULL);

	pMQ->message	= (KL_MessageNode**)calloc(1 , sizeof(KL_MessageNode*) * KLOG_DEFAULT_MSGGROWTH);
	if (!pMQ->message)
	{
		printf("消息缓冲内存申请失败：%s\n" , strerror(errno));
		return errno;
	}
	pMQ->maxmessage	= KLOG_DEFAULT_MSGGROWTH;

	//创建异步刷新线程
	pMQ->thread_id	= 0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if (pthread_create(&pMQ->thread_id , &attr , startMQDaemon , &pMQ->interval))
	{
		printf("异步刷新线程创建失败：%s\n" , strerror(errno));
		return errno;
	}

	return 0;
}

int	insertMessage(int lvl , const char *message)
{
	if (!pMQ && initMessageQueue(0 , 0))
		return errno;
	int retv	= 0;

	//构造MessageNode
	KL_MessageNode *node	= (KL_MessageNode*)calloc(1 , sizeof(KL_MessageNode));
	if (!node)
	{
		retv	= errno;
		goto clean_up;
	}
	node->lvl		= lvl;
	node->bufSize	= strlen(message);
	node->bufSize	= node->bufSize > sizeof(node->buf) ? sizeof(node->buf) : node->bufSize;
	memcpy(node->buf , message , node->bufSize * sizeof(char));

	//插入消息队列
	MLOCK(pMQ->mlock);
	if (__glibc_unlikely(pMQ->mnum >= pMQ->maxmessage))	//当前消息缓冲已满，扩充缓冲区
	{
		// OUTPUT("缓冲区满\n");
		if (pMQ->autoIncrementQueueSize)
		{
			KL_MessageNode** old	= pMQ->message;
			unsigned long nextsize	= pMQ->maxmessage;
			if (nextsize >= KLOG_DEFAULT_MSGGROWTHRESHOLD)	//若已达到指定阈值，则降低为缓慢扩充模式
					nextsize	+= KLOG_DEFAULT_MSGGROWTH;
			else
			{
				nextsize	<<= 1;	//以乘二的速率扩充
			}
		
			pMQ->message	= (KL_MessageNode**)realloc(pMQ->message , sizeof(KL_MessageNode*) * nextsize);
			if (!pMQ->message)
			{
				char errmsg[1024]	= {0};
				sprintf(errmsg , "[SYSTEM.emerg]消息队列缓冲扩充失败：%s\n" , strerror(errno));
				// OUTPUT("[SYSTEM.emerg]消息队列缓冲扩充失败：%s\n" , strerror(errno));
				if (logger->levelBasedStorage)
				{
					fwrite(errmsg , sizeof(char) , strlen(errmsg) , logger->fp.classfiedfp[KLOG_EMERG]);
					fflush(logger->fp.classfiedfp[KLOG_EMERG]);
					if (logger->levelBasedContainHigherLevel)
					{
						for (int i = KLOG_EMERG + 1 ; i < 8 ; i ++)
						{
							fwrite(errmsg , sizeof(char) , strlen(errmsg) , logger->fp.classfiedfp[i]);
							fflush(logger->fp.classfiedfp[i]);
						}
					}
				}
				else
				{
					fwrite(errmsg , sizeof(char) , strlen(errmsg) , logger->fp.commonfp);
					fflush(logger->fp.commonfp);
				}
				pMQ->message	= old;
				flushMessageQueue(1);
			}
			else
			{
				pMQ->maxmessage	= nextsize;
				// OUTPUT("扩充缓冲至%lu\n" , pMQ->maxmessage);
			}
		}
		else
		{
			// OUTPUT("缓冲区满，正在刷新缓冲区\n");
			flushMessageQueue(1);
		}
	}
		

	pMQ->message[pMQ->mnum]	= node;
	pMQ->mnum ++;

	MUNLOCK(pMQ->mlock);

clean_up:
	return retv;
}

int flushMessageQueue(int locked)
{
	if (!pMQ && initMessageQueue(0 , 0))
		return errno;
	if (!locked)
		MLOCK(pMQ->mlock);
	unsigned long msgcounts	= pMQ->mnum;
	if (pMQ->mnum == 0)
	{
		if (!locked)
			MUNLOCK(pMQ->mlock);
		return 0;
	}
	//将整个链表内存拷贝到另一个位置，然后清空原链表
	KL_MessageNode** list	= pMQ->message;
	if (pMQ->autoIncrementQueueSize)
	{
		pMQ->maxmessage	>>= 1;		//折半重新开始扩充
		pMQ->message	= (KL_MessageNode**)calloc(1 , sizeof(KL_MessageNode*) * pMQ->maxmessage);
		if (!pMQ->message)
		{
			char errmsg[1024]	= {0};
			sprintf(errmsg , "[SYSTEM.emerg]消息队列缩减失败：%s\n" , strerror(errno));
			if (logger->levelBasedStorage)
			{
				fwrite(errmsg , sizeof(char) , strlen(errmsg) , logger->fp.classfiedfp[KLOG_EMERG]);
				fflush(logger->fp.classfiedfp[KLOG_EMERG]);
				if (logger->levelBasedContainHigherLevel)
				{
					for (int i = KLOG_EMERG + 1 ; i < 8 ; i ++)
					{
						fwrite(errmsg , sizeof(char) , strlen(errmsg) , logger->fp.classfiedfp[i]);
						fflush(logger->fp.classfiedfp[i]);
					}
				}
			}
			else
			{
				fwrite(errmsg , sizeof(char) , strlen(errmsg) , logger->fp.commonfp);
				fflush(logger->fp.commonfp);
			}
			pMQ->maxmessage	= 0;
		}
		// OUTPUT("消息队列缓冲重置为%lu\n" , pMQ->maxmessage);
	}
	else
	{
		pMQ->message	= (KL_MessageNode**)calloc(1 , sizeof(KL_MessageNode*) * pMQ->maxmessage);
	}
	
	pMQ->mnum	= 0;
	if (!locked)
		MUNLOCK(pMQ->mlock);
	//处理拷贝后的内容，逐条写入
	KL_MessageNode* node	= NULL;
	for (unsigned long i = 0 ; i < msgcounts ; i ++)
	{
		node	= list[i];
		if (writeFile(node->lvl , node->buf , node->bufSize))
			insertMessage(node->lvl , node->buf);		//写入不成功的，重新插入队列
		free(node);
	}
	free(list);
	if (logger->levelBasedStorage)
	{
		for (int i = 0 ; i < 8 ; i ++)
			fflush(logger->fp.classfiedfp[i]);
	}
	else
		fflush(logger->fp.commonfp);
	return 0;
}

void emptyMessageQueue()
{
	if (!pMQ)
		return;
	MLOCK(pMQ->mlock);
	for (unsigned long i = 0 ; i < pMQ->mnum ; i ++)
	{
		free(pMQ->message[i]);
		pMQ->message[i]	= NULL;
	}
	pMQ->mnum	= 0;
	MUNLOCK(pMQ->mlock);
}

void destroyMessageQueue()
{
	if (!pMQ)
		return;
	pthread_cancel(pMQ->thread_id);
	pthread_join(pMQ->thread_id ,NULL);
	while (pMQ->mnum)
	{
		flushMessageQueue(0);
	}
	pthread_mutex_destroy(pMQ->mlock);
	free(pMQ->mlock);
	free(pMQ);
	pMQ	= NULL;
}

void recycle()
{
	destroyMessageQueue();
}

void* startMQDaemon(void* msec)
{
	int interval	= *(int*)msec;
	while (1)
	{
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE , NULL);
		pthread_testcancel();
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE , NULL);
		usleep(interval * 1000);
		flushMessageQueue(0);
	}
}
