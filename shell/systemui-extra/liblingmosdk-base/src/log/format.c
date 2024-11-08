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


#include "format.h"
#include "libkyconf.h"
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define FILENAMESIZE 512
#define FUNCSIZE 128
#define LINESIZE 10
#define BUFFERSIZE 1398 //2048-512-128-10

PrintFormat klog_printformat;

extern const char* stringLevel[8];
extern const char* stringLType[LTENUMMAX];

int	getRecordDate(char* date) NOTNULL();
int	formatMessage(int lvl , const char *filename , const char *func , int linenum , const char *message , char* result , unsigned int resultSize) NOTNULL();

int getRecordDate(char* date)
{
	time_t now;
	time(&now);
	if (ctime_r(&now , date))
		date[strlen(date) - 1]	= '\0';
	return 0;
}

void loadFormatOptions(int id)
{
	klog_printformat.vis_execName	= atoi(kdk_conf_get_value(id, "FORMAT" , "f_processname")) ? true : false;
	klog_printformat.vis_filename	= atoi(kdk_conf_get_value(id, "FORMAT" , "f_filename")) ? true : false;
	klog_printformat.vis_funcline	= atoi(kdk_conf_get_value(id, "FORMAT" , "f_funcline")) ? true : false;
	klog_printformat.vis_identifier	= atoi(kdk_conf_get_value(id, "FORMAT" , "f_identifier")) ? true : false;
	klog_printformat.vis_pid		= atoi(kdk_conf_get_value(id, "FORMAT" , "f_pid")) ? true : false;
	klog_printformat.vis_tid		= atoi(kdk_conf_get_value(id, "FORMAT" , "f_tid")) ? true : false;
}

int	formatMessage(int lvl , const char *filename , const char *func , int linenum , const char *message , char* result , unsigned int resultSize)
{
	char buffer[KLOG_MAXMSGSIZE + 1]	= {"["};	//TODO:这里开4097字节可能有越界风险
	char* pos	= buffer;
	pos ++;

	//[类型.等级]
	if (klog_printformat.vis_identifier)
	{
		if (logger->identer != LT_SPEC)
		{
			memcpy(pos , stringLType[logger->identer] , strlen(stringLType[logger->identer]) * sizeof(char));
			pos	+= strlen(stringLType[logger->identer]);
		}
		else
		{
			memcpy(pos , logger->specLogType , strlen(logger->specLogType) * sizeof(char));
			pos	+= strlen(logger->specLogType);
			strcat(pos , ".");
			pos ++;
		}
	}
	memcpy(pos , stringLevel[lvl] , strlen(stringLevel[lvl]) * sizeof(char));
	pos	+= strlen(stringLevel[lvl]);
	strcpy(pos , "] ");
	pos	+= 2;

	//[日期]
	strcpy(pos , "[");
	pos	+= 1;
	char nowtime[KLOG_MAXDATELEN + 1]	= {0};
	getRecordDate(nowtime);
	memcpy(pos , nowtime , strlen(nowtime) * sizeof(char));
	pos	+= strlen(nowtime);
	strcpy(pos , "] ");
	pos	+= 2;

	//[进程名:PID-TID]
	if (klog_printformat.vis_execName || klog_printformat.vis_pid)
	{
		strcpy(pos , "[");
		pos ++;

		if (klog_printformat.vis_execName)
		{
			memcpy(pos , logger->processName , strlen(logger->processName) * sizeof(char));
			pos	+= strlen(logger->processName);
		}
		if (klog_printformat.vis_pid)
		{
			if (__glibc_likely(klog_printformat.vis_execName))
			{
				strcpy(pos , ":");
				pos ++;
			}
			memcpy(pos , logger->stringPID , strlen(logger->stringPID) * sizeof(char));
			pos	+= strlen(logger->stringPID);
		}
		if (klog_printformat.vis_tid)
		{
			if (__glibc_likely(klog_printformat.vis_pid))
			{
				strcpy(pos , "-");
				pos ++;
			}
			char tid[32]	= {0};
			sprintf(tid , "%lu" , pthread_self());
			memmove(pos , tid , strlen(tid) * sizeof(char));
			pos	+= strlen(tid);
		}
		strcpy(pos , "] ");
		pos	+= 2;
	}

	//TODO:[自定义]

	//[文件:函数-行号]
	if (klog_printformat.vis_filename || klog_printformat.vis_funcline)
	{
		strcpy(pos , "[");
		pos	+= 1;
		if (klog_printformat.vis_filename)
		{
			size_t len = strlen(filename) * sizeof(char);
			memcpy(pos , filename , FILENAMESIZE > len ? len : FILENAMESIZE);
			pos	+= strlen(filename);
		}
		if (klog_printformat.vis_funcline)
		{
			if (__glibc_likely(klog_printformat.vis_filename))
			{
				strcpy(pos , ":");
				pos	+= 1;
			}
			size_t len = strlen(func) * sizeof(char);
			memcpy(pos , func , FUNCSIZE > len ? len : FUNCSIZE);
			pos	+= strlen(func);
			char line[10]	= {0};
			snprintf(line , 9 , "-%d" , linenum);
			len = strlen(line) * sizeof(char);
			memcpy(pos , line , LINESIZE > len ? len : LINESIZE);
			pos	+= strlen(line);
		}
		strcpy(pos , "] ");
		pos	+= 2;
	}
	// size_t remainMsgSize	= KLOG_MAXMSGSIZE - strlen(buffer);
	// size_t remainMsgSize = KLOG_MAXMSGSIZE - ((pos - buffer) / sizeof(char));
	size_t rawMsgSize	= strlen(message) * sizeof(char);
	memcpy(pos , message , BUFFERSIZE > rawMsgSize ? rawMsgSize : BUFFERSIZE);
	memcpy(result , buffer , resultSize * sizeof(char));
	return 0;
}
