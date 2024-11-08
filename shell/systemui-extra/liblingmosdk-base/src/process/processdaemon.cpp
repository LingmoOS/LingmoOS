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
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>

namespace LINGMOSDK_BASE {
static int g_dogID;
static char g_pipeName[64];  //管道名称

static void daemon(unsigned int interval, daemon_callback func, void* args)
{
    static int pipefd = open(g_pipeName, O_RDONLY);
    if (pipefd <= 0)
        return;
    fd_set read_fds, write_fds, except_fds;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&except_fds);
        FD_SET(pipefd, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = interval;
        timeout.tv_usec = 0;

        char buf[24];

        if (select(pipefd + 1, &read_fds, &write_fds, &except_fds, &timeout) > 0)
        {
            read(pipefd, buf, sizeof(buf));
            printf("got %s\n", buf);
            if (strcmp(buf, "quit") == 0)
                break;
            continue;
        }
        else
        {
            // timeout
            if (func)
                func(args);
        }
    }
    close(pipefd);
}

int startProcDaemon(unsigned int interval, daemon_callback func, void* args)
{
    if (g_dogID > 0)
        return -1;
    
    sprintf(g_pipeName, "/tmp/pipe_procdaemon_%d", getpid());

    if (mkfifo(g_pipeName, 0770) < 0)
        return -2;

    pid_t pid = fork();
    if (pid == 0)
    {
        g_dogID = pid;
        daemon(interval, func, args);
        exit(0);
    }

    return 0;
}

void feedDog()
{
    static int pipefd = open(g_pipeName, O_RDWR);
    if (pipefd <= 0)
        return;
    
    static unsigned long feedCounts = 0;
    char buf[24];
    sprintf(buf, "%023lu", feedCounts);
    write(pipefd, buf, sizeof(buf));
    feedCounts ++;
}

int stopProcDaemon()
{
    if (g_dogID <= 0)
        return -1;
    int pipefd = open(g_pipeName, O_RDWR);
    if (pipefd <= 0)
        return -1;

    char buf[24] = {0};
    strcpy(buf, "quit");
    write(pipefd, buf, sizeof(buf));
    close(pipefd);

    int status;
    waitpid(g_dogID, &status, 0);
    g_dogID = 0;
    
    remove(g_pipeName);

    printf("dog quit.\n");
    return 0;
}

}
