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


#include "core.h"
#include "klog_dump.h"
#include <kerr.h>
#include <libkyconf.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "dbus/dbus.h"
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>

#define PATHSIZE 1024

KLogger* logger;
const char* stringLevel[8] = {"EMERG", "ALERT", "CRIT", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};
const char* stringLType[LTENUMMAX] = {"user." , "local3." , "syslog."};

static int verify_file(char *pFileName)
{
    return 1;
}

static int _call_method(const char *path)
{
	DBusError err;
	DBusConnection *conn;
	int ret;
	// initialise the errors
	dbus_error_init(&err);

	// connect to the bus
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (dbus_error_is_set(&err))
	{
		// fprintf(stderr, "Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
		return -1;
	}
	if (NULL == conn)
	{
		return -1;
	}

	DBusMessage *msg;
	DBusMessageIter args;
	DBusPendingCall *pending;
	int result = 0;

	msg = dbus_message_new_method_call("com.lingmosdk.base",			// target for the method call
									   "/com/lingmosdk/base/logrotate", // object to call on
									   "com.lingmosdk.base.logrotate",	// interface to call on
									   "setConfig");				// method name
	if (NULL == msg)
	{
		// fprintf(stderr, "Message Null\n");
		return -1;
	}

	// append arguments
	dbus_message_iter_init_append(msg, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &path))
	{
		// fprintf(stderr, "Out Of Memory!\n");
		return -1;
	}

	// send message and get a handle for a reply
	if (!dbus_connection_send_with_reply(conn, msg, &pending, -1))
	{ // -1 is default timeout
		// fprintf(stderr, "Out Of Memory!\n");
		return -1;
	}
	if (NULL == pending)
	{
		// fprintf(stderr, "Pending Call Null\n");
		return -1;
	}
	dbus_connection_flush(conn);

	// free message
	dbus_message_unref(msg);

	// block until we recieve a reply
	dbus_pending_call_block(pending);
	// get the reply message
	msg = dbus_pending_call_steal_reply(pending);
	if (NULL == msg)
	{
		// fprintf(stderr, "Reply Null\n");
		return -1;
	}
	// free the pending message handle
	dbus_pending_call_unref(pending);
	// read the parameters
	if (!dbus_message_iter_init(msg, &args))
	{
		// fprintf(stderr, "Message has no arguments!\n");
		return -1;
	}
	else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args))
	{
		// fprintf(stderr, "Argument is not boolean!\n");
		return -1;
	}
	dbus_message_iter_get_basic(&args, &result);
	return result;
}

static int _dir_exist(const char *dpath)
{
	struct stat st;
	if (stat(dpath, &st))
		return 0;
	if (S_ISDIR(st.st_mode))
		return 1;
	return 0;
}

static int _create_dir(const char *dpath)
{
#ifdef __linux__
	// char *command = malloc(strlen(dpath) + 10);
	// if (!command)
	// 	return -1;
	// sprintf(command, "mkdir -p %s", dpath);
	// int ret = system(command);
	// free(command);
	// return ret;
	pid_t pid=-1;
    int status=-1;
    char **env=NULL;

    pid = fork();
    if (pid == (pid_t) 0)
    {
		char* args[] = {"mkdir -p", dpath, NULL};
        /* Child side.  */
        (void)execve("/usr/bin/mkdir", args, env);
        _exit(127);       
    }
    else if(pid<(pid_t)0)
    {
        /* The fork failed.  */
        status = -1;
    } 
    else
    {
        /* Parent side.  */
        int n;
        do
        {
            n = waitpid (pid, &status, 0);
        }
        while (n == -1 && errno == EINTR);

        if (n != pid)
            status = -1;	
    }
	return status;
#else
	return 1;
#endif
}

/*
*	初始化KLogger核心结构体
*/
int initKLogger(int cid)
{
	if (logger)
	{
		return 0;
	}

	logger	= (KLogger*)calloc(1 , sizeof(KLogger));
	if (!logger)
	{
		printf("kdk_logger实例内存分配失败：%s\n" , strerror(errno));
		return KDK_ENOMEM;
	}

	const char *identer , *stype , *otype , *specfile;

	identer	= kdk_conf_get_value(cid, "TYPE" , "identifier");
	if (identer)
	{
		if (!strcasecmp(identer , "user"))
			logger->identer	= LT_USER;
		else if (!strcasecmp(identer , "local3"))
			logger->identer	= LT_LOCAL3;
		else if (!strcasecmp(identer , "syslog"))
			logger->identer	= LT_SYSLOG;
		else
		{
			logger->identer	= LT_SPEC;
			strncpy(logger->specLogType , identer , KLOG_MAXPRIVALSIZE);
		}
	}
	else
	{
		logger->identer	= DEFAULT_LOGTYPE;
	}
	
	stype	= kdk_conf_get_value(cid, "TYPE" , "synctype");
	if (stype)
	{
		if (!strcasecmp(stype , "async"))
			logger->stype	= ASYNC;
		else if (!strcasecmp(stype , "sync"))
			logger->stype	= SYNC;
		else
			logger->stype	= DEFAULT_SYNCTYPE;
	}
	else
	{
		logger->stype	= DEFAULT_SYNCTYPE;
	}
	
	otype	= kdk_conf_get_value(cid, "TYPE" , "output");
	if (otype)
	{
		if (!strcasecmp(otype , "syslog"))
			logger->otype	= OUT_SYSLOG;
		else if (!strcasecmp(otype , "specfile"))
			logger->otype	= OUT_SPECFILE;
		else if (!strcasecmp(otype , "stdout"))
			logger->otype	= OUT_STDOUT;
		else
			logger->otype	= DEFAULT_OUTPUTTYPE;
	}
	else
	{
		logger->otype	= DEFAULT_OUTPUTTYPE;
	}

	logger->levelBasedStorage	= atoi(kdk_conf_get_value(cid, "CUSTOM" , "levelBasedStorage"));
	if (logger->levelBasedStorage != 0)
		logger->levelBasedStorage	= 1;
	
	logger->levelBasedContainHigherLevel	= atoi(kdk_conf_get_value(cid, "CUSTOM" , "levelBasedContainHigherLevel"));
	if (logger->levelBasedContainHigherLevel != 0)
		logger->levelBasedContainHigherLevel	= 1;
	
	logger->level	= atoi(kdk_conf_get_value(cid, "CUSTOM" , "logLevel"));
	if (logger->level < KLOG_EMERG || logger->level > KLOG_TRACE)
		logger->level	= DEFAULT_LOGLEVEL;

	logger->pid		= getpid();
	snprintf(logger->stringPID , 15 , "%d" , logger->pid);
	char processPath[KLOG_MAXPATHLEN + 1]	= {0};
	if (readlink("/proc/self/exe" , processPath , KLOG_MAXPATHLEN) <= 0)
	{
		printf("无法读取可执行文件名：%s\n" , strerror(errno));
		return errno;
	}

	char* pPName	= strrchr(processPath , '/');
	if (pPName)
		strncpy(logger->processName , ++ pPName , KLOG_PROCESSNAME_LEN);
	else
	{
		strcpy(logger->processName , "untagged");
	}

	logger->mlock	= (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if (!logger->mlock)
	{
		printf("kdk_logger实例锁初始化失败：%s\n" , strerror(errno));
		return errno;
	}
	pthread_mutex_init(logger->mlock , NULL);
	memset(logger->fp.classfiedfp , 0 , sizeof(FILE*) * 8);

	if (logger->otype == OUT_SYSLOG)
		printf("日志记录位置：SYSLOG\n");
	else if (logger->otype == OUT_SPECFILE)
	{
		specfile	= kdk_conf_get_value(cid, "CUSTOM" , "specName");
		const char *dpath = kdk_conf_get_value(cid, "CUSTOM", "logDir");
		if (dpath && strlen(dpath))
			strcpy(logger->rootPath, dpath);
		else
		{
			char canonical_filename[PATH_MAX] = "\0";
			memset(canonical_filename,0,PATH_MAX);
			char *hpath = getenv("HOME");
			realpath(hpath, canonical_filename);
			if(!verify_file(canonical_filename))
			{
				return -1;
			}
			if (!canonical_filename || strcmp(canonical_filename, "/root") == 0)
				strcpy(logger->rootPath, "/var/log");
			else
			{
				strncpy(logger->rootPath, canonical_filename, PATHSIZE);
				strcat(logger->rootPath, "/.log");
				if (!_dir_exist(logger->rootPath))
				{
					if (_create_dir(logger->rootPath))
					{
						return -1;
					}
				}
			}
		}
		if (!specfile || !strcmp(specfile , ""))	//未指定名称，则使用进程名作为文件名称
		{
			if (logger->levelBasedStorage)
			{
				for (int i = 0 ; i < 8 ; i ++)
				{
					sprintf(logger->logfileName.classfiedfileName[i] , "%s-%s.log" , logger->processName , stringLevel[i]);
				}
			}
			else
			{
				snprintf(logger->logfileName.commonlogfileName , KLOG_MAXPATHLEN , "%s.log" , logger->processName);
			}

			// klog_rotate_init(cid, logger->processName, logger->rootPath);
		}
		else		//使用指定的specName作为日志名称
		{
			const char* fName	= strrchr(specfile , '/');
			if (!fName)
			{
				fName	= specfile;
			}
			else
			{
				fName ++;
			}
			strcpy(logger->specName , fName);	//将指定名称保存到结构体中
			if (logger->levelBasedStorage)
			{
				for (int i = 0 ; i < 8 ; i ++)
				{
					sprintf(logger->logfileName.classfiedfileName[i] , "%s-%s.log" , fName , stringLevel[i]);
				}
			}
			else
			{
				snprintf(logger->logfileName.commonlogfileName , KLOG_MAXPATHLEN , "%s.log" , fName);
			}
			// klog_rotate_init(cid, logger->specName, logger->rootPath);
		}

		if (logger->levelBasedStorage)
		{
			char logPath[(KLOG_MAXPATHLEN << 1) + 1];
			for (int i = 0 ; i < 8 ; i ++)
			{
				snprintf(logPath, KLOG_MAXPATHLEN << 1, "%s/%s", logger->rootPath, logger->logfileName.classfiedfileName[i]);
				logger->fp.classfiedfp[i]	= fopen(logger->logfileName.classfiedfileName[i] , "at");
				if (!logger->fp.classfiedfp[i])
				{
					printf("无法打开日志文件%s：%s\n" , logPath, strerror(errno));
					return errno;
				}
				if (-1 == _call_method(logPath))
					;
					// printf("Create %s Rotate Config Failed", logPath);
			}
		}
		else
		{
			char logPath[(KLOG_MAXPATHLEN << 1) + 1];
			snprintf(logPath, KLOG_MAXPATHLEN << 1, "%s/%s", logger->rootPath, logger->logfileName.commonlogfileName);
			char canonical_filename[PATH_MAX] = "\0";
			memset(canonical_filename,0,PATH_MAX);
			realpath(logPath, canonical_filename);
			if(!verify_file(canonical_filename))
			{
				return -1;
			}
			logger->fp.commonfp	= fopen(canonical_filename , "at");
			if (!logger->fp.commonfp)
			{
				printf("无法打开日志文件%s：%s\n" ,logPath, strerror(errno));
				return errno;
			}
			printf("日志记录文件：%s\n" , logPath);
			if (-1 == _call_method(logPath))
				;
			// printf("Create %s Rotate Config Failed", logPath);
		}
	}
	
	return 0;
}

int setRootDir(const char *dpath)
{
	if (!logger || logger->otype != OUT_SPECFILE)
		return -1;
	
	if (!_dir_exist(dpath))
	{
		if (_create_dir(dpath))
			return -1;
	}

	strncpy(logger->rootPath, dpath, KLOG_MAXPATHLEN);
	if (logger->levelBasedStorage)
	{
		char logPath[KLOG_MAXPATHLEN * 2];
		for (int i = 0; i < 8; i++)
		{
			fclose(logger->fp.classfiedfp[i]);
			sprintf(logPath, "%s/%s", logger->rootPath, logger->logfileName.classfiedfileName[i]);
			logger->fp.classfiedfp[i] = fopen(logger->logfileName.classfiedfileName[i], "at");
			if (!logger->fp.classfiedfp[i])
			{
				printf("无法打开日志文件%s：%s\n", logPath, strerror(errno));
				return errno;
			}
		}
	}
	else
	{
		char logPath[KLOG_MAXPATHLEN * 2];
		fclose(logger->fp.commonfp);
		sprintf(logPath, "%s/%s", logger->rootPath, logger->logfileName.commonlogfileName);
		char canonical_filename[PATH_MAX] = "\0";
		memset(canonical_filename,0,PATH_MAX);
		realpath(logPath, canonical_filename);
		if(!verify_file(canonical_filename))
		{
			return -1;
		}
		logger->fp.commonfp = fopen(canonical_filename, "at");
		if (!logger->fp.commonfp)
		{
			printf("无法打开日志文件%s：%s\n", logPath, strerror(errno));
			return errno;
		}
	}

	printf("日志记录位置已修改：%s\n", logger->rootPath);

	return 0;
}

void destroyKLogger()
{
	if (logger)
	{
		MLOCK(logger->mlock);
		if (logger->levelBasedStorage)
		{
			for (int i = 0 ; i < 8 ; i ++)
			{
				if (logger->fp.classfiedfp[i])
				{
					fclose(logger->fp.classfiedfp[i]);
					logger->fp.classfiedfp[i]	= NULL;
				}
			}
		}
		else
		{
			if (logger->fp.commonfp)
			{
				fclose(logger->fp.commonfp);
				logger->fp.commonfp	= NULL;
			}
		}
		MUNLOCK(logger->mlock);
		pthread_mutex_destroy(logger->mlock);
		free(logger->mlock);
		free(logger);
		logger	= NULL;
	}
}

void set_autowrap(int autowrap)
{
	if (! logger)
		return;
	MLOCK(logger->mlock);
	logger->autowrap = autowrap ? 1 : 0;
	MUNLOCK(logger->mlock);
}

void append_wrap(char *message)
{
	if (logger->autowrap && logger->otype != OUT_SYSLOG)
		strcat(message, "\n");
}
