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


#include <libkylog.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int stop	= 0;
int start	= 0;
pthread_mutex_t	lock;
unsigned int	counts	= 0;

void stopHandler(int sig)
{
	stop	= 1;
}

void startTimer()
{
	alarm(10);
}

void* startlog(void* arg)
{
	while (!start);
	unsigned int count	= 0;
	srand(time(NULL));
	while (!stop)
	{
		int lvl	= rand() % 8;
		switch (lvl)
		{
			case 0 :{ 
				assert(klog_emerg("this is emerg count %d\n" , ++ count) == 0);
				}break;
			case 1 : {
				assert(klog_alert("this is alert count %d\n" , ++ count) == 0);
			}break;
			case 2:{
				assert(klog_crit("this is crit count %d\n" , ++ count) == 0);
			}break;
			case 3:{
				assert(klog_err("this is err count %d\n" , ++ count) == 0);
			}break;
			case 4:{
				assert(klog_warning("this is warning count %d\n" , ++ count) == 0);
			}break;
			case 5:{
				assert(klog_notice("this is notice count %d\n" , ++ count) == 0);
			}break;
			case 6:{
				assert(klog_info("this is info count %d\n" , ++ count) == 0);
			}break;
			case 7:{
				assert(klog_debug("this is debug count %d\n" , ++ count) == 0);
			}break;
		}
	}
	pthread_mutex_lock(&lock);
	counts	+= count;
	pthread_mutex_unlock(&lock);
	printf("共写入%d条日志\n" , count);
	return NULL;
}

int main(int argc , char** argv)
{
	assert(kdk_logger_init("./logtest.conf") == 0);

	// //越界测试
	// char msg[4096];
	// memset(msg , 'F' , sizeof(char) * 4096);
	// klog_debug(msg);

	//压力测试
    signal(SIGALRM, stopHandler);
    pthread_mutex_init(&lock, NULL);
    pthread_t children[7] = {0};
    for (int i = 0; i < 7; i++)
    {
        pthread_create(&children[i], NULL, startlog, NULL);
    }

    start = 1;

    startTimer();
    for (int i = 0; i < 7; i++)
    {
        pthread_join(children[i], NULL);
    }
    kdk_logger_flush();
    pthread_mutex_lock(&lock);
    printf("所有线程写入总量：%u\n", counts);
    pthread_mutex_unlock(&lock);
    return system("cat /var/log/logtest.log | wc -l");
}
