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

#include "libkyprinter.h"
#include <errno.h>
#include <libkylog.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <cups/cups.h>
#include <cups/adminutil.h>
#include <cups/ipp.h>
#include <cups/ppd.h>

#include <curl/curl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

#define _IPP_PRIVATE_STRUCTURES 1

cups_option_t *cups_options; // cups设置
int num_options;             // cups设置

char **kdk_printer_get_list()
{
    // printf("[%s] Start\n", __FUNCTION__);
    cups_dest_t *dests = NULL;
    int num_dests = 0;
    int i = 0;
    char **printers = NULL;

    num_dests = cupsGetDests(&dests);
    if (num_dests == 0 &&
        (cupsLastError() == IPP_STATUS_ERROR_BAD_REQUEST ||
         cupsLastError() == IPP_STATUS_ERROR_VERSION_NOT_SUPPORTED))
    {
#ifdef PRT_DEBUG
        _cupsLangPrintf(stderr, "Printer: Error - add '/version=1.1' to server name.");
#endif
        return NULL;
    }
    for (i = 0; i < num_dests; i++)
    {
        // printf("[%s] printer name is %s \n", __FUNCTION__, dests[i].name);
        char **tmp = realloc(printers, sizeof(char *) * i);
        if(!tmp)
        {
            goto err_out;
        }
        printers = tmp;
        printers[i] = malloc(sizeof(char) * (strlen(dests[i].name) + 1));
        if(!printers[i])
        {
            goto err_out;
        }
        strcpy(printers[i], dests[i].name);
    }
    char **tmp = realloc(printers, sizeof(char *) * (i + 1));
    if(!tmp)
    {
        goto err_out;
    }
    printers = tmp;
    printers[i] = NULL;
    cupsFreeDests(num_dests, dests);
    return printers;
err_out:
    while (i)
    {
        free(printers[i - 1]);
        i--;
    }
    free(printers);
    return NULL;
}

char **kdk_printer_get_available_list()
{
    // printf("[%s] Start\n", __FUNCTION__);
    cups_dest_t *dests = NULL;
    int num_dests = 0;
    int i = 0, j = 0;
    char **printers = NULL;

    num_dests = cupsGetDests(&dests);
    if (num_dests == 0 &&
        (cupsLastError() == IPP_STATUS_ERROR_BAD_REQUEST ||
         cupsLastError() == IPP_STATUS_ERROR_VERSION_NOT_SUPPORTED))
    {
#ifdef PRT_DEBUG
        _cupsLangPrintf(stderr, "Printer: Error - add '/version=1.1' to server name.");
#endif
        return NULL;
    }
    for (i = 0; i < num_dests; i++)
    {
        if (kdk_printer_get_status(dests[i].name) == CUPS_PRINTER_STATE_IDLE)
        {
            // printf("[%s] printer name is %s \n", __FUNCTION__, dests[i].name);
            char **tmp = realloc(printers, sizeof(char *) * j);
            if(!tmp)
            {
                goto err_out;
            }
            printers = tmp;
            printers[j] = malloc(sizeof(char) * (strlen(dests[i].name) + 1));
            if(!printers[j])
            {
                goto err_out;
            }
            strcpy(printers[j], dests[i].name);
            //
            j++;
        }
    }
    char **tmp = realloc(printers, sizeof(char *) * (j + 1));
    if(!tmp)
    {
        goto err_out;
    }
    printers = tmp;
    printers[j] = NULL;

    cupsFreeDests(num_dests, dests);

    return printers;
err_out:
    while (j)
    {
        free(printers[j]);
        j--;
    }
    free(printers);
    return NULL;
}

int kdk_printer_remove_options()
{
    // printf("[%s] Start\n", __FUNCTION__);
    if (num_options != 0 && cups_options != NULL)
    {
        cupsFreeOptions(num_options, cups_options);
        num_options = 0;
        cups_options = NULL;
    }
    // printf("[%s] num_options = %d\n", __FUNCTION__, num_options);
    return 0;
}

void kdk_printer_set_options(int number_up,
                             const char *media,
                             const char *number_up_layout, const char *sides)
{
    // printf("[%s] Start\n", __FUNCTION__);
    kdk_printer_remove_options();
    cups_options = NULL;
    num_options = 0;

    char NumberUp[2] = {0};

    snprintf(NumberUp, 2, "%d", number_up);
    /* cups options */
    num_options = cupsAddOption("number-up", NumberUp, num_options, &cups_options);

    num_options = cupsAddOption("media", media, num_options, &cups_options);

    num_options = cupsAddOption("number-up-layout", number_up_layout, num_options, &cups_options);

    num_options = cupsAddOption("sides", sides, num_options, &cups_options);
}

int kdk_printer_print_local_file(const char *printername, const char *filepath)
{
    // printf("[%s] Start\n", __FUNCTION__);
    // printf("[%s] printer_name: %s, file_path: %s\n", __FUNCTION__,
    //        printername,
    //        filepath);
    int jobid = 0;
    // stop状态禁止打印
    if (kdk_printer_get_status(printername) != CUPS_PRINTER_STATE_STOPPED)
    {
        jobid = cupsPrintFile(printername, filepath, "Print report", num_options, cups_options);
        //注意：这里没有设置格式的话会直接用默认格式，一般默认格式是单页打印
        // printf("[%s] num_options = %d,name = %s,value = %s",
        //     __FUNCTION__,num_options,cups_options->name,cups_options->value);
    }
    return jobid;
}

int kdk_printer_cancel_all_jobs(const char *printername)
{
    // printf("[%s] Start\n", __FUNCTION__);
    int ret = cupsCancelJob(printername, CUPS_JOBID_ALL); // 1=success 0=fail
    if (ret == 0)
    {
        // printf("%s ,job_id = CUPS_JOBID_ALL, find err = %s ", __FUNCTION__, cupsLastErrorString());
        return -1;
    }
    return 0;
}

static int                   /* O - 1 on match, 0 on no match */
match_list(const char *list, /* I - List of names */
           const char *name) /* I - Name to find */
{
    const char *nameptr; /* Pointer into name */
    /*
     * An empty list always matches...
     */

    if (!list || !*list)
        return (1);
    if (!name)
        return (0);
    while (*list)
    {
        /*
         * Skip leading whitespace and commas...
         */
        while (isspace(*list & 255) || *list == ',')
            list++;
        if (!*list)
            break;
        /*
         * Compare names...
         */
        for (nameptr = name;
             *nameptr && *list && tolower(*nameptr & 255) == tolower(*list & 255);
             nameptr++, list++)
            ;
        if (!*nameptr && (!*list || *list == ',' || isspace(*list & 255)))
            return (1);
        while (*list && !isspace(*list & 255) && *list != ',')
            list++;
    }
    return (0);
}

static int                               /* O - 0 on success, 1 on fail */
_show_printer_state(const char *printers /* I - Destinations */
                    // char* buf,	/* I - Number of user-defined dests */
                    ) /* I - User-defined destinations */
{
    int i;                 /* Looping var */
    ipp_t *request,        /* IPP Request */
        *response;         /* IPP Response */
    ipp_attribute_t *attr; /* Current attribute */
    time_t ptime;          /* Printer state time */

    const char *printer, /* Printer name */
        *device;         /* Printer device URI */
    ipp_pstate_t pstate;
    int state_code = 0;
    static const char *pattrs[] = /* Attributes we need for printers... */
        {
            "printer-name",
            "printer-state"};

#ifdef PRT_DEBUG
    // printf(("show_devices(printers=\"%s\")\n", printers));
#endif
    if (printers != NULL && !strcmp(printers, "all"))
        printers = NULL;

    /*
     * Build a CUPS_GET_PRINTERS request, which requires the following
     * attributes:
     *
     *    attributes-charset
     *    attributes-natural-language
     *    requested-attributes
     *    requesting-user-name
     */

    request = ippNewRequest(CUPS_GET_PRINTERS);

    ippAddStrings(request, IPP_TAG_OPERATION, IPP_TAG_KEYWORD,
                  "requested-attributes", sizeof(pattrs) / sizeof(pattrs[0]),
                  NULL, pattrs);

    ippAddString(request, IPP_TAG_OPERATION, IPP_TAG_NAME, "requesting-user-name",
                 NULL, cupsUser());

    /*
     * Do the request and get back a response...
     */

    response = cupsDoRequest(CUPS_HTTP_DEFAULT, request, "/");

    if (cupsLastError() == IPP_STATUS_ERROR_BAD_REQUEST ||
        cupsLastError() == IPP_STATUS_ERROR_VERSION_NOT_SUPPORTED)
    {
#ifdef PRT_DEBUG
        _cupsLangPrintf(stderr,
                        "%s: Error - add '/version=1.1' to server name.",
                        "lpstat");
#endif
        ippDelete(response);
        return (1);
    }
    else if (cupsLastError() > IPP_STATUS_OK_CONFLICTING)
    {
#ifdef PRT_DEBUG
        _cupsLangPrintf(stderr, "lpstat: %s", cupsLastErrorString());
#endif
        ippDelete(response);
        return (1);
    }

    if (response)
    {
        /*
         * Loop through the printers returned in the list and display
         * their devices...
         */
        attr = ippFirstAttribute(response);
        for (; attr != NULL; attr = ippNextAttribute(response))
        {
            /*
             * Skip leading attributes until we hit a job...
             */

            while (attr != NULL && ippGetGroupTag(attr) != IPP_TAG_PRINTER)
                attr = ippNextAttribute(response);

            if (attr == NULL)
                break;

            /*
             * Pull the needed attributes from this job...
             */
            printer = NULL;
            pstate = IPP_PRINTER_IDLE;
            while (attr != NULL && ippGetGroupTag(attr) == IPP_TAG_PRINTER)
            {
                const char *attr_name = ippGetName(attr);
                ipp_tag_t value_tag = ippGetValueTag(attr);
                if (!strcmp(attr_name, "printer-name") &&
                    value_tag == IPP_TAG_NAME)
                    printer = ippGetString(attr, 0, NULL);
                else if (!strcmp(attr_name, "printer-state") &&
                         value_tag == IPP_TAG_ENUM)
                {
                    pstate = (ipp_pstate_t)ippGetInteger(attr, 0);
                    // printf("statecode: %d\n",ippGetInteger(attr,0));
                }

                attr = ippNextAttribute(response);
            }

            /*
             * See if we have everything needed...
             */

            if (printer == NULL)
            {
                if (attr == NULL)
                    break;
                else
                    continue;
            }
            if (match_list(printers, printer))
            {
                switch (pstate)
                {
                case IPP_PRINTER_IDLE:
                    state_code = CUPS_PRINTER_STATE_IDLE;
                    // snprintf(buf,buf_len,PRINTER_STATE_IDLE);
                    break;
                case IPP_PRINTER_PROCESSING:
                    state_code = CUPS_PRINTER_STATE_PROCESSING;
                    // snprintf(buf,buf_len,PRINTER_STATE_PROCESSING);
                    break;
                case IPP_PRINTER_STOPPED:
                    state_code = CUPS_PRINTER_STATE_STOPPED;
                    // snprintf(buf,buf_len,PRINTER_STATE_STOPPED);
                    break;
                case IPP_PRINTER_BUSY:
                    state_code = CUPS_PRINTER_STATE_PROCESSING;
                    // snprintf(buf,buf_len,PRINTER_STATE_PROCESSING);
                    break;
                default:
                    state_code = CUPS_PRINTER_STATE_STOPPED;
                    // snprintf(buf,buf_len,PRINTER_STATE_STOPPED);
                    break;
                }
            }
            if (attr == NULL)
                break;
        }

        ippDelete(response);
    }

    return state_code;
}

int kdk_printer_get_status(const char *printername)
{
    // printf("[%s] Start\n", __FUNCTION__);
    http_t *http; /* Connection to server */
    int ret;
    if ((http = httpConnectEncrypt(cupsServer(), ippPort(),
                                   cupsEncryption())) != NULL)
    {
        httpClose(http);
    }
    else
    {
        return -2;
    }
    return _show_printer_state(printername);
}

size_t processdata(void *buffer, size_t size, size_t nmemb, void *user_p)
{
    return nmemb;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
//获取url内的文件名
char* kdk_printer_get_filename(const char *url)
{
    // printf("[%s] Start\n", __FUNCTION__);
    char *filename = malloc(strlen(url) + 1);
    memset(filename, 0, sizeof(url));
    int len = strlen(url);
    int i;
    for (i = (len - 1); i >= 0; i--)
    {
        if ((url[i] == '\\') || (url[i] == '/'))
        {
            break;
        }
    }
    strcpy(filename, &url[i + 1]);
    return filename;
}
//检查url是否存在
int check_url(char *url)
{
    // printf("[%s] Start\n", __FUNCTION__);
    // 获得一个CURL会话，进行网络操作
    CURL *handle = curl_easy_init();
    if (NULL != handle && NULL != url)
    {
        // 设置本次会话的参数
        // URL，就是我们要验证的网址
        curl_easy_setopt(handle, CURLOPT_URL, url);
        // 设置连接超时
        curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 5);
        // 只是获取HTML的header
        curl_easy_setopt(handle, CURLOPT_HEADER, true);
        curl_easy_setopt(handle, CURLOPT_NOBODY, true);
        // 设置最大重定向数为0，不允许页面重定向
        curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 0);
        // 设置一个空的写入函数，屏蔽屏幕输出
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &processdata);

        // 以上面设置的参数执行这个会话，向服务器发起请求
        curl_easy_perform(handle);
        // 获取HTTP的状态代码
        // 根据代码判断网址是否有效
        int retcode = 0;
        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &retcode);
        int res = URL_UNAVAILABLE; 
        // 如果HTTP反应代码为200，表示网址有效
        if (200 == retcode)
        {
            res = 0;
        }
        // 执行会话的清理工作
        curl_easy_cleanup(handle);

        return res;
    }
    else // 无法验证
    {
        return CAN_NOT_CHECK_URL;
    }
}
// 下载url到文件
int download_file(char *url, const char *savefile)
{
    // printf("[%s] Start\n", __FUNCTION__);
    CURL *curl;
    FILE *fp = NULL;
    CURLcode res;
    // printf("[%s] url = %s \n", __FUNCTION__, url);
    // printf("[%s] savefile = %s \n", __FUNCTION__, savefile);
    //检查保存路径是否可用
    fp = fopen(savefile, "wb");
    // printf("[%s] fp = %d \n", __FUNCTION__, fp);
    if (!fp)
    {
        switch (errno)
        {
            case EACCES: // you don't have permission
                return PATH_NO_PERMISSION;
            case ENOENT: // the file doesn't exists
                return PATH_NOT_EXIST;
            default:
                return PATH_ERROR;
        }
    }
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L); // timeout for the connect phase
        /* abort if slower than 1 bytes/sec during 6 seconds */
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 6);
        res = curl_easy_perform(curl);
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    fclose(fp);
    return res;
}

int kdk_printer_print_download_remote_file(const char *url, const char *filepath)
{
    // printf("[%s] Start\n", __FUNCTION__);
    int res = -1;

    int available = check_url(url);
    if (available == 0)
    {
        res = download_file(url, filepath);
        // printf("res = %d.\n", res);
        return res;
    }
    else
    {
        // printf("%s seems to be unavailable.\n", url);
        return available;
    }

}

int kdk_printer_get_job_status(const char *printername, int jobid)
{
    // printf("[%s] Start\n", __FUNCTION__);
    cups_job_t *jobs = NULL;
    int i;
    int number_jobs;
    int ret;
    // cupsGetJobs函数的第三个参数应该是0而不是1
    // 因为1表示只获取正在进行中的任务，而0表示获取所有任务，包括已完成的和已停止的
    number_jobs = cupsGetJobs(&jobs, printername, 0, CUPS_WHICHJOBS_ALL);
    ipp_jstate_t state = 0;

    for (i = 0; i < number_jobs; i++)
    {
        if (jobs[i].id == jobid)
        {
            state = jobs[i].state;
            // printf("[%s] job state is %d\n", __FUNCTION__, jobs[i].state);
            break;
        }
    }
    cupsFreeJobs(number_jobs, jobs);

    return state;
}

inline void kdk_printer_freeall(char **list)
{
    if (! list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index ++;
    }
    free(list);
}
