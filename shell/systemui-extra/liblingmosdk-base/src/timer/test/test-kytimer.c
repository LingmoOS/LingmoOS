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


#include <libkytimer.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

void func(char* comment)
{
	char date[32]	= {0};
	time_t now;
	time(&now);
	if (ctime_r(&now , date))
		date[strlen(date) - 1]	= '\0';
	printf("[%s]这是%s\n" , date , comment);
}

void stop(size_t* fdpoll)
{
	printf("开始停止定时器\n");
	for (int i = 0 ; i < 10 ; i ++)
	{
		kdk_timer_stop(fdpoll[i]);
	}
}

int main()
{
	assert(kdk_timer_init() == 0);

	//测试1 -- 基本功能
	size_t fdpoll[10]	= {0};
	// for (int i = 1 ; i <= 10 ; i ++)
	// {
	// 	char* data	= (char*)malloc(10);
	// 	assert(data);
	// 	sprintf(data , "%d号" , i);
	// 	fdpoll[i - 1]	= kdk_timer_start(i * 1000 , (time_handler)func , KTIMER_PERIODIC , KTIMER_ABSOLUTE, (void*)data , 1);
	// 	assert(fdpoll[i - 1]);
	// }
	kdk_timer_start(10000 , (time_handler)stop , KTIMER_SINGLESHOT , KTIMER_ABSOLUTE, (void*)fdpoll , 0);
	sleep(11);

	//测试3 -- 单次触发
	printf("单次触发测试：\n");
	kdk_timer_start(2000 , (time_handler)func , KTIMER_SINGLESHOT , KTIMER_ABSOLUTE, "2号" , 0);
	sleep(5);

	//测试2 -- 重置定时器时间
	printf("修改时间测试：\n");
	int persec	= kdk_timer_start(1000 , (time_handler)func , KTIMER_PERIODIC , KTIMER_ABSOLUTE, "1号" , 0);
	int sec3	= kdk_timer_start(3000 , (time_handler)func , KTIMER_SINGLESHOT , KTIMER_ABSOLUTE, "3号" , 0);
	sleep(2);
	kdk_timer_reset(sec3 , 4000);
	printf("sec3 时钟已被重置为4000ms\n");
	sleep(10);

	printf("正在销毁定时器核心...\n");
	kdk_timer_destroy();
	return 0;
}
