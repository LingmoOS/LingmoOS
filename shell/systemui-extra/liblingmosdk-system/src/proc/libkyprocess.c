/*
 * liblingmosdk-system's Library
 *
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

#include "libkyprocess.h"
#include <cstring-extension.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <linux/param.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <locale.h>
#include <sys/stat.h>
#include <libkylog.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>

#define PRG_SOCKET_PFX "socket:["
#define PRG_SOCKET_PFXl (strlen(PRG_SOCKET_PFX))
#define PRG_SOCKET_PFX2 "[0000]:"
#define PRG_SOCKET_PFX2l (strlen(PRG_SOCKET_PFX2))

#define LINE_MAX 4096

#define COLWID 240
#define UPTIME_FILE "/proc/uptime"
#define MEMINFO "/proc/meminfo"
#define MAX_PF_NAME 1024
#define PROC_DIR "/proc"
#define MAX_PROC_NUM 1024
#define PROC_NUM 1024 * 5
#define BUF_SIZE 1024
static int uptime_fd = -1;
#define LC_NUMERIC __LC_NUMERIC
#define __LC_NUMERIC 1
static char buf[8192];
#define O_RDONLY 00
#define BAD_OPEN_MESSAGE                                           \
    "Error: /proc must be mounted\n"                               \
    "  To mount /proc at boot you need an /etc/fstab line like:\n" \
    "      proc   /proc   proc    defaults\n"                      \
    "  In the meantime, run \"mount proc /proc -t proc\"\n"

#define FILE_TO_BUF(filename, fd)                              \
    do                                                         \
    {                                                          \
        static int local_n;                                    \
        if (fd == -1 && (fd = open(filename, O_RDONLY)) == -1) \
        {                                                      \
            fputs(BAD_OPEN_MESSAGE, stderr);                   \
            fflush(NULL);                                      \
            _exit(102);                                        \
        }                                                      \
        lseek(fd, 0L, SEEK_SET);                               \
        if ((local_n = read(fd, buf, sizeof buf - 1)) < 0)     \
        {                                                      \
            perror(filename);                                  \
            fflush(NULL);                                      \
            _exit(103);                                        \
        }                                                      \
        buf[local_n] = '\0';                                   \
    } while (0)

#define SET_IF_DESIRED(x, y) \
    do                       \
    {                        \
        if (x)               \
            *(x) = (y);      \
    } while (0)

struct pstat
{
    long unsigned int utime_ticks;
    long unsigned int stime_ticks;
    long unsigned int cpu_total_time;
};

/* 进程信息 */
typedef struct process_info
{
    long proc_pid;          // 进程id
    char proc_name[128];    // 进程名
    char proc_command[1024]; // 进程启动的命令行,包含启动参数
    char proc_uid[128];     // 进程所属用户　用户uid
    char start_time[128];   // 进程启动的时间 (char * ctime(time_t time) 函数返回值Tue Jul 28 08:50:26 2020)
    long utime;             // 进程cpu使用时间,单位s
    double proc_cpu;        // 进程cpu使用率 %
    double proc_mem;        // 进程mem使用量 %
    char proc_state[20];    // 进程状态
    char proc_time[50];     // 进程运行时间
    char proc_port[20];     // 进程端口号

} process_info;

#define PROCESS_ITEM 14
#define PROC_COMD_NUM 11520
#define PATH_MAX 50
#define FREE(ptr)       \
    {                   \
        if(ptr)         \
            free(ptr);  \
        ptr = NULL;     \
    }

typedef struct
{
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
} Total_Cpu_Occupy_t;

typedef struct
{
    unsigned int pid;
    unsigned long utime;  // user time
    unsigned long stime;  // kernel time
    unsigned long cutime; // all user time
    unsigned long cstime; // all dead time
} Proc_Cpu_Occupy_t;

static unsigned long lookup_num(char *buffer, const char *key, unsigned long *res)
{
    if (strstartswith(buffer, key) == 0)
    {
        char *val = buffer;
        val += strlen(key) + 1;
        *res = atol(strskipblank(val));
        return 1;
    }
    return 0;
}

static int lookup(char *line, char *pattern, char **value)
{
    char *p, *v;
    int len = strlen(pattern);

    /* pattern */
    if (strncmp(line, pattern, len))
        return 0;

    /* white spaces */
    for (p = line + len; isspace(*p); p++)
        ;

    /* separator */
    if (*p != ':')
        return 0;

    /* white spaces */
    for (++p; isspace(*p); p++)
        ;

    /* value */
    if (!*p)
        return 0;
    v = p;

    /* end of value */
    len = strlen(line) - 1;
    for (p = line + len; isspace(*(p - 1)); p--)
        ;
    *p = '\0';

    *value = strdup(v);
    return 1;
}

static int isnumber(char *c)
{
    char *i = c;

    while (*i != '\0')
    {
        if (!isdigit(*i))
            return 0;
        i++;
    }
    return 1;
}

// 获取第N项开始的指针
const char *get_items(const char *buffer, unsigned int item)
{

    const char *p = buffer;

    int len = strlen(buffer);
    int count = 0;

    for (int i = 0; i < len; i++)
    {
        if (' ' == *p)
        {
            count++;
            if (count == item - 1)
            {
                p++;
                break;
            }
        }
        p++;
    }

    return p;
}

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/proc", strlen("/proc"));
}

// 获取总的CPU时间
unsigned long get_cpu_total_occupy()
{

    FILE *fd = NULL;
    char buff[1024] = {0};
    Total_Cpu_Occupy_t t;

    fd = fopen("/proc/stat", "r");
    if (!fd)
    {
        return 0;
    }

    fgets(buff, sizeof(buff), fd);
    fseek(fd, 0, SEEK_SET);
    fclose(fd);
    char name[64] = {0};
    sscanf(buff, "%s %ld %ld %ld %ld", name, &t.user, &t.nice, &t.system, &t.idle);
    return (t.user + t.nice + t.system + t.idle);
}

// 获取进程的CPU时间
unsigned long get_cpu_proc_occupy(unsigned int pid)
{

    char file_name[64] = {0};
    Proc_Cpu_Occupy_t t;
    FILE *fd;
    char line_buff[1024] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(file_name, "/proc/%d/stat", pid);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return 0;
        }
        if (!realpath(file_name, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return 0;
        }
    }

    fd = fopen(canonical_filename, "r");
    if (!fd)
    {
        FREE(canonical_filename);
        return 0;
    }

    fgets(line_buff, sizeof(line_buff), fd);

    sscanf(line_buff, "%u", &t.pid);
    const char *q = get_items(line_buff, PROCESS_ITEM);
    sscanf(q, "%ld %ld %ld %ld", &t.utime, &t.stime, &t.cutime, &t.cstime);
    fseek(fd, 0, SEEK_SET);
    fclose(fd);
    FREE(canonical_filename);

    return (t.utime + t.stime + t.cutime + t.cstime);
}

char *kdk_get_process_name(int proc_num)
{
    char path[100] = "\0";
    char *name = NULL;
    char link[64] = "\0";
    char buffer[1025] = "\0";
    sprintf(path, "/proc/%d/exe", proc_num);

    int count = readlink(path, link, sizeof(link));
    if (count != -1)
    {
        name = calloc(1, strlen(basename(link)) + 1);
        if(NULL == name)
            return NULL;
        strcpy(name, basename(link));
    }
    else
    {
        memset(path, 0, sizeof(path));
        sprintf(path, "/proc/%d/status", proc_num);
        FILE *fp = fopen(path, "rt");
        if (!fp)
        {
            free(name);
            return NULL;
        }

        while (fgets(buffer, sizeof(buffer), fp))
        {
            if (lookup(buffer, "Name", &name))
                break;
        }

        fclose(fp);
    }
    return name;
}

int kdk_get_process_id(char *proc_name)
{
    DIR *dir;
    struct dirent *ptr;
    FILE *fp;
    char path[512] = "\0";
    int i = 0;
    char *realpath_res = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;
    char line[1024] = {0};
    char *name = NULL;
    char link[128] = "\0";
    int res = 0;

    dir = opendir("/proc");
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL) // 循环读取/proc下的每一个文件/文件夹
        {
            // 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (DT_DIR != ptr->d_type)
                continue;

            sprintf(path, "/proc/%s/exe", ptr->d_name); // 生成要读取的文件的路径
            memset(link, 0, sizeof(link));
            int count = readlink(path, link, sizeof(link));
            if (count != -1)
            {
                if ((strcmp(basename(link), proc_name)) == 0)
                {
                    sscanf(ptr->d_name, "%d", &res);
                }
            }
            else
            {
                memset(path, 0, sizeof(path));
                sprintf(path, "/proc/%s/status", ptr->d_name);
                if (path_size > 0)
                {
                    canonical_filename = malloc(path_size);
                    if (canonical_filename == NULL)
                    {
                        FREE(canonical_filename);
                        continue;
                    }
                    realpath_res = realpath(path, canonical_filename);
                }
                if (canonical_filename == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                if (!verify_file(canonical_filename))
                {
                    FREE(canonical_filename);
                    continue;
                }

                fp = fopen(canonical_filename, "r");
                if (fp == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                else
                {
                    while (fgets(line, sizeof(line), fp))
                    {
                        if (lookup(line, "Name", &name))
                            break;
                    }
                    FREE(canonical_filename);
                    fclose(fp);
                }

                if ((strcmp(name, proc_name)) == 0)
                {
                    sscanf(ptr->d_name, "%d", &res);
                }
                FREE(name);
            }
        }
        closedir(dir);
    }
    return res;
}

float kdk_get_process_cpu_usage_percent(int proc_num)
{
    unsigned long totalcputime1, totalcputime2;
    unsigned long procputime1, procputime2;

    totalcputime1 = get_cpu_total_occupy();
    procputime1 = get_cpu_proc_occupy(proc_num);

    usleep(200000);

    totalcputime2 = get_cpu_total_occupy();
    procputime2 = get_cpu_proc_occupy(proc_num);

    float pcpu = 0.0;
    if (0 != totalcputime2 - totalcputime1)
    {
        pcpu = 100.0 * (procputime2 - procputime1) / (totalcputime2 - totalcputime1);
    }

    return pcpu;
}

float kdk_get_process_mem_usage_percent(int proc_num)
{
    char path[100] = {0};
    float percent = 0.00;
    unsigned long memTotal = 0, memRSS = 0;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(path, "/proc/%d/status", proc_num);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return 0.00;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return 0.00;
        }
    }

    FILE *fp = fopen(canonical_filename, "r");
    if (!fp)
        goto err;

    char buffer[1025];
    char buffers[1025];

    while (fgets(buffer, 1024, fp))
    {
        if (lookup_num(buffer, "VmRSS", &memRSS))
            break;
    }
    fseek(fp, 0, SEEK_SET);
    fclose(fp);

    FILE *fb = fopen(MEMINFO, "rt");
    if (!fb)
        goto err;
    while (fgets(buffers, 1024, fb))
    {
        if (lookup_num(buffers, "MemTotal", &memTotal))
            break;
    }
    if (memTotal)
        percent = ((float)memRSS / (float)memTotal) * 100;

    fseek(fb, 0, SEEK_SET);
    fclose(fb);
    FREE(canonical_filename);
    return percent;

err:
    FREE(canonical_filename);
    return 0.00;
}

char *kdk_get_process_status(int proc_num)
{
    char path[100] = {0};
    char *name = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(path, "/proc/%d/status", proc_num);

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return NULL;
        }
    }

    FILE *fp = fopen(canonical_filename, "r");
    if (!fp)
        goto err;

    char buffer[1025] = {0};

    while (fgets(buffer, 1024, fp))
    {
        if (lookup(buffer, "State", &name))
            break;
    }
    fseek(fp, 0, SEEK_SET);
    fclose(fp);
    FREE(canonical_filename);
    return name;

err:
    FREE(canonical_filename);
    return NULL;
}

static int extract_type_1_socket_inode(const char lname[], unsigned long *inode_p)
{

    /* If lname is of the form "socket:[12345]", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (lname[strlen(lname) - 1] != ']')
        return -1;

    {
        char inode_str[strlen(lname + 1)]; /* e.g. "12345" */
        const int inode_str_len = strlen(lname) - PRG_SOCKET_PFXl - 1;
        char *serr;

        strncpy(inode_str, lname + PRG_SOCKET_PFXl, inode_str_len);
        inode_str[inode_str_len] = '\0';
        *inode_p = strtoul(inode_str, &serr, 0);
        if (!serr || *serr || *inode_p == ~0)
            return -1;
    }
    return 0;
}

static int extract_type_2_socket_inode(const char lname[], unsigned long *inode_p)
{

    /* If lname is of the form "[0000]:12345", extract the "12345"
       as *inode_p.  Otherwise, return -1 as *inode_p.
       */

    if (strlen(lname) < PRG_SOCKET_PFX2l + 1)
        return -1;
    if (memcmp(lname, PRG_SOCKET_PFX2, PRG_SOCKET_PFX2l))
        return -1;
    {
        char *serr;
        *inode_p = strtoul(lname + PRG_SOCKET_PFX2l, &serr, 0);
        if (!serr || *serr || *inode_p == ~0)
            return -1;
    }
    return 0;
}

int kdk_get_process_port(int proc_num)
{
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128], local_addr[128];
    FILE *fd = NULL;
    char line_buff[1024] = {0};
    char line[LINE_MAX];
    int lnamelen;
    char lname[30];
    unsigned long inode_num;
    DIR *dirfd = NULL;
    struct dirent *direfd;
    char path[100] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(path, "/proc/%d/fd", proc_num);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return 0;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return 0;
        }
    }

    dirfd = opendir(canonical_filename);

    if (!dirfd)
    {
        goto err;
    }

    while ((direfd = readdir(dirfd)))
    {
        sprintf(line, "/proc/%d/fd/%s", proc_num, direfd->d_name);
        lnamelen = readlink(line, lname, sizeof(lname) - 1);
        if (lnamelen == -1)
            continue;
        lname[lnamelen] = '\0'; /*make it a null-terminated string*/

        if (extract_type_1_socket_inode(lname, &inode_num) < 0)
            if (extract_type_2_socket_inode(lname, &inode_num) < 0)
                continue;
        fd = fopen("/proc/net/tcp", "r");
        if (NULL == fd)
        {
            closedir(dirfd);
            goto err;
        }
        fseek(fd, 0, SEEK_SET);

        while (fgets(line_buff, sizeof(line_buff), fd))
        {
            num = sscanf(line_buff,
                         "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                         &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                         &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
            if (inode_num == inode)
            {
                // sprintf(proc_prot, "%d", local_port);
                fclose(fd);
                closedir(dirfd);
                FREE(canonical_filename);
                return local_port;
            }
            else
            {
                continue;
            }
        }
        fclose(fd);
        FREE(canonical_filename);
    }
    FREE(canonical_filename);
    closedir(dirfd);
    return 0;

err:
    FREE(canonical_filename);
    return 0;
}

int* kdk_get_process_port_nums(int proc_num)
{
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128], local_addr[128];
    FILE *fd = NULL;
    char line_buff[1024] = {0};
    char line[LINE_MAX];
    int lnamelen;
    char lname[30];
    unsigned long inode_num;
    DIR *dirfd = NULL;
    struct dirent *direfd;
    char path[100] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;
    int i = 0;
    int *res = NULL;

    sprintf(path, "/proc/%d/fd", proc_num);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return NULL;
        }
    }

    dirfd = opendir(canonical_filename);

    if (!dirfd)
    {
        goto err;
    }

    int *tmp = realloc(res, sizeof(int) * 128);
    if (!tmp)
    {
        goto err;
    }
    res = tmp;
    memset(res, 0, sizeof(int) * 128);
    
    while ((direfd = readdir(dirfd)))
    {
        sprintf(line, "/proc/%d/fd/%s", proc_num, direfd->d_name);
        lnamelen = readlink(line, lname, sizeof(lname) - 1);
        if (lnamelen == -1)
            continue;
        lname[lnamelen] = '\0'; /*make it a null-terminated string*/

        if (extract_type_1_socket_inode(lname, &inode_num) < 0)
            if (extract_type_2_socket_inode(lname, &inode_num) < 0)
                continue;

        fd = fopen("/proc/net/tcp", "r");
        if (NULL == fd)
        {
            closedir(dirfd);
            goto err;
        }
        fseek(fd, 0, SEEK_SET);

        while (fgets(line_buff, sizeof(line_buff), fd))
        {
            num = sscanf(line_buff,
                         "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                         &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                         &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
            if (inode_num == inode)
            {
                res[i] = local_port;
                i++;
            }
        }
        fclose(fd);
    }
    FREE(canonical_filename);
    closedir(dirfd);

    return res;

err:
    FREE(canonical_filename);
    if(res)
        free(res);
    return NULL;
}

char *kdk_get_process_start_time(int proc_num)
{
    char path[100] = {0};
    char *name = (char *)malloc(sizeof(char) * 128);
    char st[128] = {0};
    if (!name)
    {
        klog_err("内存申请失败:%s\n", strerror(errno));
        return NULL;
    }
    char *t = NULL;
    char tt[1024] = {0};
    unsigned long long time1, starttime;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(path, "/proc/%d/stat", proc_num);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            free(name);
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            free(name);
            FREE(canonical_filename);
            return NULL;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        goto err;
    }

    t = fgets(tt, sizeof(tt), fpstat);
    if (sscanf(t, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %19llu %*u", &time1) == EOF)
    {
        fclose(fpstat);
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);
    fclose(fpstat);
    FREE(canonical_filename);

    time_t now;
    now = time(NULL);
    struct timespec time2 = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time2);

    starttime = (unsigned long long)(time1 / HZ) + (now - time2.tv_sec) - 0.1;

    time_t time3 = starttime;

    strcpy(st, ctime(&time3));
    strstripspace(st);
    strncpy(name, st, 128);
    return name;

err:
    FREE(canonical_filename);
    free(name);
    return NULL;
}

int uptime(double *restrict uptime_secs, double *restrict idle_secs)
{
    double up = 0, idle = 0;
    char *savelocale;

    FILE_TO_BUF(UPTIME_FILE, uptime_fd);
    savelocale = strdup(setlocale(LC_NUMERIC, NULL));
    setlocale(LC_NUMERIC, "C");
    if (sscanf(buf, "%lf %lf", &up, &idle) < 2)
    {
        setlocale(LC_NUMERIC, savelocale);
        free(savelocale);
        fputs("bad data in " UPTIME_FILE "\n", stderr);
        return 0;
    }
    setlocale(LC_NUMERIC, savelocale);
    free(savelocale);
    SET_IF_DESIRED(uptime_secs, up);
    SET_IF_DESIRED(idle_secs, idle);
    return up; /* assume never be zero seconds in practice */
}

char *kdk_get_process_running_time(int proc_num)
{
    char path[100] = {0};
    char *t = NULL;
    char tt[1024] = {0};
    unsigned long long time1;
    unsigned long long seconds;
    char *cp = (char *)calloc(128, sizeof(char));
    if (!cp)
    {
        klog_err("内存申请失败:%s\n", strerror(errno));
        return NULL;
    }
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(path, "/proc/%d/stat", proc_num);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            free(cp);
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            free(cp);
            FREE(canonical_filename);
            return NULL;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        goto err;
    }

    t = fgets(tt, sizeof(tt), fpstat);
    if (sscanf(t, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %19llu %*u", &time1) == EOF)
    {
        fclose(fpstat);
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);
    fclose(fpstat);
    FREE(canonical_filename);

    time_t seconds_since_boot = uptime(0, 0);
    seconds = seconds_since_boot - (time1 / 100);

    unsigned dd, hh, mm, ss;
    char buff[256];
    ss = seconds % 60;
    seconds /= 60;
    mm = seconds % 60;
    seconds /= 60;
    hh = seconds % 24;
    seconds /= 24;
    dd = seconds;
    if (dd)
    {
        snprintf(cp, COLWID, "%u-", dd);
    }
    else
    {
        snprintf(cp, COLWID, "%u-", 0);
    }
    if (dd || hh)
    {
        snprintf(buff, COLWID, "%02u:", hh);
        strcat(cp, buff);
    }
    else
    {
        snprintf(buff, COLWID, "%02u:", 0);
        strcat(cp, buff);
    }
    memset(buff, '\0', sizeof(buff));
    snprintf(buff, COLWID, "%02u:%02u", mm, ss);
    strcat(cp, buff);
    return cp;

err:
    free(cp);
    FREE(canonical_filename);
    return NULL;
}

char *kdk_get_process_cpu_time(int proc_num)
{
    char path[100] = {0};
    struct pstat result;
    unsigned long utime = 0;
    char *name = (char *)malloc(128 * sizeof(char));
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(path, "/proc/%d/stat", proc_num);

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            free(name);
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            free(name);
            return NULL;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        goto err;
    }

    // read values from /proc/pid/stat
    bzero(&result, sizeof(struct pstat));

    if (fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*s",
               &result.utime_ticks, &result.stime_ticks) == EOF)
    {
        fclose(fpstat);
        goto err;
    }

    utime = (long)((result.utime_ticks + result.stime_ticks) / HZ);
    sprintf(name, "%ld秒", utime);
    fseek(fpstat, 0, SEEK_SET);
    fclose(fpstat);
    FREE(canonical_filename);
    return name;

err:
    free(name);
    FREE(canonical_filename);
    return NULL;
}

char *kdk_get_process_command(int proc_num)
{
    FILE *fd;
    char path[128] = {0};
    char *command = (char *)malloc(1024 * sizeof(char));
    if (!command)
    {
        klog_err("内存申请失败:%s\n", strerror(errno));
        return NULL;
    }
    char line[1024] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    if (proc_num < 0)
    {
        free(command);
        return NULL;
    }
    else
    {
        sprintf(path, "/proc/%d/cmdline", proc_num);
    }

    if (strstr(path, "../"))
    {
        free(command);
        return NULL;
    }
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            free(command);
            return NULL;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            free(command);
            return NULL;
        }
    }

    if ((fd = fopen(canonical_filename, "r")) == NULL)
    {
        goto err;
    }

    int c;
    int i = 0;
    while (1)
    {
        char a = ' ';

        c = fgetc(fd);
        if (feof(fd))
            break;
        if (c == '\0')
            line[i] = a;
        else
            line[i] = c;
        i++;
    }
    strcpy(command, line);
    fseek(fd, 0, SEEK_SET);
    fclose(fd);
    FREE(canonical_filename);
    return command;

err:
    free(command);
    FREE(canonical_filename);
    return NULL;
}

char *kdk_get_process_user(int proc_num)
{
    FILE *fpprocstat, *fp;
    char procstatf[128] = {0};
    char line[128] = {0};
    char buff[128] = {0};
    char *name = NULL;
    int j = 0, i = 0;
    char *bufs[30], *q = NULL;
    char *buf[5], *p = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(procstatf, "/proc/%d/status", proc_num);

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return NULL;
        }
        if (!realpath(procstatf, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return NULL;
        }
    }

    if ((fpprocstat = fopen(canonical_filename, "r")) == NULL)
        goto err;

    while (fgets(line, sizeof(line), fpprocstat) != NULL)
    {
        if (lookup(line, "Uid", &name))
            break;
    }
    fseek(fpprocstat, 0, SEEK_SET);
    fclose(fpprocstat);

    char tmp[256] = "";
    strcpy(tmp, name);
    FREE(name);
    p = strtok(tmp, "	");
    while (p)
    {
        buf[i] = p;
        i++;
        p = strtok(NULL, "	");
    }
    if ((fp = fopen("/etc/passwd", "r")) == NULL)
        goto err;

    while (fgets(buff, sizeof(buff), fp) != NULL)
    {
        if (strstr(buff, buf[1]))
        {
            q = strtok(buff, ":");
            while (q)
            {
                bufs[j] = q;
                j++;
                q = strtok(NULL, ":");
            }

            if (!strcmp(buf[1], bufs[2]))
            {
                name = strdup(bufs[0]);
                break;
            }
        }
    }
    fseek(fp, 0, SEEK_SET);
    fclose(fp);
    FREE(canonical_filename);
    return name;
err:
    FREE(canonical_filename);
    return NULL;
}

int *getPidByName(char *task_name)
{
    DIR *dir;
    struct dirent *ptr;
    FILE *fp;
    char path[512] = "\0";
    char cur_task_name[50];
    char buf[BUF_SIZE];
    int i = 0;
    char *realpath_res = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;
    char *name = NULL;
    char link[128] = "\0";

    int *res = NULL;
    int max_count = 0;

    dir = opendir("/proc");
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL) // 循环读取/proc下的每一个文件/文件夹
        {
            if (((max_count - 1)  == i) || (max_count == 0))
            {
                max_count += 5;
                int *tmp = realloc(res, sizeof(int) * max_count);
                if (!tmp)
                {
                    klog_err("内存申请失败:%s\n", strerror(errno));
                    goto err;
                }
                res = tmp;
                memset(res + i, 0, sizeof(int) * (max_count - i));
            }

            // 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (DT_DIR != ptr->d_type)
                continue;
            sprintf(path, "/proc/%s/exe", ptr->d_name); // 生成要读取的文件的路径
            int count = readlink(path, link, sizeof(link));
            if (count != -1)
            {
                if (strstr(basename(link), task_name))
                {
                    sscanf(ptr->d_name, "%d", res + i);
                    i++;
                }
            }
            else
            {
                memset(path, 0, sizeof(path));
                sprintf(path, "/proc/%s/status", ptr->d_name);
                if (path_size > 0)
                {
                    canonical_filename = malloc(path_size);
                    if (canonical_filename == NULL)
                    {
                        FREE(canonical_filename);
                        continue;
                    }
                    realpath_res = realpath(path, canonical_filename);
                }
                if (canonical_filename == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                if (!verify_file(canonical_filename))
                {
                    FREE(canonical_filename);
                    continue;
                }

                fp = fopen(canonical_filename, "r");
                if (fp == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                else
                {
                    while (fgets(buf, sizeof(buf), fp))
                    {
                        if (lookup(buf, "Name", &name))
                            break;
                    }
                    FREE(canonical_filename);
                }

                if (strstr(name, task_name))
                {
                    sscanf(ptr->d_name, "%d", res + i);
                    i++;
                }
                FREE(name);
            }
        }
        closedir(dir);
    }
    return res;
err:
    free(res);
    closedir(dir);
    free(res);
    return NULL;
}

char **kdk_procname_get_process_infomation(char *proc_name)
{
    int *pid = getPidByName(proc_name);
    if (!pid)
    {
        return NULL;
    }
    char **res = NULL;
    
    char result[PROC_COMD_NUM] = "\0";
    char tmp[PROC_COMD_NUM] = "\0";
    char proc[PROC_COMD_NUM] = "\0";
    float percent = 0.00;
    int num = 0;

    int max_count = 0;
    int index = 0;
    while (pid[index])
    {
        if ((max_count - 1 == index) || (max_count == 0))
        {
            max_count += 5;
            char **tmp = realloc(res, sizeof(char *) * max_count);
            if (!tmp)
            {
                klog_err("内存申请失败:%s\n", strerror(errno));
                goto err;
            }
            res = tmp;
            memset(res + index, 0, sizeof(char *) * (max_count - index));
        }

        memset(result, 0, PROC_COMD_NUM * sizeof(char));
        memset(proc, 0, PROC_COMD_NUM * sizeof(char));

        strcat(result, "proc_pid:");
        sprintf(tmp, "%d", pid[index]);
        strcat(result, tmp);
        strcat(result, ", ");

        percent = kdk_get_process_cpu_usage_percent(pid[index]);
        strcat(result, "proc_cpu:");
        sprintf(tmp, "%0.1f", percent);
        strcat(result, tmp);
        strcat(result, ", ");

        percent = kdk_get_process_mem_usage_percent(pid[index]);
        strcat(result, "proc_mem:");
        sprintf(tmp, "%0.1f", percent);
        strcat(result, tmp);
        strcat(result, ", ");

        char *proc_state = kdk_get_process_status(pid[index]);
        strcat(result, "proc_state:");
        strcat(result, proc_state);
        strcat(result, ", ");
        FREE(proc_state);

        num = kdk_get_process_port(pid[index]);
        strcat(result, "proc_port:");
        sprintf(tmp, "%d", num);
        strcat(result, tmp);
        strcat(result, ", ");

        char *start_time = kdk_get_process_start_time(pid[index]);
        strcat(result, "start_time:");
        strcat(result, start_time);
        strcat(result, ", ");
        free(start_time);

        char *proc_time = kdk_get_process_running_time(pid[index]);
        strcat(result, "proc_time:");
        strcat(result, proc_time);
        strcat(result, ", ");
        free(proc_time);

        char *cpu_time = kdk_get_process_cpu_time(pid[index]);
        strcat(result, "utime:");
        strcat(result, cpu_time);
        strcat(result, ", ");
        free(cpu_time);

        char *proc_command = kdk_get_process_command(pid[index]);
        strcat(result, "proc_command:");
        strcat(result, proc_command);
        strcat(result, ", ");
        free(proc_command);

        char *proc_uid = kdk_get_process_user(pid[index]);
        // strcat(result, "proc_uid:");
        // strcat(result, proc_uid);
        FREE(proc_uid);

        res[index] = strdup(result);
        index++;
    }
    free(pid);
    return res;
err:
    free(pid);
    kdk_proc_freeall(res);
    return NULL;
}

static int get_proc_name(pid_t pid, char *name)
{
    char path[100] = "\0";
    char link[256] = "\0";
    char *tmp = NULL;
    char buffer[1025] = "\0";
    char canonical_filename[PATH_MAX] = "\0";
    if (pid < 0)
        return -1;
    sprintf(path, "/proc/%d/exe", pid);

    int count = readlink(path, link, sizeof(link));
    if (count != -1)
    {
        strcpy(name, basename(link));
    }
    else
    {
        memset(path, 0, sizeof(path)); 
        sprintf(path, "/proc/%d/status", pid);

        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            return -1;
        }
        FILE *fp = fopen(canonical_filename, "rt");
        if (!fp)
        {
            // free(name);
            return -1;
        }

        while (fgets(buffer, sizeof(buffer), fp))
        {
            if (lookup(buffer, "Name", &tmp))
                break;
        }
        strcpy(name, tmp);
        FREE(tmp);

        fclose(fp);
    }
    return 0;
}

int get_proc_state(pid_t pid, char *state)
{
    FILE *fpprocstat = NULL;
    char procstatf[128] = {0};
    char line[128] = {0};
    char *name = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    if (pid < 0)
        return -1;
    else
        sprintf(procstatf, "/proc/%d/status", pid);

    if (strstr(procstatf, "../"))
        return -1;

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(procstatf, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    if ((fpprocstat = fopen(canonical_filename, "r")) == NULL)
    {
        goto err;
    }
    fseek(fpprocstat, 0, SEEK_SET);

    while (fgets(line, sizeof(line), fpprocstat) != NULL)
    {
        if (lookup(line, "State", &name))
            break;
    }
    fclose(fpprocstat);
    FREE(canonical_filename);
    strcpy(state, name);
    FREE(name);
    return 0;

err:
    FREE(canonical_filename);
    FREE(name);
    return -1;
}

static int get_proc_command(pid_t pid, char *command)
{
    FILE *fd = NULL;
    char path[128] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    if (pid < 0)
        return -1;
    else
        sprintf(path, "/proc/%d/cmdline", pid);

    if (strstr(path, "../"))
        return -1;

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    if ((fd = fopen(canonical_filename, "r")) == NULL)
    {
        goto err;
    }
    fseek(fd, 0, SEEK_SET);
    int c;
    int i = 0;
    while (1)
    {
        char a = ' ';

        c = fgetc(fd);
        if (feof(fd))
            break;
        if (c == '\0')
            command[i] = a;
        else
            command[i] = c;
        i++;
    }

    fclose(fd);
    FREE(canonical_filename);
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

int get_proc_uid(pid_t pid, char *proc_uid)
{
    FILE *fpprocstat = NULL, *fp = NULL;
    char procstatf[128] = {0};
    char line[128] = {0};
    char buff[128] = {0};
    char *name = NULL;
    char *cpname = (char *)malloc(20 * sizeof(char));
    int j = 0, i = 0;
    char *cpbuf = (char *)malloc(20 * sizeof(char));
    char *bufs[20], *q = NULL;
    char *buf[5], *p = NULL;
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(procstatf, "/proc/%d/status", pid);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            free(cpbuf);
            free(cpname);
            return -1;
        }
        if (!realpath(procstatf, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            free(cpbuf);
            free(cpname);
            return -1;
        }
    }

    if ((fpprocstat = fopen(canonical_filename, "r")) == NULL)
        goto err;

    while (fgets(line, sizeof(line), fpprocstat) != NULL)
    {
        if (lookup(line, "Uid", &name))
            break;
    }
    fseek(fpprocstat, 0, SEEK_SET);
    fclose(fpprocstat);

    p = strtok(name, "	");
    while (p)
    {
        buf[i] = p;
        i++;
        p = strtok(NULL, "	");
    }
    memset(cpname, 0, sizeof(cpname));
    strcpy(cpname, buf[1]);
    if ((fp = fopen("/etc/passwd", "r")) == NULL)
        goto err;

    while (fgets(buff, sizeof(buff), fp) != NULL)
    {
        if (strstr(buff, cpname))
        {
            q = strtok(buff, ":");
            while (q)
            {
                bufs[j] = q;
                j++;
                q = strtok(NULL, ":");
            }
            memset(cpbuf, 0, sizeof(cpbuf));
            strcpy(cpbuf, bufs[2]);

            if (!strcmp(cpname, cpbuf))
            {
                strcpy(name, bufs[0]);
                break;
            }
        }
    }
    fseek(fp, 0, SEEK_SET);
    strcpy(proc_uid, name);
    free(cpname);
    free(cpbuf);
    fclose(fp);
    FREE(canonical_filename);
    FREE(name);
    return 0;

err:
    free(cpbuf);
    free(cpname);
    FREE(name);
    FREE(canonical_filename);
    return -1;
}

int get_proc_mem_usage_pct(const pid_t pid, double *pct)
{
    FILE *fpmeminfo = NULL;
    FILE *fpprocstat = NULL;
    double memtotal = 1.0;
    double memproc = 0.0;
    char procstatf[128] = {0};
    char line[128] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    if (pid <= 0)
        return -1;

    if ((fpmeminfo = fopen(MEMINFO, "r")) == NULL)
        return -1;
    fseek(fpmeminfo, 0, SEEK_SET);
    while (fgets(line, sizeof(line), fpmeminfo) != NULL)
    {
        if (!strncmp(line, "MemTotal:", 9))
        {
            sscanf(line + 9, "%lf", &memtotal);
            break;
        }
    }
    fclose(fpmeminfo);
    sprintf(procstatf, "/proc/%d/status", pid);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(procstatf, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    fpprocstat = fopen(canonical_filename, "r");
    if (fpprocstat == NULL)
        goto err;
    fseek(fpprocstat, 0, SEEK_SET);
    memset(line, 0, 128);
    while (fgets(line, sizeof(line), fpprocstat) != NULL)
    {
        if (!strncmp(line, "VmRSS:", 6))
        {
            sscanf(line + 6, "%lf", &memproc);
            break;
        }
    }
    fclose(fpprocstat);
    FREE(canonical_filename);
    *pct = 100 * memproc / memtotal;
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

int get_proc_starttime(pid_t pid, char *start_time)
{
    char stat_filepath[30] = {0};
    char *t = NULL;
    char tt[1024] = {0};
    unsigned long long time1, starttime;
    char st[128] = {0};
    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    if (pid < 0)
    {
        return -1;
    }
    else
        sprintf(stat_filepath, "/proc/%d/stat", pid);

    if (strstr(stat_filepath, "../"))
    {
        return -1;
    }

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(stat_filepath, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        klog_err("文件%s打开失败:%s\n", canonical_filename, strerror(errno));
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);

    t = fgets(tt, sizeof(tt), fpstat);
    if (sscanf(t, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %19llu %*u", &time1) == EOF)
    {
        fclose(fpstat);
        goto err;
    }
    fclose(fpstat);
    FREE(canonical_filename);

    time_t now;
    now = time(NULL);
    struct timespec time2 = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time2);

    starttime = (unsigned long long)(time1 / HZ) + (now - time2.tv_sec);
    time_t time3 = starttime;

    strcpy(st, ctime(&time3));
    strstripspace(st);
    strncpy(start_time, st, 128);
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

static long get_proc_cpu_time(pid_t pid)
{
    char stat_filepath[30] = {0};
    struct pstat result;
    long utime = 0;

    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    if (pid < 0)
        return -1;
    else
        sprintf(stat_filepath, "/proc/%d/stat", pid);

    if (strstr(stat_filepath, "../"))
        return -1;
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(stat_filepath, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        klog_err("文件%s打开失败:%s\n", canonical_filename, strerror(errno));
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);

    bzero(&result, sizeof(struct pstat));

    if (fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*s",
               &result.utime_ticks, &result.stime_ticks) == EOF)
    {
        fclose(fpstat);
        goto err;
    }

    utime = (long)((result.utime_ticks + result.stime_ticks) / HZ);
    fclose(fpstat);
    FREE(canonical_filename);
    return utime;

err:
    FREE(canonical_filename);
    return -1;
}

int get_proc_cpu_pcpu(pid_t pid, double *pcpu)
{
    char stat_filepath[30] = {0};
    struct pstat result;
    unsigned long long time1;
    long cutime, cstime;
    *pcpu = 0;

    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    if (pid < 0)
        return -1;
    else
        sprintf(stat_filepath, "/proc/%d/stat", pid);

    if (strstr(stat_filepath, "../"))
        return -1;

    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(stat_filepath, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        klog_err("文件%s打开失败:%s\n", canonical_filename, strerror(errno));
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);

    bzero(&result, sizeof(struct pstat));
    if (fscanf(fpstat, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %lu %lu %*d %*d %*d %*d %llu %*u",
               &result.utime_ticks, &result.stime_ticks, &cutime, &cstime, &time1) == EOF)
    {
        fclose(fpstat);
        goto err;
    }

    struct timespec time2 = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &time2);

    long long t1, t2;
    t1 = (result.utime_ticks + result.stime_ticks) / HZ;
    t2 = time2.tv_sec - time1 / HZ;

    if (t2)
        *pcpu = (double)(100 * t1 / t2);
    fclose(fpstat);
    FREE(canonical_filename);
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

int get_proc_running_time(pid_t pid, char *proc_time)
{
    char path[100] = {0};
    char *t = NULL;
    char tt[1024] = {0};
    unsigned long long time1;
    unsigned long long seconds;
    char cp[128] = {0};
    char buff[128] = {0};
    if (!buff)
    {
        klog_err("内存申请失败:%s\n", strerror(errno));
        return -1;
    }

    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(path, "/proc/%d/stat", pid);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    FILE *fpstat = fopen(canonical_filename, "r");
    if (fpstat == NULL)
    {
        goto err;
    }
    fseek(fpstat, 0, SEEK_SET);

    t = fgets(tt, sizeof(tt), fpstat);
    if (sscanf(t, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %19llu %*u", &time1) == EOF)
    {
        fclose(fpstat);
        goto err;
    }
    fclose(fpstat);
    FREE(canonical_filename);

    time_t seconds_since_boot = uptime(0, 0);
    seconds = seconds_since_boot - (time1 / 100);

    unsigned dd, hh, mm, ss;
    ss = seconds % 60;
    seconds /= 60;
    mm = seconds % 60;
    seconds /= 60;
    hh = seconds % 24;
    seconds /= 24;
    dd = seconds;
    if (dd)
    {
        snprintf(cp, COLWID, "%u-", dd);
    }
    else
    {
        snprintf(cp, COLWID, "%u-", 0);
    }
    if (dd || hh)
    {
        snprintf(buff, COLWID, "%02u:", hh);
        strcat(cp, buff);
    }
    else
    {
        snprintf(buff, COLWID, "%02u:", 0);
        strcat(cp, buff);
    }
    memset(buff, 0, sizeof(buff));
    snprintf(buff, COLWID, "%02u:%02u", mm, ss);
    strcat(cp, buff);
    strcpy(proc_time, cp);
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

int get_proc_port(pid_t pid, char *proc_prot)
{
    unsigned long rxq, txq, time_len, retr, inode;
    int num, local_port, rem_port, d, state, uid, timer_run, timeout;
    char rem_addr[128], local_addr[128];
    FILE *fd = NULL;
    char line_buff[1024] = {0};
    char line[LINE_MAX];
    int lnamelen;
    char lname[30];
    unsigned long inode_num;
    DIR *dirfd = NULL;
    struct dirent *direfd;
    char path[100] = {0};

    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    sprintf(path, "/proc/%d/fd", pid);
    if (path_size > 0)
    {
        canonical_filename = malloc(path_size);
        if (canonical_filename == NULL)
        {
            return -1;
        }
        if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
        {
            FREE(canonical_filename);
            return -1;
        }
    }

    dirfd = opendir(canonical_filename);
    if (!dirfd)
    {
        goto err;
    }

    while ((direfd = readdir(dirfd)))
    {
        sprintf(line, "/proc/%d/fd/%s", pid, direfd->d_name);
        lnamelen = readlink(line, lname, sizeof(lname) - 1);
        if (lnamelen == -1)
            continue;
        lname[lnamelen] = '\0'; /*make it a null-terminated string*/

        if (extract_type_1_socket_inode(lname, &inode_num) < 0)
            if (extract_type_2_socket_inode(lname, &inode_num) < 0)
                continue;
        fd = fopen("/proc/net/tcp", "r");
        if (NULL == fd)
        {
            closedir(dirfd);
            goto err;
        }
        fseek(fd, 0, SEEK_SET);

        while (fgets(line_buff, sizeof(line_buff), fd))
        {
            num = sscanf(line_buff,
                         "%d: %64[0-9A-Fa-f]:%X %64[0-9A-Fa-f]:%X %X %lX:%lX %X:%lX %lX %d %d %lu %*s\n",
                         &d, local_addr, &local_port, rem_addr, &rem_port, &state,
                         &txq, &rxq, &timer_run, &time_len, &retr, &uid, &timeout, &inode);
            if (inode_num == inode)
            {
                sprintf(proc_prot, "%d", local_port);
            }
            else
            {
                continue;
            }
        }
        fclose(fd);
    }
    FREE(canonical_filename);
    closedir(dirfd);
    return 0;

err:
    FREE(canonical_filename);
    return -1;
}

int get_process_info(process_info *info, unsigned int *proc_num)
{
    DIR *dir;
    struct dirent *drd;
    struct stat buf;
    process_info *proc_info = info;
    char dirname[MAX_PF_NAME] = {0};

    char *canonical_filename = NULL;
    size_t path_size = 0;
    path_size = (size_t)PATH_MAX;

    int count = 0;
    *proc_num = 0;
    int i = 0;
    if ((dir = opendir(PROC_DIR)) == NULL)
        return -1;

    while ((drd = readdir(dir)) != NULL)
    {
        if (*proc_num >= MAX_PROC_NUM)
            break;
        if (isnumber(drd->d_name))
        {
            sprintf(dirname, "%s/%s", PROC_DIR, drd->d_name);
            if (path_size > 0)
            {
                canonical_filename = malloc(path_size);
                if (canonical_filename == NULL)
                {
                    FREE(canonical_filename);
                    continue;
                }
                if (!realpath(dirname, canonical_filename) || !verify_file(canonical_filename))
                {
                    FREE(canonical_filename);
                    continue;
                }
            }

            if (stat(canonical_filename, &buf) < 0)
            {
                FREE(canonical_filename);
                continue;
            }
            if (!S_ISDIR(buf.st_mode))
            {
                FREE(canonical_filename);
                continue;
            }
            i++;
            memset(proc_info, 0, sizeof(process_info));
            sscanf(drd->d_name, "%ld", &(proc_info->proc_pid));
            get_proc_name(proc_info->proc_pid, proc_info->proc_name);

            get_proc_state(proc_info->proc_pid, proc_info->proc_state);

            get_proc_command(proc_info->proc_pid, proc_info->proc_command);

            get_proc_uid(proc_info->proc_pid, proc_info->proc_uid);

            get_proc_mem_usage_pct(proc_info->proc_pid, &(proc_info->proc_mem));

            get_proc_starttime(proc_info->proc_pid, proc_info->start_time);

            proc_info->utime = get_proc_cpu_time(proc_info->proc_pid);

            get_proc_cpu_pcpu(proc_info->proc_pid, &(proc_info->proc_cpu));

            get_proc_running_time(proc_info->proc_pid, proc_info->proc_time);

            get_proc_port(proc_info->proc_pid, proc_info->proc_port);

            proc_info++;
            ++count;
            FREE(canonical_filename);
        }
    }
    *proc_num = i;
    closedir(dir);

    return 0;
}

char **kdk_get_process_all_information()
{
    process_info pp[PROC_NUM];
    unsigned int proc_num;
    char result[115200] = "\0";
    char tmp[115200] = "\0";
    size_t counts = 0;
    char **res = NULL;
    char **tmpres = NULL;
    int ret = get_process_info(pp, &proc_num);
    if (ret == 0)
    {
        for (int i = 0; i < proc_num; i++)
        {
            memset(result, 0, 115200 * sizeof(char));
            memset(tmp, 0, 115200 * sizeof(char));
            strcat(result, "process_id:");
            sprintf(tmp, "%ld", pp[i].proc_pid);
            strcat(result, tmp);
            strcat(result, ", ");
            strcat(result, "proc_name:");
            strcat(result, pp[i].proc_name);
            strcat(result, ", ");
            strcat(result, "proc_command:");
            strcat(result, pp[i].proc_command);
            strcat(result, ", ");
            strcat(result, "proc_uid:");
            sprintf(tmp, "%s", pp[i].proc_uid);
            strcat(result, tmp);
            strcat(result, ", ");
            strcat(result, "start_time:");
            strcat(result, pp[i].start_time);
            strcat(result, ", ");
            strcat(result, "utime:");
            sprintf(tmp, "%ld", pp[i].utime);
            strcat(result, tmp);
            strcat(result, ", ");
            strcat(result, "proc_cpu:");
            sprintf(tmp, "%.2f", (double)pp[i].proc_cpu);
            strcat(result, tmp);
            strcat(result, ", ");
            strcat(result, "proc_mem:");
            sprintf(tmp, "%.2f", (double)pp[i].proc_mem);
            strcat(result, tmp);
            strcat(result, ", ");
            strcat(result, "proc_state:");
            sprintf(tmp, "%s", pp[i].proc_state);
            strcat(result, tmp);
            strcat(result, ", ");
            strcat(result, "proc_time:");
            sprintf(tmp, "%s", pp[i].proc_time);
            strcat(result, tmp);
            strcat(result, ", ");
            strcat(result, "proc_port:");
            sprintf(tmp, "%s", pp[i].proc_port);
            strcat(result, tmp);
            tmpres = realloc(res, (proc_num + 3) * sizeof(char *));
            if (!tmpres)
            {
                klog_err("内存申请失败:%s\n", strerror(errno));
                free(res);
                return NULL;
            }
            res = tmpres;
            res[counts] = malloc(115200 * sizeof(char) + 1);
            if (!res[counts])
            {
                free(tmpres);
                klog_err("内存申请失败:%s\n", strerror(errno));
                return NULL;
            }
            strcpy(res[counts], result);
            counts++;
        }
    }
    res[counts] = NULL;
    return res;
}

inline void kdk_proc_freeall(char **list)
{
    if (!list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index++;
    }
    free(list);
}

inline void kdk_proc_freeid(int *list)
{
    if (!list)
        return;
    free(list);
}
