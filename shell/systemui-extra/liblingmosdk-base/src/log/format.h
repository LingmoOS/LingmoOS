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


#ifndef	__KLOG_FMT_H__
#define	__KLOG_FMT_H__
#include <stdbool.h>
#include "core.h"

typedef struct format
{
	bool	vis_identifier;
	bool	vis_execName;
	bool	vis_pid;
	bool	vis_tid;
	bool	vis_filename;
	bool	vis_funcline;
}PrintFormat;

extern PrintFormat	klog_printformat;

#ifndef KLOG_FORMATSET
#define	KLOG_FORMATSET
#define	FORMAT_LOGTYPE	0x0001
#define	FORMAT_PROCESSNAME	0x0002
#define	FORMAT_PID		0x0004
#define	FORMAT_FILENAME	0x0008
#define	FORMAT_LINENUM	0x0010
#endif

extern int	getRecordDate(char* date) NOTNULL();
extern void	loadFormatOptions();
extern int	formatMessage(int lvl , const char *filename , const char *func , int linenum , const char *message , char* result , unsigned int resultSize) NOTNULL();

#endif
