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


#include "writeFile.h"
#include "core.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static int verify_file(char *pFileName)
{
    return 1;
}

int writeFile(int lvl , const char *message , unsigned int len)
{
	if (logger->levelBasedStorage)
	{
		if (!logger->fp.classfiedfp[lvl])
		{
			logger->fp.classfiedfp[lvl]	= fopen(logger->logfileName.classfiedfileName[lvl] , "at");
			if (!logger->fp.classfiedfp[lvl])
			{
				printf("无法打开日志文件%s：%s\n" , logger->logfileName.classfiedfileName[lvl] , strerror(errno));
				return errno;
			}
		}
		if (fwrite(message , sizeof(char) , len , logger->fp.classfiedfp[lvl]) < len)
		{
			printf("日志写入错误，%s\n" , strerror(errno));
			return errno;
		}
		if (logger->levelBasedContainHigherLevel)	//当要求低优先级日志需要包含更高优先级的日志时，逐个写入日志文件
		{
			for (int i = lvl + 1 ; i < 8 ; i ++)
			{
				if (!logger->fp.classfiedfp[i])
				{
					logger->fp.classfiedfp[i]	= fopen(logger->logfileName.classfiedfileName[i] , "at");
					if (!logger->fp.classfiedfp[i])
					{
						printf("无法打开日志文件%s：%s\n" , logger->logfileName.classfiedfileName[i] , strerror(errno));
						continue;
					}
				}
				if (fwrite(message , sizeof(char) , len , logger->fp.classfiedfp[i]) < len)
				{
					printf("日志写入错误，%s\n" , strerror(errno));
					return errno;
				}
			}
		}
	}
	else
	{
		if (!logger->fp.commonfp)
		{
			char canonical_filename[PATH_MAX] = "\0";
			memset(canonical_filename,0,PATH_MAX);
			realpath(logger->logfileName.commonlogfileName, canonical_filename);
			if(!verify_file(canonical_filename))
			{
				return errno;
			}
			logger->fp.commonfp	= fopen(canonical_filename, "at");
			if (!logger->fp.commonfp)
			{
				printf("无法打开日志文件：%s\n" , strerror(errno));
				return errno;
			}
		}
		if (fwrite(message , sizeof(char) , len , logger->fp.commonfp) < len)
		{
			printf("日志写入错误，%s\n" , strerror(errno));
			return errno;
		}
	}
	return 0;
}
