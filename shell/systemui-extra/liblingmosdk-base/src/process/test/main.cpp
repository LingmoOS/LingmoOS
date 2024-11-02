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


#include "processdaemon.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int quit;

void func(void* data)
{
    printf("dog fake.\n");
    quit = 1;
    LINGMOSDK_BASE::stopProcDaemon();
}

int main()
{
    LINGMOSDK_BASE::startProcDaemon(5, func, NULL);
    int counts = 0;
    srand(time(NULL));
    while (!quit)
    {
        int stime = rand() % 7;
        printf("sleep %d\n", stime);
        sleep(stime);
        LINGMOSDK_BASE::feedDog();
    }

    printf("quit.\n");
    LINGMOSDK_BASE::stopProcDaemon();
    return 0;
}
