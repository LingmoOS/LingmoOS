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

#include <stdio.h>
#include <string.h>
#include <X11/extensions/Xrandr.h>
#include <dirent.h>
#include <errno.h>
#include <libkylog.h>
#include <fcntl.h>
#include <lingmosdk/lingmosdk-base/cstring-extension.h>
#include <lingmosdk/lingmosdk-base/sdkmarcos.h>
#include <math.h>
#include <stdint.h>
#include <assert.h>
#include "libkyedid.h"
#include "hd.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <lingmosdk/lingmosdk-base/sdkmarcos.h>

static int edid_lines = 0;

// static int claims_one_point_oh = 0;
static int claims_one_point_two = 0;
static int claims_one_point_three = 0;
static int claims_one_point_four = 0;

static char* get_val_from_file(FILE *fp, const char *key)
{
    if (! fp)
        return NULL;
    char *val = NULL;
    char buf[2048] = {0};
    while (fgets(buf, 2048, fp))
    {
        if (strncmp(buf, key, strlen(key)) == 0)
        {
            val = strdup((char*)buf + strlen(key) +1);
            break;
        }
    }

    return val;
}

static int verify_file(char *pFileName)
{
    if(!strncmp(pFileName, "/sys", strlen("/sys")))
    {
        return 1;
    }
    else if(!strncmp(pFileName, "/tmp", strlen("/tmp")))
    {
        return 1;
    }
    else{
        return 0;
    }
}

static char *cmd_exec(char *file)
{
    char *res = NULL;
    int fd[2];
    if (pipe(fd) == -1) {
        return NULL;
    }

    pid_t pid = fork();
    if (pid == -1) {
        return NULL;
    }

    if (pid == 0) {
        close(fd[0]); // 关闭读取端
        dup2(fd[1], STDOUT_FILENO); // 将管道写入端连接到标准输出

        // 使用execve执行所需的命令，例如：
        char* args[] = {"edid-decode", file, NULL};
        execve("/bin/edid-decode", args, NULL);

        // 如果execve执行失败，需要打印错误信息并退出
        perror("Failed to execute command");
        exit(1);
    } else {
        close(fd[1]); // 关闭写入端

        const char* targetLine1 = "Display Product Name"; // 要匹配的目标行
        const char* targetLine2 = "Alphanumeric Data String"; // 要匹配的目标行
        
        res = (char *)malloc(sizeof(char) * 128);
        FILE* fp = fdopen(fd[0], "r"); // 将读取端转换为文件指针
        char *val = get_val_from_file(fp, targetLine1);
        if(!val)
        {
            char *value = get_val_from_file(fp, targetLine2);
            if(!value)
            {
                free(res);
                return NULL;
            }
            strcpy(res, value);
            free(value);
        }
        else{
            strcpy(res, val);
            free(val);
        }

        fclose(fp); // 关闭文件指针

        // 等待子进程退出并获取退出状态
        // int status;
        // waitpid(pid, &status, 0);
    }
    return res;
}

static void kdk_edid(char *name)
{
    char edid_text[2048] = "\0";
    char *edid_character = (char *)malloc(sizeof(char) * 1024);
    if(!edid_character)
    {
        return;
    }
    FILE *pipeLine = popen("xrandr --prop", "r");  // 建立流管道
    if (!pipeLine)
    {
        free(edid_character);
        return;
    }
    int i = 0;
    int isEdid = 0;
    memset(edid_character, 0, 1024);
    while(fgets(edid_text, sizeof(edid_text), pipeLine))
    {
        if(i != 1)
        {
            if(strstr(edid_text, name))
            {
                i = 1;
                continue;
            }
            else{
                continue;
            }
        }
        else{
            if(isEdid != 1)
            {
                if(strstr(edid_text, "EDID"))
                {
                    isEdid = 1;
                    continue;
                }
                else
                    continue;
            }
            
            if(strstr(edid_text, "TearFree") || strstr(edid_text, "GAMMA_LUT_SIZE:") || strstr(edid_text, "non-desktop:") || strstr(edid_text, "BorderDimensions"))
            {
                break;
            }
        
            strstripspace(edid_text);
            strcat(edid_character, edid_text);
        }
    }
    pclose(pipeLine);

    FILE *fp = fopen("/tmp/sdk-edid", "w");
    if(!fp)
    {
        free(edid_character);
        return;
    }
    fprintf(fp, "%s", edid_character);
    
    fclose(fp);
    free(edid_character);
    return;

}

char **kdk_edid_get_interface()
{
    Display *disp;
    XRRScreenResources *screen;
    XRROutputInfo *info;
    int iscres;
    int icrtc;
    char tmp[128] = "\0";
    char **res = NULL;
    int count = 0;
    char test[100] = {0};
    
    disp = XOpenDisplay(0);
    screen = XRRGetScreenResources(disp, DefaultRootWindow(disp));
    for (iscres = screen->noutput; iscres > 0;)
    {
        --iscres;

        info = XRRGetOutputInfo(disp, screen, screen->outputs[iscres]);
        if (info->connection == RR_Connected)
        {
            for (icrtc = info->ncrtc; icrtc > 0;)
            {
                memset(tmp, 0, sizeof(tmp));

                if (strstr(test, info->name))
                {
                    break;
                }
                else
                {
                    strcpy(tmp, info->name);
                    strcpy(test, tmp);
                    --icrtc;
                }

                res = realloc(res, (count + 2) * sizeof(char *));
                if (!res)
                {
                    klog_err("内存申请失败：%s\n", strerror(errno));
                    goto err_out;
                }
                res[count] = calloc(strlen(tmp) + 1, sizeof(char));
                if (!res[count])
                {
                    klog_err("内存申请失败：%s\n", strerror(errno));
                    goto err_out;
                }
                strcpy(res[count], tmp);
                count++;
            }
        }      
        XRRFreeOutputInfo(info);
    }
    XRRFreeScreenResources(screen);
    XCloseDisplay(disp);
    res[count] = NULL;
    goto out;

err_out:
    while (count)
    {
        free(res[count - 1]);
        count--;
    }
    SAFE_FREE(res);
    return NULL;
out:
    return res;
}

static unsigned char *extract_edid(int fd)
{
    char *ret = NULL;
    char *start, *c;
    unsigned char *out = NULL;
    int state = 0;
    int lines = 0;
    int i;
    int out_index = 0;
    int len, size;

    size = 1 << 10;
    ret = malloc(size);
    len = 0;

    if (ret == NULL)
        return NULL;

    for (;;)
    {
        i = read(fd, ret + len, size - len);
        if (i < 0)
        {
            free(ret);
            return NULL;
        }
        if (i == 0)
            break;
        len += i;
        if (len == size)
        {
            char *t;
            size <<= 1;
            t = realloc(ret, size);
            if (t == NULL)
            {
                free(ret);
                return NULL;
            }
            ret = t;
        }
    }

    start = strstr(ret, "EDID_DATA:");
    if (start == NULL)
        start = strstr(ret, "EDID:");
    /* Look for xrandr --verbose output (lines of 16 hex bytes) */
    if (start != NULL)
    {
        const char indentation1[] = "                ";
        const char indentation2[] = "\t\t";
        /* Used to detect that we've gone past the EDID property */
        const char half_indentation1[] = "        ";
        const char half_indentation2[] = "\t";
        const char *indentation;
        char *s;

        lines = 0;
        for (i = 0;; i++)
        {
            int j;

            /* Get the next start of the line of EDID hex, assuming spaces for indentation */
            s = strstr(start, indentation = indentation1);
            /* Did we skip the start of another property? */
            if (s && s > strstr(start, half_indentation1))
                break;

            /* If we failed, retry assuming tabs for indentation */
            if (!s)
            {
                s = strstr(start, indentation = indentation2);
                /* Did we skip the start of another property? */
                if (s && s > strstr(start, half_indentation2))
                    break;
            }

            if (!s)
                break;

            lines++;
            start = s + strlen(indentation);

            s = realloc(out, lines * 16);
            if (!s)
            {
                free(ret);
                free(out);
                return NULL;
            }
            out = (unsigned char *)s;
            c = start;
            for (j = 0; j < 16; j++)
            {
                char buf[3];
                /* Read a %02x from the log */
                if (!isxdigit(c[0]) || !isxdigit(c[1]))
                {
                    if (j != 0)
                    {
                        lines--;
                        break;
                    }
                    free(ret);
                    free(out);
                    return NULL;
                }
                buf[0] = c[0];
                buf[1] = c[1];
                buf[2] = 0;
                out[out_index++] = strtol(buf, NULL, 16);
                c += 2;
            }
        }

        free(ret);
        edid_lines = lines;
        return out;
    }

    /* Is the EDID provided in hex? */
    for (i = 0; i < 32 && isxdigit(ret[i]); i++)
        ;
    if (i == 32)
    {
        out = malloc(size >> 1);
        if (out == NULL)
        {
            free(ret);
            return NULL;
        }

        for (c = ret; *c; c++)
        {
            char buf[3];

            if (*c == '\n')
                continue;

            /* Read a %02x from the log */
            if (!isxdigit(c[0]) || !isxdigit(c[1]))
            {
                free(ret);
                free(out);
                return NULL;
            }

            buf[0] = c[0];
            buf[1] = c[1];
            buf[2] = 0;

            out[out_index++] = strtol(buf, NULL, 16);
            c++;
        }

        free(ret);
        edid_lines = out_index >> 4;
        return out;
    }

    /* wait, is this a log file? */
    for (i = 0; i < 8; i++)
    {
        if (!isascii(ret[i]))
        {
            edid_lines = len / 16;
            return (unsigned char *)ret;
        }
    }

    /* I think it is, let's go scanning */
    if (!(start = strstr(ret, "EDID (in hex):")))
        return (unsigned char *)ret;
    if (!(start = strstr(start, "(II)")))
        return (unsigned char *)ret;

    for (c = start; *c; c++)
    {
        if (state == 0)
        {
            char *s;
            /* skip ahead to the : */
            s = strstr(c, ": \t");
            if (!s)
                s = strstr(c, ":     ");
            if (!s)
                break;
            c = s;
            /* and find the first number */
            while (!isxdigit(c[1]))
                c++;
            state = 1;
            lines++;
            s = realloc(out, lines * 16);
            if (!s)
            {
                free(ret);
                free(out);
                return NULL;
            }
            out = (unsigned char *)s;
        }
        else if (state == 1)
        {
            char buf[3];
            /* Read a %02x from the log */
            if (!isxdigit(*c))
            {
                state = 0;
                continue;
            }
            buf[0] = c[0];
            buf[1] = c[1];
            buf[2] = 0;
            out[out_index++] = strtol(buf, NULL, 16);
            c++;
        }
    }

    edid_lines = lines;

    free(ret);

    return out;
}

const float kdk_edid_get_gamma(char *name)
{
    if(!name)
    {
        return 0.00;
    }
    int fd;
    unsigned char *edid;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    float ga = 0.00;
    char canonical_filename[100] = "\0";
    char filename[100] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);
    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return 0.00;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return 0.00;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename)
            || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
	    // close(fd);
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            closedir(dirfd);
            return 0.00;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            closedir(dirfd);
            return 0.00;
        }
	    fclose(fs);
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            closedir(dirfd);
            return 0.00;
        }
        char *gamma = get_val_from_file(fp, "Gamma");
        if(gamma)
        {
            strstripspace(gamma);
            ga = atof(gamma);
        }
        free(gamma);
        closedir(dirfd);
        pclose(fp);
        return ga;

    }
    else{
        edid = extract_edid(fd);
        if (!edid)
        {
            goto err_out;
        }

        if (!edid || memcmp(edid, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8))
        {
            goto err_out;
        }

        switch (edid[0x13])
        {
        case 4:
            claims_one_point_four = 1;
        case 3:
            claims_one_point_three = 1;
        case 2:
            claims_one_point_two = 1;
        default:
            break;
        }
        close(fd);
        closedir(dirfd);

        if (edid[0x17] == 0xff)
        {
            if (claims_one_point_four)
                ;
            else{
                free(edid);
                return 1.0;
            }
        }
        else
        {
            ga = ((edid[0x17] + 100.0) / 100.0);
            free(edid);
            return ga;
        }
    }

err_out:
    close(fd);
    closedir(dirfd);
    if(edid)
    {
        free(edid);
    }
    return 0.00;
}

float kdk_edid_get_size(char *name)
{
    if(!name)
    {
        return 0.00;
    }
    int fd;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    float size = 0.00;
    char filename[100] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);

    FILE *fp = NULL;
    int num1 = 0;
    int num2 = 0;

    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return 0.00;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return 0.00;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    closedir(dirfd);
    free(fg);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
        || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            
            return 0.00;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            return 0.00;
        }
		fclose(fs);
        fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            return 0.00;
        }
        
    }
    else{
        int fd_pipe[2];
        if (pipe(fd_pipe) == -1) {
            return 0.00;
        }

        pid_t pid = fork();
        if (pid == -1) {
            return 0.00;
        }

        if (pid == 0) {
            close(fd_pipe[0]); // 关闭读取端
            dup2(fd_pipe[1], STDOUT_FILENO); // 将管道写入端连接到标准输出

            // 使用execve执行所需的命令，例如：
            char* args[] = {"edid-decode", path, NULL};
            execve("/bin/edid-decode", args, NULL);

            // 如果execve执行失败，需要打印错误信息并退出
            perror("Failed to execute command");
            exit(1);
        } else {
            close(fd_pipe[1]); // 关闭写入端
            fp = fdopen(fd_pipe[0], "r"); // 将读取端转换为文件指针
            if(!fp)
            {
                return 0.00;
            }
        }
    }
    char xsize[16] = "\0";
    char ysize[16] = "\0";
    char *tmp = get_val_from_file(fp, "Maximum image size");
    if(!tmp)
    {
        pclose(fp);
        return 0.00;
    }
    char *msize = get_val_from_file(fp, "Detailed mode");
    if(msize)
    {
        sscanf(msize, "%*s %*s %*s %s %*s %*s %s", xsize, ysize);
        num1 = atoi(xsize);
        num2 = atoi(ysize);
        size = sqrt(pow(num1, 2) + pow(num2, 2)) / 25.4;
        free(msize);
        free(tmp);
    }
    else if(tmp)
    {
        sscanf(tmp, "%s %*s %*s %s", xsize, ysize);
        num1 = atoi(xsize);
        num2 = atoi(ysize);
        size = sqrt(pow(num1, 2) + pow(num2, 2)) / 2.54;
        free(tmp);
    }
    pclose(fp);

    char *model = kdk_edid_get_model(name);
    if(!model)
    {
        return size;
    }
    // greatwall
    if(!strcmp("24AL60", model))
    {
        size = 23.8;
    }

    // X100
    if(!strcmp("LM133LF-8L02", model))
    {
        size = 13.3;
    }
    free(model);
    return size;
}

char *kdk_edid_get_max_resolution(char *name)
{
    if(!name)
    {
        return NULL;
    }
    char edid_text[2048] = "\0";
    char tmp[64] = "\0";
    char *res = (char *)malloc(sizeof(char) * 64);
    if(!res)
    {
        return NULL;
    }
    memset(res, 0, 64);
    FILE *pipeLine = popen("xrandr --prop", "r");  // 建立流管道
    if (!pipeLine)
    {
        free(res);
        return NULL;
    }
    int i = 0;
    while(fgets(edid_text, sizeof(edid_text), pipeLine))
    {
        if(i != 1)
        {
            if(strstr(edid_text, name))
            {
                i = 1;
            }
            continue;
        }
        if(strstr(edid_text, "+"))
        {
            strncpy(tmp, edid_text, 63);
        }

    }
    pclose(pipeLine);
    sscanf(tmp, "%s", res);
    if(strlen(res) == 0)
    {
        free(res);
        return NULL;
    }
    
    return res;
}

/* extract a string from a detailed subblock, checking for termination */
static char *extract_string(const unsigned char *x, unsigned len)
{
    static char s[50];
    int seen_newline = 0;
    unsigned i;

    memset(s, 0, sizeof(s));

    for (i = 0; i < len; i++)
    {
        if (isgraph(x[i]))
        {
            s[i] = x[i];
        }
        else if (!seen_newline)
        {
            if (x[i] == 0x0a)
            {
                seen_newline = 1;
                if (!i)
                    ;
                else if (s[i - 1] == 0x20)
                    ;
            }
            else if (x[i] == 0x20)
            {
                s[i] = x[i];
            }
            else
            {
                return s;
            }
        }
        else if (x[i] != 0x20)
        {
            return s;
        }
    }
    /* Does the string end with a space? */
    if (!seen_newline && s[len - 1] == 0x20)
        ;

    return s;
}

static char *detailed_block(const unsigned char *x)
{
    static const unsigned char zero_descr[18] = {0};

    if (!memcmp(x, zero_descr, sizeof(zero_descr)))
    {
        return NULL;
    }

    switch (x[3])
    {
    case 0xfc:
        return extract_string(x + 5, 13);
    }
    return NULL;
}

char *kdk_edid_get_model(char *name)
{
    if(!name)
    {
        return NULL;
    }
    int fd;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    char filename[100] = "\0";
    char *edid_text = NULL;
    char str_name[32] = "\0";
    strcpy(str_name, name);
    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return NULL;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return NULL;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    closedir(dirfd);

    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
                        || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            goto err_out;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            goto err_out;
        }
		fclose(fs);
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            goto err_out;
        }
        edid_text = get_val_from_file(fp, "Display Product Name");
        if(!edid_text)
        {
            fp = popen("edid-decode /tmp/sdk-edid", "r");
            if(!fp)
            {
                goto err_out;
            }
            char *str_model = get_val_from_file(fp, "Alphanumeric Data String");
            if(!str_model)
            {
                goto err_out;
            }
            pclose(fp);
            
            strstripspace(str_model);
            return str_model;
        }
        else{
            pclose(fp);
            strstripspace(edid_text);
            return edid_text;
        }
    }

    char *cmd_val = cmd_exec(path);
    if(!cmd_val)
    {
        hd_data_t *hd_data;
        hd_data = (hd_data_t *)calloc(1, sizeof *hd_data);
        if(!hd_data)
        {
            goto err_out;
        }
        char src_model[128] = "\0";
        memset(src_model, 0, 128);

        hd_data->progress = NULL;
        hd_data->debug = ~(HD_DEB_DRIVER_INFO | HD_DEB_HDDB);

        hd_t *hd, *hd0;
        hd0 = hd_list(hd_data, hw_monitor, 1, NULL);
        for (hd = hd0; hd; hd = hd->next)
        {
            if(hd->model)
            {
                strcpy(src_model, hd->model);
            }
        }
        
        hd_free_hd_data(hd_data);
        hd_free_hd_list(hd0);
        if(strlen(src_model) == 0)
        {
            goto err_out;
        }
        else{
            char *model = (char *)malloc(sizeof(char) * 128);
            strcpy(model, src_model);
            return model;
        }
    }
    else{
        strstripspace(cmd_val);
        return cmd_val;
    }
err_out:
    return NULL;
}

char *kdk_edid_get_visible_area(char *name)
{
    if(!name)
    {
        return NULL;
    }
    int fd;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    char filename[100] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);

    FILE *fp = NULL;
    float num1 = 0.0;
    float num2 = 0.0;

    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return NULL;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return NULL;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    closedir(dirfd);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
        || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            return NULL;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            return NULL;
        }
		fclose(fs);
        fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            return NULL;
        }
        
    }
    else{
        int fd_pipe[2];
        if (pipe(fd_pipe) == -1) {
            return NULL;
        }

        pid_t pid = fork();
        if (pid == -1) {
            return NULL;
        }

        if (pid == 0) {
            close(fd_pipe[0]); // 关闭读取端
            dup2(fd_pipe[1], STDOUT_FILENO); // 将管道写入端连接到标准输出

            // 使用execve执行所需的命令，例如：
            char* args[] = {"edid-decode", path, NULL};
            execve("/bin/edid-decode", args, NULL);

            // 如果execve执行失败，需要打印错误信息并退出
            perror("Failed to execute command");
            exit(1);
        } else {
            close(fd_pipe[1]); // 关闭写入端
            fp = fdopen(fd_pipe[0], "r"); // 将读取端转换为文件指针
            if(!fp)
            {
                return NULL;
            }
        }
    }
    char xsize[16] = "\0";
    char ysize[16] = "\0";
    char *tmp = get_val_from_file(fp, "Maximum image size");
    if(!tmp)
    {
        pclose(fp);
        return NULL;
    }
    char *area = (char *)malloc(sizeof(char) * 32);
    if (!area)
    {
        klog_err("内存申请失败：%s\n", strerror(errno));
        pclose(fp);
        return NULL;
    }
    char *msize = get_val_from_file(fp, "Detailed mode");
    if(msize)
    {
        sscanf(msize, "%*s %*s %*s %s %*s %*s %s", xsize, ysize);
        num1 = atoi(xsize);
        num2 = atoi(ysize);
        sprintf(area, "%0.1f X %0.1f mm", num1, num2);
        free(msize);
        free(tmp);
    }
    else if(tmp)
    {
        sscanf(tmp, "%s %*s %*s %s", xsize, ysize);
        num1 = atoi(xsize);
        num2 = atoi(ysize);
        sprintf(area, "%0.1f X %0.1f cm", num1, num2);
        free(tmp);
    }
    pclose(fp);
    return area;
}

static char *manufacturer_name(const unsigned char *x)
{
    static char name[4];

    name[0] = ((x[0] & 0x7c) >> 2) + '@';
    name[1] = ((x[0] & 0x03) << 3) + ((x[1] & 0xe0) >> 5) + '@';
    name[2] = (x[1] & 0x1f) + '@';
    name[3] = 0;

    if (!isupper(name[0]) || !isupper(name[1]) || !isupper(name[2]))
        ;

    return name;
}

char *kdk_edid_get_manufacturer(char *name)
{
    if(!name)
    {
        return NULL;
    }
    int fd;
    unsigned char *edid;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    char filename[100] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);

    char *manufacturer = (char *)malloc(sizeof(char) * 128);
    if(!manufacturer)
    {
        return NULL;
    }
    memset(manufacturer, 0, 128);

    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        free(manufacturer);
        return NULL;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        free(manufacturer);
        closedir(dirfd);
        return NULL;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
                            || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
		// close(fd);
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            closedir(dirfd);
            free(manufacturer);
            return NULL;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            closedir(dirfd);
            free(manufacturer);
            return NULL;
        }
		fclose(fs);
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            closedir(dirfd);
            free(manufacturer);
            return NULL;
        }
        char *msize = get_val_from_file(fp, "Manufacturer");
        if(!msize)
        {
            closedir(dirfd);
            pclose(fp);
            free(manufacturer);
            return NULL;
        }
        char tmp[128] = "\0";
        sscanf(msize, "%s", tmp);
        if(tmp[0] == '\0')
        {
            free(msize);
            closedir(dirfd);
            pclose(fp);
            free(manufacturer);
            return NULL;
        }
        strcpy(manufacturer, tmp);
        free(msize);
        closedir(dirfd);
        pclose(fp);
        return manufacturer;
    }

    edid = extract_edid(fd);
    if (!edid)
    {
        goto err_out;
    }

    if (!edid || memcmp(edid, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8))
    {
        goto err_out;
    }
    close(fd);
    closedir(dirfd);
    char *mn = manufacturer_name(edid + 0x08);
    free(edid);
    if(mn)
    {
        strcpy(manufacturer, mn);
        return manufacturer;
    }
    else
    {
        free(manufacturer);
        return NULL;
    }

err_out:
    close(fd);
    closedir(dirfd);
    free(manufacturer);
    if(edid)
    {
        free(edid);
    }
    return NULL;
}

int kdk_edid_get_week(char *name)
{
    if(!name)
    {
        return -1;
    }
    int fd;
    unsigned char *edid;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    char tmp[20] = {0};
    unsigned edid_minor;
    int wek;
    char filename[100] = "\0";
    char edid_text[11520] = "\0";
    char edid_week[32] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);


    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return -1;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return -1;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
                        || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
		// close(fd);
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            closedir(dirfd);
            return -1;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            closedir(dirfd);
            return -1;
        }
		fclose(fs);
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            closedir(dirfd);
            return -1;
        }
        while(fgets(edid_text, sizeof(edid_text), fp))
        {
            if(strstr(edid_text, "Made in week"))
            {
                strcpy(edid_week, edid_text);
            }
        }
        int year = 0;
        sscanf(edid_week, "%*s %*s %*s %d %*s %d", &wek ,&year);

        closedir(dirfd);
        pclose(fp);
        return wek;
    }

    edid = extract_edid(fd);
    if (!edid)
    {
        goto err_out;
    }

    if (!edid || memcmp(edid, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8))
    {
        goto err_out;
    }
    close(fd);
    closedir(dirfd);

    if (edid[0x12] == 1)
    {
        edid_minor = edid[0x13];
        if (edid_minor > 4)
            ;
        if (edid_minor < 3)
            ;
    }
    else
    {
        ;
    }

    unsigned char week = edid[0x10];

    if (week)
    {
        if (edid_minor <= 3 && week == 0xff)
            ;
        if (edid_minor <= 3 && week == 54)
            ;
        if (week != 0xff && week > 54)
            ;
        if (week != 0xff)
        {
            sprintf(tmp, "%hhu", week);
            wek = strtol(tmp, NULL, 10);
        }
    }
    free(edid);
    return wek;

err_out:
    close(fd);
    closedir(dirfd);
    if(edid)
    {
        free(edid);
    }
    return -1;
}

int kdk_edid_get_year(char *name)
{
    if(!name)
    {
        return -1;
    }
    int fd;
    unsigned char *edid;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    unsigned edid_minor;
    char filename[100] = "\0";
    char edid_text[11520] = "\0";
    char edid_week[32] = "\0";
    int year = 0;
    char str_name[32] = "\0";
    strcpy(str_name, name);


    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return -1;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return -1;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
                        || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
		// close(fd);
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            closedir(dirfd);
            return -1;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            closedir(dirfd);
            return -1;
        }
		fclose(fs);
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            closedir(dirfd);
            return -1;
        }
        while(fgets(edid_text, sizeof(edid_text), fp))
        {
            if(strstr(edid_text, "Made in week"))
            {
                strcpy(edid_week, edid_text);
            }
        }
        int week = 0;
        sscanf(edid_week, "%*s %*s %*s %d %*s %d", &week ,&year);

        closedir(dirfd);
        pclose(fp);
        return year;
    }

    edid = extract_edid(fd);
    if (!edid)
    {
        goto err_out;
    }

    if (!edid || memcmp(edid, "\x00\xFF\xFF\xFF\xFF\xFF\xFF\x00", 8))
    {
        goto err_out;
    }
    close(fd);
    closedir(dirfd);

    if (edid[0x12] == 1)
    {
        edid_minor = edid[0x13];
        if (edid_minor > 4)
            ;
        if (edid_minor < 3)
            ;
    }
    else
    {
        ;
    }
    year = 1990 + edid[0x11];
    free(edid);
    return year;

err_out:
    close(fd);
    closedir(dirfd);
    if(edid)
    {
        free(edid);
    }
    return -1;
}

int kdk_edid_get_primary(char *name)
{
    if(!name)
    {
        return -1;
    }
    int primary = 0;

    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        primary = -1;
        goto out;
    }

    Window root = DefaultRootWindow(display);
    RROutput primary_output = XRRGetOutputPrimary(display, root);

    XRRScreenResources *screen = XRRGetScreenResources(display, root);
    assert(screen);

    for (int i = 0; i < screen->noutput; i++)
    {
        XRROutputInfo *output_info = XRRGetOutputInfo(
            display, screen, screen->outputs[i]);
        assert(output_info);

        if (output_info->connection == RR_Connected)
        {
            XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(
                display, screen, output_info->crtc);
            assert(crtc_info);

            for (int j = 0; j < crtc_info->noutput; j++)
                if (crtc_info->outputs[j] == primary_output)
                {
                    if (strstr(output_info->name, name))
                        primary = 1;
                }
            XRRFreeCrtcInfo(crtc_info);
        }
        XRRFreeOutputInfo(output_info);
    }
    XRRFreeScreenResources(screen);
    XCloseDisplay(display);

out:
    return primary;
}

char *kdk_edid_get_resolution(char *name)
{
    if(!name)
    {
        return NULL;
    }
    Display *disp;
    XRRScreenResources *screen;
    XRROutputInfo *info;
    XRRCrtcInfo *crtc_info;
    int iscres;
    int icrtc;
    char *resolution = NULL;
    char tmp[128];

    disp = XOpenDisplay(0);
    screen = XRRGetScreenResources(disp, DefaultRootWindow(disp));
    for (iscres = screen->noutput; iscres > 0;)
    {
        --iscres;

        info = XRRGetOutputInfo(disp, screen, screen->outputs[iscres]);
        if (info->connection == RR_Connected)
        {
            for (icrtc = info->ncrtc; icrtc > 0;)
            {
                --icrtc;

                crtc_info = XRRGetCrtcInfo(disp, screen, screen->crtcs[icrtc]);
                if (crtc_info->width != 0 || crtc_info->height != 0)
                {
                    if (strstr(info->name, name))
                    {
                        sprintf(tmp, "%dx%d", crtc_info->width, crtc_info->height);
                        resolution = (char *)calloc(strlen(tmp) + 1, sizeof(char));
                        if (!resolution)
                        {
                            XRRFreeCrtcInfo(crtc_info);
                            goto err_out;
                        }
                        strcpy(resolution, tmp);
                    }
                }
                XRRFreeCrtcInfo(crtc_info);
            }
        }
        XRRFreeOutputInfo(info);
    }
    XRRFreeScreenResources(screen);
    XCloseDisplay(disp);
    if(strlen(resolution) == 0)
    {
        free(resolution);
        return NULL;
    }
    return resolution;

err_out:
    XRRFreeOutputInfo(info);
    XRRFreeScreenResources(screen);
    XCloseDisplay(disp);
    return NULL;
}

char* kdk_edid_get_ratio(char *name)
{
    if(!name)
    {
        return NULL;
    }
    Display *disp;
    XRRScreenResources *screen;
    XRROutputInfo *info;
    XRRCrtcInfo *crtc_info;
    int iscres;
    int icrtc;
    char tmp[32] = "\0";
    char *ratio = (char *)malloc(sizeof(char) * 32);
    if(!ratio)
    {
        return NULL;
    }

    disp = XOpenDisplay(0);
    screen = XRRGetScreenResources(disp, DefaultRootWindow(disp));
    for (iscres = screen->noutput; iscres > 0;)
    {
        --iscres;

        info = XRRGetOutputInfo(disp, screen, screen->outputs[iscres]);
        if (info->connection == RR_Connected)
        {
            for (icrtc = info->ncrtc; icrtc > 0;)
            {
                --icrtc;

                crtc_info = XRRGetCrtcInfo(disp, screen, screen->crtcs[icrtc]);
                if (crtc_info->width != 0 || crtc_info->height != 0)
                {
                    if (strstr(info->name, name))
                    {
                        int ret = (crtc_info->width*100 / crtc_info->height);
                        if (ret == 125)
                            strcpy(tmp, "5:4");
                        else if (ret == 133)
                            strcpy(tmp, "4:3");
                        else if (ret == 160)
                            strcpy(tmp, "16:10");
                        else if (ret == 177)
                            strcpy(tmp, "16:9");
                    }
                }
                XRRFreeCrtcInfo(crtc_info);
            }
        }
        XRRFreeOutputInfo(info);
    }
    XRRFreeScreenResources(screen);
    XCloseDisplay(disp);

    if(tmp[0] == "\0")
    {
        free(ratio);
        return NULL;
    }
    strcpy(ratio, tmp);
    return ratio;
}

char* kdk_edid_get_character(char *name)
{
    if(!name)
    {
        return NULL;
    }
    FILE *fd = NULL;
    char *chater = (char *)malloc(sizeof(char) * 2048);
    if(!chater)
    {
        return NULL;
    }
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    unsigned char buf[2048] = "\0";
    char hex[2048] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);


    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        free(chater);
        return NULL;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        free(chater);
        closedir(dirfd);
        return NULL;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
                                     || !(fd = fopen(canonical_filename, "rb")))
    {
        closedir(dirfd);
        free(chater);
        char edid_text[2048] = "\0";
        char *edid_character = (char *)malloc(sizeof(char) * 2048);
        if(!edid_character)
        {
            return NULL;
        }
        FILE *pipeLine = popen("xrandr --prop", "r");  // 建立流管道
        if (!pipeLine)
        {
            free(edid_character);
            return NULL;
        }
        int i = 0;
        memset(edid_character, 0, 2048);
        while(fgets(edid_text, sizeof(edid_text), pipeLine))
        {
            if(i != 1)
            {
                if(strstr(edid_text, name))
                {
                    i = 1;
                    continue;
                }
                else{
                    continue;
                }
            }
	        else{
                if(strstr(edid_text, "EDID"))
                {
                    continue;
                }
                if(strstr(edid_text, "TearFree") || strstr(edid_text, "GAMMA_LUT_SIZE:") || strstr(edid_text, "non-desktop:"))
                {
                        break;
                }

                if(edid_text[0] != '\0')
                {
                    strstripspace(edid_text);
                    strcat(edid_character, edid_text);
                }
                else{
                    free(edid_character);
                    return NULL;
                }
	        }
        }
        return edid_character;
    }

    fread(buf, sizeof(buf), 1, fd);
    long size = ftell(fd);
    fclose(fd);
    closedir(dirfd);

    for (int i = 0; i < size; i++) {
        sprintf(hex + i * 3, "%02X ", buf[i]);
    }
    strcpy(chater, hex);
    return chater;
}

inline void kdk_edid_freeall(char **list)
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

char* kdk_edid_get_serialNumber(char *name)
{
    if(!name)
    {
        return NULL;
    }
    int fd;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    char filename[100] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);
    char tmp[128] = {0};

    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return NULL;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return NULL;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    closedir(dirfd);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
                            || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
		// close(fd);
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            return NULL;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            return NULL;
        }
		fclose(fs);
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            return NULL;
        }
        char *msize = get_val_from_file(fp, "Display Product Serial Number");
        if(!msize)
        {
            pclose(fp);
            return NULL;
        }
        pclose(fp);
        strstripspace(msize);
        strcpy(tmp, msize);
        free(msize);
    }
    else{
        char cmd[128] = {0};
        sprintf(cmd, "edid-decode %s", canonical_filename);
        FILE *fp = popen(cmd, "r");
        if(!fp)
        {
            return NULL;
        }
        char *msize = get_val_from_file(fp, "Display Product Serial Number");
        if(!msize)
        {
            pclose(fp);
            return NULL;
        }
        pclose(fp);
        strstripspace(msize);
        strcpy(tmp, msize);
        free(msize);
    }

    char *serial = (char *)malloc(sizeof(char) * 128);
    if(!serial)
    {
        return NULL;
    }
    memset(serial, 0, 128);
    if(strlen(tmp) != 0)
    {
        strcpy(serial, tmp);
        return serial;
    }
    free(serial);
    return NULL;
}

BrightnessInfo* kdk_edid_get_max_brightness(char *name)
{
    if(!name)
    {
        return NULL;
    }
    BrightnessInfo *info = NULL;
    char *serial = kdk_edid_get_serialNumber(name);
    
    char line[256] = {0};

    char bus_name[32] = {0};
    char **bus = NULL;
    char command[32] = {0};
    int brightness = 0;

    FILE *fp = popen("ddcutil detect", "r");
    if(!fp)
    {
        return NULL;
    }
    while (fgets(line, 256, fp))
    {
        if(strlen(line) == 0)
            break;
        if(strstr(line, "I2C bus:"))
        {
            strstripspace(line);
            strcpy(bus_name, line);
            continue;
        }

        if(serial)
        {
            if(strstr(line, serial))
            {
                break;
            }
        }
    }
    pclose(fp);
    
    
    if(strlen(bus_name) != 0)
    {
        char *p = NULL;
        char *buff[3];
        int i = 0;
        p = strtok(bus_name, "-");
        while(p)
        {
            buff[i] = p;
            i++;
            p = strtok(NULL,"");
        }

        sprintf(command, "ddcutil --bus %s getvcp 10", buff[1]);
        fp = popen(command, "r");
        if(!fp)
        {
            return NULL;
        }
        fgets(line, 256, fp);
        pclose(fp);
        char res[256] = {0};
        sscanf(line, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %s", res);
        strstrip(res, ',');
        brightness = atoi(res);
    }
    if(brightness == 0)
    {
        brightness = 100;
    }
    
    DIR *dirfd = opendir("/sys/class/backlight");
    if (!dirfd)
    {
        return NULL;
    }
    struct dirent *direfd = NULL;
    char path[256] = {0};
    int brightness_value = 0;

    while ((direfd = readdir(dirfd)) != NULL) // 循环读取/proc下的每一个文件/文件夹
    {
        // 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
        if ((strcmp(direfd->d_name, ".") == 0) || (strcmp(direfd->d_name, "..") == 0))
            continue;

        sprintf(path, "/sys/class/backlight/%s/max_brightness", direfd->d_name);
        
    }
    closedir(dirfd);

    fp = fopen(path, "r");
    if(fp)
    {
        fgets(line, 256, fp);
        brightness_value = atoi(line);
        fclose(fp);
    }
    info = (BrightnessInfo *)calloc(1, sizeof(BrightnessInfo));
    info->brightness_value = brightness_value;
    info->brightness_percentage = brightness;
    return info;
}

BrightnessInfo* kdk_edid_get_current_brightness(char *name)
{
    if(!name)
    {
        return NULL;
    }
    BrightnessInfo *info = NULL;
    FILE *fp = NULL;
    int brightness = 0;
    char line[256] = {0};
    
    DIR *dirfd = opendir("/sys/class/backlight");
    if (!dirfd)
    {
        return NULL;
    }
    struct dirent *direfd = NULL;
    char path[512] = {0};

    while ((direfd = readdir(dirfd)) != NULL) // 循环读取/proc下的每一个文件/文件夹
    {
        // 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
        if ((strcmp(direfd->d_name, ".") == 0) || (strcmp(direfd->d_name, "..") == 0))
            continue;
        sprintf(path, "/sys/class/backlight/%s/brightness", direfd->d_name);
    }
    closedir(dirfd);
    int brightness_value = 0;

    fp = fopen(path, "r");
    if(fp)
    {
        fgets(line, 256, fp);
        brightness_value = atoi(line);
        fclose(fp);
    }
    info = (BrightnessInfo *)calloc(1, sizeof(BrightnessInfo));
    
    info->brightness_value = brightness_value;

    BrightnessInfo *max_info = kdk_edid_get_max_brightness(name);
    if(max_info)
    {
        if(max_info->brightness_value != 0)
        {
            float value = ((float)brightness_value / (float)max_info->brightness_value) *100;
            brightness = value;
        }
    }
    
    if(brightness == 0)
    {
        char *serial = kdk_edid_get_serialNumber(name);

        char bus_name[32] = {0};
        char **bus = NULL;

        fp = popen("ddcutil detect", "r");
        if(!fp)
        {
            return NULL;
        }
        while (fgets(line, 256, fp))
        {
            if(strlen(line) == 0)
                break;
            if(strstr(line, "I2C bus:"))
            {
                strstripspace(line);
                strcpy(bus_name, line);
                continue;
            }

            if(serial)
            {
                if(strstr(line, serial))
                {
                    break;
                }
            }
        }
        pclose(fp);

        if(strlen(bus_name) != 0)
        {
            char *p = NULL;
            char *buff[3];
            int i = 0;
            p = strtok(bus_name, "-");
            while(p)
            {
                buff[i] = p;
                i++;
                p = strtok(NULL,"");
            }

            char command[32] = {0};
            sprintf(command, "ddcutil --bus %s getvcp 10", buff[1]);
            fp = popen(command, "r");
            if(!fp)
            {
                return NULL;
            }
            fgets(line, 256, fp);
            char res[256] = {0};
            pclose(fp);
            sscanf(line, "%*s %*s %*s %*s %*s %*s %*s %*s %s", res);
            strstrip(res, ',');
            brightness = atoi(res);
            info->brightness_percentage = brightness;
        }
    }
    else
    {
        info->brightness_percentage = brightness;
    }
    return info;
}

ChromaticityCoordinates* kdk_edid_get_red_primary(char *name)
{
    if(!name)
    {
        return NULL;
    }
    int fd;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    char filename[100] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);
    char tmp[128] = {0};
    ChromaticityCoordinates *info = NULL;

    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return NULL;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return NULL;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    closedir(dirfd);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
                            || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
		// close(fd);
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            return NULL;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            return NULL;
        }
		fclose(fs);
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            return NULL;
        }
        char *msize = get_val_from_file(fp, "  Red");
        if(!msize)
        {
            pclose(fp);
            return NULL;
        }
        pclose(fp);
        strstripspace(msize);
        strcpy(tmp, msize);
        free(msize);
    }
    else{
        char cmd[128] = {0};
        sprintf(cmd, "edid-decode %s", canonical_filename);
        FILE *fp = popen(cmd, "r");
        if(!fp)
        {
            return NULL;
        }
        char *msize = get_val_from_file(fp, "  Red");
        if(!msize)
        {
            pclose(fp);
            return NULL;
        }
        pclose(fp);
        strstripspace(msize);
        strcpy(tmp, msize);
        free(msize);
    }

    if(strlen(tmp) != 0)
    {
        char *p = NULL;
        char *buff[3];
        int i = 0;
        p = strtok(tmp, ",");
        while(p)
        {
            buff[i] = p;
            i++;
            p = strtok(NULL,"");
        }
        strstripspace(buff[0]);
        strstripspace(buff[1]);
        info = (ChromaticityCoordinates *)calloc(1, sizeof(ChromaticityCoordinates));
        if(!info)
        {
            return NULL;
        }
        if(strlen(buff[0]) != 0)
        {
            info->xCoordinate = (char *)calloc(1, sizeof(buff[0]));
            if(!info->xCoordinate)
            {
                free(info);
                return NULL;
            }
            strcpy(info->xCoordinate, buff[0]);
        }
            
        if(strlen(buff[1]) != 0)
        {
            info->yCoordinate = (char *)calloc(1, sizeof(buff[1]));
            if(!info->yCoordinate)
            {
                free(info);
                return NULL;
            }
            strcpy(info->yCoordinate, buff[1]);
        }
    }
    return info;
}

ChromaticityCoordinates* kdk_edid_get_green_primary(char *name)
{
    if(!name)
    {
        return NULL;
    }
    int fd;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    char filename[100] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);
    char tmp[128] = {0};
    ChromaticityCoordinates *info = NULL;

    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return NULL;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return NULL;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    closedir(dirfd);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
                            || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
		// close(fd);
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            return NULL;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            return NULL;
        }
		fclose(fs);
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            return NULL;
        }
        char *msize = get_val_from_file(fp, "  Green");
        if(!msize)
        {
            pclose(fp);
            return NULL;
        }
        pclose(fp);
        strstripspace(msize);
        strcpy(tmp, msize);
        free(msize);
    }
    else{
        char cmd[128] = {0};
        sprintf(cmd, "edid-decode %s", canonical_filename);
        FILE *fp = popen(cmd, "r");
        if(!fp)
        {
            return NULL;
        }
        char *msize = get_val_from_file(fp, "  Green");
        if(!msize)
        {
            pclose(fp);
            return NULL;
        }
        pclose(fp);
        strstripspace(msize);
        strcpy(tmp, msize);
        free(msize);
    }

    if(strlen(tmp) != 0)
    {
        char *p = NULL;
        char *buff[3];
        int i = 0;
        p = strtok(tmp, ",");
        while(p)
        {
            buff[i] = p;
            i++;
            p = strtok(NULL,"");
        }
        strstripspace(buff[0]);
        strstripspace(buff[1]);
        info = (ChromaticityCoordinates *)calloc(1, sizeof(ChromaticityCoordinates));
        if(!info)
        {
            return NULL;
        }
        if(strlen(buff[0]) != 0)
        {
            info->xCoordinate = (char *)calloc(1, sizeof(buff[0]));
            if(!info->xCoordinate)
            {
                free(info);
                return NULL;
            }
            strcpy(info->xCoordinate, buff[0]);
        }
            
        if(strlen(buff[1]) != 0)
        {
            info->yCoordinate = (char *)calloc(1, sizeof(buff[1]));
            if(!info->yCoordinate)
            {
                free(info);
                return NULL;
            }
            strcpy(info->yCoordinate, buff[1]);
        }
    }
    return info;
}

ChromaticityCoordinates* kdk_edid_get_blue_primary(char *name)
{
    if(!name)
    {
        return NULL;
    }
    int fd;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    char filename[100] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);
    char tmp[128] = {0};
    ChromaticityCoordinates *info = NULL;

    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return NULL;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return NULL;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    closedir(dirfd);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
                            || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
		// close(fd);
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            return NULL;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            return NULL;
        }
		fclose(fs);
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            return NULL;
        }
        char *msize = get_val_from_file(fp, "  Blue");
        if(!msize)
        {
            pclose(fp);
            return NULL;
        }
        pclose(fp);
        strstripspace(msize);
        strcpy(tmp, msize);
        free(msize);
    }
    else{
        char cmd[128] = {0};
        sprintf(cmd, "edid-decode %s", canonical_filename);
        FILE *fp = popen(cmd, "r");
        if(!fp)
        {
            return NULL;
        }
        char *msize = get_val_from_file(fp, "  Blue");
        if(!msize)
        {
            pclose(fp);
            return NULL;
        }
        pclose(fp);
        strstripspace(msize);
        strcpy(tmp, msize);
        free(msize);
    }

    if(strlen(tmp) != 0)
    {
        char *p = NULL;
        char *buff[3];
        int i = 0;
        p = strtok(tmp, ",");
        while(p)
        {
            buff[i] = p;
            i++;
            p = strtok(NULL,"");
        }
        strstripspace(buff[0]);
        strstripspace(buff[1]);
        info = (ChromaticityCoordinates *)calloc(1, sizeof(ChromaticityCoordinates));
        if(!info)
        {
            return NULL;
        }
        if(strlen(buff[0]) != 0)
        {
            info->xCoordinate = (char *)calloc(1, sizeof(buff[0]));
            if(!info->xCoordinate)
            {
                free(info);
                return NULL;
            }
            strcpy(info->xCoordinate, buff[0]);
        }
            
        if(strlen(buff[1]) != 0)
        {
            info->yCoordinate = (char *)calloc(1, sizeof(buff[1]));
            if(!info->yCoordinate)
            {
                free(info);
                return NULL;
            }
            strcpy(info->yCoordinate, buff[1]);
        }
    }
    return info;
}

ChromaticityCoordinates* kdk_edid_get_white_primary(char *name)
{
    if(!name)
    {
        return NULL;
    }
    int fd;
    char path[512] = "\0";
    DIR *dirfd = NULL;
    struct dirent *direfd = NULL;
    char canonical_filename[100] = "\0";
    char filename[100] = "\0";
    char str_name[32] = "\0";
    strcpy(str_name, name);
    char tmp[128] = {0};
    ChromaticityCoordinates *info = NULL;

    dirfd = opendir("/sys/class/drm");
    if (!dirfd)
    {
        return NULL;
    }
    char **fg = strsplit(str_name, '-');
    if (!fg)
    {
        closedir(dirfd);
        return NULL;
    }

    while ((direfd = readdir(dirfd)) != NULL)
    {
        if (strstr(direfd->d_name, fg[0]))
        {
            sprintf(path, "/sys/class/drm/%s/edid", direfd->d_name);
        }
    }
    free(fg);
    closedir(dirfd);
    if (!realpath(path, canonical_filename) || !verify_file(canonical_filename) \
                            || (fd = open(canonical_filename, O_RDONLY)) == -1)
    {
		// close(fd);
        kdk_edid(name);
        if (!realpath("/tmp/sdk-edid", filename) || !verify_file(filename))
        {
            return NULL;
        }
        FILE *fs = fopen(filename, "r");
        if(!fs)
        {
            return NULL;
        }
		fclose(fs);
        FILE *fp = popen("edid-decode /tmp/sdk-edid", "r");
        if(!fp)
        {
            return NULL;
        }
        char *msize = get_val_from_file(fp, "  White");
        if(!msize)
        {
            pclose(fp);
            return NULL;
        }
        pclose(fp);
        strstripspace(msize);
        strcpy(tmp, msize);
        free(msize);
    }
    else{
        char cmd[128] = {0};
        sprintf(cmd, "edid-decode %s", canonical_filename);
        FILE *fp = popen(cmd, "r");
        if(!fp)
        {
            return NULL;
        }
        char *msize = get_val_from_file(fp, "  White");
        if(!msize)
        {
            pclose(fp);
            return NULL;
        }
        pclose(fp);
        strstripspace(msize);
        strcpy(tmp, msize);
        free(msize);
    }

    if(strlen(tmp) != 0)
    {
        char *p = NULL;
        char *buff[3];
        int i = 0;
        p = strtok(tmp, ",");
        while(p)
        {
            buff[i] = p;
            i++;
            p = strtok(NULL,"");
        }
        strstripspace(buff[0]);
        strstripspace(buff[1]);
        info = (ChromaticityCoordinates *)calloc(1, sizeof(ChromaticityCoordinates));
        if(!info)
        {
            return NULL;
        }
        if(strlen(buff[0]) != 0)
        {
            info->xCoordinate = (char *)calloc(1, sizeof(buff[0]));
            if(!info->xCoordinate)
            {
                free(info);
                return NULL;
            }
            strcpy(info->xCoordinate, buff[0]);
        }
            
        if(strlen(buff[1]) != 0)
        {
            info->yCoordinate = (char *)calloc(1, sizeof(buff[1]));
            if(!info->yCoordinate)
            {
                free(info);
                return NULL;
            }
            strcpy(info->yCoordinate, buff[1]);
        }
    }
    return info;
}

float kdk_edid_get_rawDpiX(char *name)
{
    if(!name)
    {
        return 0.0;
    }
    char *area = kdk_edid_get_visible_area(name);
    if(!area)
    {
        return 0.0;
    }

    char horization[16] = {0};
    sscanf(area,"%s %*s", horization);
    free(area);

    float inch = 0.0;
    inch = atof(horization) / 25.4;

    char *max = kdk_edid_get_max_resolution(name);
    if(!max)
    {
        return 0.0;
    }
    char *p = NULL;
    char *buff[3];
    int i = 0;
    p = strtok(max, "x");
    while(p)
    {
        buff[i] = p;
        i++;
        p = strtok(NULL,"");
    }

    float reslou = atof(buff[0]);
    float rawDpiX = reslou / inch;
    return rawDpiX;
}

float kdk_edid_get_rawDpiY(char *name)
{
    if(!name)
    {
        return 0.0;
    }
    char *area = kdk_edid_get_visible_area(name);
    if(!area)
    {
        return 0.0;
    }

    char horization[16] = {0};
    sscanf(area,"%*s %*s %s", horization);
    free(area);

    float inch = 0.0;
    inch = atoi(horization) / 25.4;

    char *max = kdk_edid_get_max_resolution(name);
    if(!max)
    {
        return 0.0;
    }
    char *p = NULL;
    char *buff[3];
    int i = 0;
    p = strtok(max, "x");
    while(p)
    {
        buff[i] = p;
        i++;
        p = strtok(NULL,"");
    }

    int reslou = atoi(buff[1]);
    float rawDpiY = reslou / inch;
    return rawDpiY;
}

char* kdk_edid_get_refreshRate(char *name)
{
    if(!name)
    {
        return NULL;
    }
    char line[512] = {0};

    FILE *fp = popen("xrandr", "r");
    if(!fp)
    {
        return NULL;
    }

    char **buffer = NULL;
    char *rate = NULL;
    int i = 0;

    while (fgets(line, 512, fp))
    {
        if(i != 1)
        {
            if(strstr(line, name))
            {
                i = 1;
                continue;
            }
            else{
                continue;
            }
        }
        else{
            if(strstr(line, "*"))
            {
                char* ptr = strchr(line, '*');
                if (ptr != NULL) {
                    char num[10];
                    char* start = ptr;
                    while (start >= line && *start != ' ') {
                        start--;
                    }
                    sscanf(start + 1, "%s", num);

                    char **buffer = strsplit(num, '*');

                    rate = (char *)malloc(32 * sizeof(char));
                    if(!rate)
                        return NULL;

                    if(strlen(buffer[0]) == 0)
                    {
                        free(rate);
                        return NULL;
                    }
                    strcpy(rate, buffer[0]);
                }
                break;
            }
        }
    }  

    return rate;
}

char* kdk_edid_get_rotation(char *name)
{
    if(!name)
    {
        return NULL;
    }
    char line[512] = {0};

    FILE *fp = popen("xrandr", "r");
    if(!fp)
    {
        return NULL;
    }

    char **buffer = NULL;
    char *rate = NULL;
    char res[32] = {0};
    int i = 0;

    while (fgets(line, 512, fp))
    {
        if(strstr(line, name))
        {
            char *p = NULL;
            char *buffer[3];
            int i = 0;
            p = strtok(line, "(");
            while(p)
            {
                buffer[i] = p;
                i++;
                p = strtok(NULL,"");
            }

            if(strstr(buffer[0], "right"))
            {
                strcpy(res, "right");
                break;
            }
            else if(strstr(buffer[0], "left"))
            {
                strcpy(res, "left");
                break;
            }
            else if(strstr(buffer[0], "inverted"))
            {
                strcpy(res, "inverted");
                break;
            }
            else
            {
                strcpy(res, "normal");
                break;
            }
        }
    }
    if(strlen(res) != 0)
    {
        rate = (char *)malloc(32 * sizeof(char));
        if (!rate)
        {
            return NULL;
        }
        strcpy(rate, res);
    }

    return rate;
}

void kdk_free_brightnessInfo(BrightnessInfo *info)
{
    SAFE_FREE(info);
}

void kdk_free_chromaticityCoordinates(ChromaticityCoordinates *info)
{
    SAFE_FREE(info->xCoordinate);
    SAFE_FREE(info->yCoordinate);
    SAFE_FREE(info);
}
