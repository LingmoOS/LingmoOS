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


#include "write.h"
#include "klog_mqueue.h"
#include "writeFile.h"
#include <stdarg.h>
#include <errno.h>
#include <syslog.h>
#include <systemd/sd-journal.h>

static inline void	writeSyslog(int lvl , const char *message)
{
#ifndef __loongarch__
	sd_journal_send("MESSAGE=%s", message , "PRIORITY=%i" , lvl ,\
							"SYSLOG_FACILITY=%i" , 19 , NULL);
#else
	// TODO: loongarch64 's syslog write
	return;
#endif
}

static int writeSpecFile(int lvl , const char *filename , const char *func , int linenum , const char *message)
{
	char buffer[KLOG_MAXMSGSIZE + 1]	= {0};
	formatMessage(lvl , filename , func , linenum , message , buffer , KLOG_MAXMSGSIZE);
	if (logger->stype == SYNC)
	{
		if (logger->levelBasedStorage)
			return writeFile(lvl , buffer , strlen(buffer)) || fflush(logger->fp.classfiedfp[lvl]);
		else
			return writeFile(lvl , buffer , strlen(buffer)) || fflush(logger->fp.commonfp);
	}
	else
	{
		insertMessage(lvl , buffer);
	}
	return 0;
}

static void writeStdout(int lvl , const char *filename , const char *func , int linenum , const char *message)
{
	char buffer[KLOG_MAXMSGSIZE + 1]	= {0};
	formatMessage(lvl , filename , func , linenum , message , buffer , KLOG_MAXMSGSIZE);
	fprintf(stdout , "%s" , buffer);
}

int writeLog(int lvl , const char *filename , const char *func , int linenum , const char *message)
{
	switch (logger->otype)
	{
		case OUT_SYSLOG:
		{
			writeSyslog(lvl , message);
		}break;
		case OUT_SPECFILE:{
			writeSpecFile(lvl , filename , func , linenum , message);
		}break;
		case OUT_STDOUT:{
			writeStdout(lvl , filename , func , linenum , message);
		}break;
		default:{
			printf("输出类别无效！%d\n" , logger->otype);
			return EINVAL;
		}break;
	}

	return 0;	
}
