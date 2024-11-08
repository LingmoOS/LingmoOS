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

#include "libkysysinfo.h"
#include <dbus-1.0/dbus/dbus.h>
#include <lingmosdk/lingmosdk-base/cstring-extension.h>
#include <lingmosdk/lingmosdk-base/libkylog.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <X11/extensions/Xrandr.h>
#include <dirent.h>
#include <cjson/cJSON.h>
#include <locale.h>
#include <libintl.h>
#include <utmp.h>
#include <time.h>
#include <utime.h>
#include "pci/pci.h"
#include "sys/sysinfo.h"
#include <pthread.h>
#include <glib.h>
#include <sys/stat.h>

#define SAFE_FREE(x) \
    if (x)           \
    {                \
        free(x);     \
        x = NULL;    \
    }
#define LINGMO_ACTIVATION_DBUS_ADDRESS "org.freedesktop.activation"

#define SYSTEM_VERSION_PATH "/etc/lingmo-version/lingmo-system-version.conf"
#define UPDATE_VERSION_PATH "/usr/share/lingmo-update-desktop-config/data/"
#define UPDATE_VERSION_PATH_REPLACE "/usr/share/lingmo-update-desktop-config/config/"
#define VERSION_FILE_PATH "/etc/lingmo-version/lingmo-system-version.conf"
#define OS_RELEASE_PATH "/etc/os-release"
#define LINGMO_RELEASE_ID "LINGMO_RELEASE_ID"
#define MAX_LINE 1024

#define BUF_SIZE 1024

#define LOCALEDIR "/usr/share/locale/"
#define GETTEXT_PACKAGE "lingmosdk-sysinfo"

struct device
{
    struct device *next;
    struct pci_dev *dev;
    /* Bus topology calculated by grow_tree() */
    struct device *bus_next;
    struct bus *parent_bus;
    struct bridge *bridge;
    /* Cache */
    unsigned int config_cached, config_bufsize;
    unsigned char *config;  /* Cached configuration space data */
    unsigned char *present; /* Maps which configuration bytes are present */
};

static uint verify_file(char *pFileName)
{
    return !strncmp(pFileName, "/proc", strlen("/proc"));
}

static char *get_val_from_file(FILE *fp, const char *key)
{
    if (!fp)
        return NULL;
    char *val = NULL;
    char buf[1024] = {0};
    while (fgets(buf, 1024, fp))
    {
        if (strncmp(buf, key, strlen(key)) == 0)
        {
            val = strdup((char *)buf + strlen(key) + 1);
            break;
        }
    }

    return val;
}

char *kdk_system_get_architecture()
{
    char *architecture = NULL;
#ifdef __linux__
    FILE *fp = fopen("/proc/osinfo", "r");
    if (!fp)
        return NULL;

    architecture = get_val_from_file(fp, "Architecture");
    if (!architecture)
    {
        fclose(fp);
        return NULL;
    }
    strstrip(architecture, '\n');
    strstrip(architecture, '\t');

    fclose(fp);
#endif
    return architecture;
}

char *kdk_system_get_systemName()
{
    char *sysname = NULL;
#ifdef __linux__
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp)
        return NULL;

    sysname = get_val_from_file(fp, "NAME");
    if (!sysname)
        return NULL;
    strstrip(sysname, '\n');
    strstrip(sysname, '\"');

    fclose(fp);
#endif
    return sysname;
}

char *kdk_system_get_version(bool verbose)
{
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);
    char *sysversion = NULL;
    if (verbose)
    {
#ifdef __linux__
        FILE *fp = fopen("/etc/.kyinfo", "r");
        if (!fp)
            return NULL;
        sysversion = get_val_from_file(fp, "milestone");
        if (!sysversion)
        {
            fclose(fp);
            return NULL;
        }
        fclose(fp);
    }
    else
    {
        char *version_us = NULL;
        char *version = (char *)malloc(sizeof(char) * 128);
        if (!version)
        {
            return NULL;
        }
        memset(version, 0, 128);
        FILE *fp = fopen("/etc/os-release", "r");
        if (!fp)
        {
            free(version);
            return NULL;
        }

        version_us = get_val_from_file(fp, "VERSION_US");
        if (!version_us)
        {
            fclose(fp);
            free(version);
            return NULL;
        }
        strstrip(version_us, '\"');
        strstrip(version_us, '\n');
        strstrip(version_us, '\"');
        fclose(fp);
        if (strcmp(version_us, "Lingmo Linux Desktop V10 (SP1)") == 0)
        {
            sprintf(version, gettext("Lingmo Linux Desktop V10 (SP1)"));
        }
        free(version_us);
        if (strlen(version) == 0)
        {
            fp = fopen("/etc/os-release", "r");
            if (!fp)
            {
                free(version);
                return NULL;
            }
            sysversion = get_val_from_file(fp, "VERSION");
            if (!sysversion)
            {
                fclose(fp);
                free(version);
                return NULL;
            }
            strstrip(sysversion, '\"');
            fclose(fp);
            free(version);
        }
        else
        {
            return version;
        }
    }
    strstrip(sysversion, '\n');
    strstrip(sysversion, '\"');
#endif
    return sysversion;
}

int kdk_system_get_activationStatus(int *status_error_num, int *date_error_num)
{
    if (NULL == status_error_num || NULL == date_error_num)
    {
        klog_err("激活状态接口调用失败：参数错误\n");
        return -1;
    }

    int res = 0;

// #define _LINGMO_ACTIVATION_H_
#ifdef __linux__
#ifdef _LINGMO_ACTIVATION_H_
    int err;
    int ret = -1;

    typedef int (*lingmo_activation_activate_status)(int *);
    typedef int (*lingmo_activation_trial_status)(int *);
    void *hwnd = dlopen("/usr/lib/liblingmo-activation.so", RTLD_LAZY);

    if (!hwnd)
    {
        klog_err("加载liblingmo-activation.so失败\n");
        return ret;
    }

    do
    {
        lingmo_activation_activate_status plingmo_activation_activate_status = (lingmo_activation_activate_status)dlsym(hwnd, "lingmo_activation_activate_status");
        lingmo_activation_trial_status plingmo_activation_trial_status = (lingmo_activation_trial_status)dlsym(hwnd, "lingmo_activation_trial_status");

        if (!plingmo_activation_activate_status || !plingmo_activation_trial_status)
        {
            klog_err("获取接口地址失败\n");
            break;
        }
        res = plingmo_activation_activate_status(&err);
        if (err != 0)
        {
            klog_err("激活状态获取失败：%d\n", err);
            ret = 0;
            break;
        }
        if (res)
        {
            ret = 1;
            break;
        }

        res = plingmo_activation_trial_status(&err);
        if (err != 0)
        {
            klog_err("试用状态获取失败：%d\n", err);
            ret = 0;
            break;
        }

        if (res == 1)
        {
            ret = 0;
            break;
        }
    } while (false);

    dlclose(hwnd);
    return ret;
#else  // 修改dbus通信
    DBusConnection *conn;
    DBusError err;
    int ret;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        return -1;
    }

    DBusMessage *status_msg = NULL;
    DBusMessage *status_pending_msg = NULL;
    DBusMessage *date_msg = NULL;
    DBusMessage *date_pending_msg = NULL;
    DBusMessageIter args;
    DBusPendingCall *status_pending = NULL;
    DBusPendingCall *date_pending = NULL;
    int stat;

    // 获取激活状态
    status_msg = dbus_message_new_method_call("org.freedesktop.activation",           // target for the method call
                                              "/org/freedesktop/activation",          // object to call on
                                              "org.freedesktop.activation.interface", // interface to call on
                                              "status");                              // method name

    if (!status_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        return -1;
    }

    if (!dbus_connection_send_with_reply(conn, status_msg, &status_pending, -1))
    { // -1 is default timeout
        klog_err("status_msg：dbus_connection_send_with_reply调用失败\n");
        return -1;
    }
    if (NULL == status_pending)
    {
        klog_err("status_pending：返回值为null\n");
        return -1;
    }
    dbus_connection_flush(conn);
    dbus_message_unref(status_msg);

    // 接收返回值
    dbus_pending_call_block(status_pending);
    status_pending_msg = dbus_pending_call_steal_reply(status_pending);
    if (NULL == status_pending_msg)
    {
        klog_err("status_pending_msg：获取激活状态接口返回值异常\n");
        return -1;
    }

    // 释放status_pending
    dbus_pending_call_unref(status_pending);

    if (!dbus_message_iter_init(status_pending_msg, &args))
    {
        klog_err("status_pending_msg：dbus_message_iter_init获取激活状态接口返回值异常\n");
        return -1;
    }
    else
        dbus_message_iter_get_basic(&args, &stat);
    if (!dbus_message_iter_next(&args))
    {
        klog_err("status_pending_msg：dbus_message_iter_next获取激活状态接口返回值异常\n");
        return -1;
    }
    else
        dbus_message_iter_get_basic(&args, status_error_num);
    klog_debug("激活状态：%d，错误码：%d\n", stat, *status_error_num);

    // 释放status_pending_msg
    dbus_message_unref(status_pending_msg);

    if (stat == 0)
    {
        char *date_stat;

        // 获取技术服务日期
        date_msg = dbus_message_new_method_call("org.freedesktop.activation",           // target for the method call
                                                "/org/freedesktop/activation",          // object to call on
                                                "org.freedesktop.activation.interface", // interface to call on
                                                "date");                                // method name
        if (!dbus_connection_send_with_reply(conn, date_msg, &date_pending, -1))
        { // -1 is default timeout
            klog_err("date_msg：dbus_connection_send_with_reply 接口调用失败\n");
            return -1;
        }
        if (NULL == date_pending)
        {
            klog_err("date_pending：返回值为null\n");
            return -1;
        }
        dbus_connection_flush(conn);
        dbus_message_unref(date_msg);
        // 接收返回值
        dbus_pending_call_block(date_pending);
        date_pending_msg = dbus_pending_call_steal_reply(date_pending);

        if (NULL == date_pending_msg)
        {
            klog_err("date_pending_msg：Reply Null\n");
            return -1;
        }
        if (!dbus_message_iter_init(date_pending_msg, &args))
        {
            dbus_message_unref(date_pending_msg);
            klog_err("date_pending_msg：dbus_message_iter_init获取技术服务日期接口返回值异常\n");
            return -1;
        }
        else
            dbus_message_iter_get_basic(&args, &date_stat);
        if (!dbus_message_iter_next(&args))
        {
            dbus_message_unref(date_pending_msg);
            klog_err("date_pending_msg：dbus_message_iter_next获取技术服务日期接口返回值异常\n");
            return -1;
        }
        else
            dbus_message_iter_get_basic(&args, date_error_num);
        klog_debug("技术服务日期：%s，错误码：%d\n", date_stat, *date_error_num);
        if (*date_error_num == 0)
        {
            if (strcmp(date_stat, ""))
            {
                dbus_message_unref(date_pending_msg);
                return 2;
            }
            else
            {
                dbus_message_unref(date_pending_msg);
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else if (stat == 1)
    {
        if (conn)
        {
            dbus_connection_close(conn);
        }
        
        return 1;
    }

    // #endif  // _LINGMO_ACTIVATION_H_
    // #endif  // __linux__
    if (status_msg)
    {
        dbus_message_unref(status_msg);
    }
    if (status_pending_msg)
    {
        dbus_message_unref(status_pending_msg);
    }
    if (date_msg)
    {
        dbus_message_unref(date_msg);
    }
    if (date_pending_msg)
    {
        dbus_message_unref(date_pending_msg);
    }
    if (status_pending)
    {
        dbus_pending_call_unref(status_pending);
    }
    if (date_pending)
    {
        dbus_pending_call_unref(date_pending);
    }
    if (conn)
    {
        dbus_connection_unref(conn);
    }
    return 0;
#endif // _LINGMO_ACTIVATION_H_
#endif // __linux__
}

char *kdk_system_get_serialNumber()
{
    char *serial = NULL;
#ifdef __linux__
#ifndef _LINGMO_ACTIVATION_H_ // 修改成dbus通信
    FILE *fp = fopen("/etc/.kyinfo", "r");
    if (!fp)
        return NULL;

    serial = get_val_from_file(fp, "key");
    fclose(fp);
    if (serial)
        strskipspace(serial);
#else
    int err;

    typedef char *(*lingmo_activation_get_serial_number)(int *);
    void *hwnd = dlopen("/usr/lib/liblingmo-activation.so", RTLD_LAZY);

    if (!hwnd)
    {
        klog_err("加载liblingmo-activation.so失败\n");
        return serial;
    }
    lingmo_activation_get_serial_number plingmo_activation_get_serial_number = (lingmo_activation_get_serial_number)dlsym(hwnd, "lingmo_activation_get_serial_number");
    if (!plingmo_activation_get_serial_number)
    {
        klog_err("加载接口lingmo_activation_get_serial_number失败\n");
        return serial;
    }
    serial = plingmo_activation_get_serial_number(&err);
    if (!serial)
    {
        klog_err("序列号获取失败：%d\n", err);
    }
    else
    {
        strskipspace(serial);
    }
    dlclose(hwnd);
    return serial;
#endif // _LINGMO_ACTIVATION_H_
#endif // __linux__
    return serial;
}

char *kdk_system_get_kernelVersion()
{
    char *kernver = NULL;
#ifdef __linux__
    FILE *fp = fopen("/proc/version", "r");
    if (!fp)
        return NULL;

    char buf[1024];
    if (fgets(buf, 1024, fp) == NULL)
    {
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    char *p = buf;
    size_t pos = strfirstof(p, ' ');
    p += pos + 1;
    pos = strfirstof(p, ' ');
    p += pos + 1;
    size_t endpos = strfirstof(p, ' ');
    kernver = strndup(p, endpos);
#endif

    return kernver;
}

char *kdk_system_get_eUser()
{
    char *e_user = NULL;

    DBusConnection *connection = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusMessageIter iter;

    // 初始化DBus连接
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL)
    {
        fprintf(stderr, "DBus connection error: %s\n", error.message);
        goto out;
    }

    // 创建DBus方法调用消息
    message = dbus_message_new_method_call("org.freedesktop.login1",
                                           "/org/freedesktop/login1",
                                           "org.freedesktop.login1.Manager",
                                           "ListUsers");
    if (message == NULL)
    {
        fprintf(stderr, "DBus message allocation failed\n");
        goto out;
    }

    // 发送DBus消息并等待回复
    reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
    if (reply == NULL)
    {
        fprintf(stderr, "DBus reply error: %s\n", error.message);
        goto out;
    }

    // 释放message
    dbus_message_unref(message);
    message = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_STRUCT)
    {
        fprintf(stderr, "DBus reply parsing failed\n");
        goto out;
    }

    // 释放reply
    dbus_message_unref(reply);
    reply = NULL;

    int element_count = dbus_message_iter_get_element_count(&iter);
    char **users = calloc(element_count + 1, sizeof(char *));
    if (NULL == users)
        goto out;

    dbus_message_iter_recurse(&iter, &iter);

    // 解析返回值
    // 循环中过程中调用新的dbus接口会导致迭代器iter被清空，但是没细查原因
    // 先用一个字符串数组存储所有用户的object path
    for (int i = 0; i < element_count; i++)
    {
        char *path = NULL;

        DBusMessageIter struct_iter;
        dbus_message_iter_recurse(&iter, &struct_iter);
        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_get_basic(&struct_iter, &path);

        users[i] = strdup(path);

        dbus_message_iter_next(&iter);
    }

    for (int i = 0; i < element_count; i++)
    {
        char *interface = "org.freedesktop.login1.User";
        char *state = "State";
        char *property = "Name";

        message = dbus_message_new_method_call("org.freedesktop.login1",
                                               users[i],
                                               "org.freedesktop.DBus.Properties",
                                               "Get");
        if (message == NULL)
        {
            fprintf(stderr, "DBus message allocation failed\n");
            goto out;
        }

        dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &state, DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            fprintf(stderr, "DBus reply error: %s\n", error.message);
            goto out;
        }

        dbus_message_unref(message);
        message = NULL;

        if (!dbus_message_iter_init(reply, &iter) ||
            dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
        {
            fprintf(stderr, "DBus reply parsing failed\n");
            goto out;
        }

        dbus_message_unref(reply);
        reply = NULL;

        char *active = NULL;
        dbus_message_iter_recurse(&iter, &iter);
        dbus_message_iter_get_basic(&iter, &active);

        if (0 == strcmp(active, "active"))
        {
            message = dbus_message_new_method_call("org.freedesktop.login1",
                                                   users[i],
                                                   "org.freedesktop.DBus.Properties",
                                                   "Get");
            if (message == NULL)
            {
                fprintf(stderr, "DBus message allocation failed\n");
                goto out;
            }

            dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

            reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
            if (reply == NULL)
            {
                fprintf(stderr, "DBus reply error: %s\n", error.message);
                goto out;
            }

            dbus_message_unref(message);
            message = NULL;

            if (!dbus_message_iter_init(reply, &iter) ||
                dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
            {
                fprintf(stderr, "DBus reply parsing failed\n");
                goto out;
            }

            dbus_message_unref(reply);
            reply = NULL;

            dbus_message_iter_recurse(&iter, &iter);
            dbus_message_iter_get_basic(&iter, &e_user);

            break;
        }
    }

    if (e_user)
        e_user = strdup(e_user);
out:
    if (users)
        g_strfreev(users);
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);
    if (message)
        dbus_message_unref(message);
    if (reply)
        dbus_message_unref(reply);
    if (connection)
        dbus_connection_unref(connection);
    return e_user;
}

char *kdk_system_get_eUser_login_time()
{
    dbus_uint64_t time_stamp = 0;
    char *login_time = NULL;

    DBusConnection *connection = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusMessageIter iter;

    // 初始化DBus连接
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL)
    {
        fprintf(stderr, "DBus connection error: %s\n", error.message);
        goto out;
    }

    // 创建DBus方法调用消息
    message = dbus_message_new_method_call("org.freedesktop.login1",
                                           "/org/freedesktop/login1",
                                           "org.freedesktop.login1.Manager",
                                           "ListUsers");
    if (message == NULL)
    {
        fprintf(stderr, "DBus message allocation failed\n");
        goto out;
    }

    // 发送DBus消息并等待回复
    reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
    if (reply == NULL)
    {
        fprintf(stderr, "DBus reply error: %s\n", error.message);
        goto out;
    }

    // 释放message
    dbus_message_unref(message);
    message = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_STRUCT)
    {
        fprintf(stderr, "DBus reply parsing failed\n");
        goto out;
    }

    // 释放reply
    dbus_message_unref(reply);
    reply = NULL;

    int element_count = dbus_message_iter_get_element_count(&iter);
    char **users = calloc(element_count + 1, sizeof(char *));
    if (NULL == users)
        goto out;

    dbus_message_iter_recurse(&iter, &iter);

    // 解析返回值
    // 循环中过程中调用新的dbus接口会导致迭代器iter被清空，但是没细查原因
    // 先用一个字符串数组存储所有用户的object path
    for (int i = 0; i < element_count; i++)
    {
        char *path = NULL;

        DBusMessageIter struct_iter;
        dbus_message_iter_recurse(&iter, &struct_iter);
        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_get_basic(&struct_iter, &path);

        users[i] = strdup(path);

        dbus_message_iter_next(&iter);
    }

    char *interface = "org.freedesktop.login1.User";
    char *state = "State";
    char *property = "Timestamp";

    for (int i = 0; i < element_count; i++)
    {
        message = dbus_message_new_method_call("org.freedesktop.login1",
                                               users[i],
                                               "org.freedesktop.DBus.Properties",
                                               "Get");
        if (message == NULL)
        {
            fprintf(stderr, "DBus message allocation failed\n");
            goto out;
        }

        dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &state, DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL)
        {
            fprintf(stderr, "DBus reply error: %s\n", error.message);
            goto out;
        }

        dbus_message_unref(message);
        message = NULL;

        if (!dbus_message_iter_init(reply, &iter) ||
            dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
        {
            fprintf(stderr, "DBus reply parsing failed\n");
            goto out;
        }

        dbus_message_unref(reply);
        reply = NULL;

        char *active = NULL;
        dbus_message_iter_recurse(&iter, &iter);
        dbus_message_iter_get_basic(&iter, &active);

        if (0 == strcmp(active, "active"))
        {
            message = dbus_message_new_method_call("org.freedesktop.login1",
                                                   users[i],
                                                   "org.freedesktop.DBus.Properties",
                                                   "Get");
            if (message == NULL)
            {
                fprintf(stderr, "DBus message allocation failed\n");
                goto out;
            }

            dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

            reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
            if (reply == NULL)
            {
                fprintf(stderr, "DBus reply error: %s\n", error.message);
                goto out;
            }

            dbus_message_unref(message);
            message = NULL;

            if (!dbus_message_iter_init(reply, &iter) ||
                dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
            {
                fprintf(stderr, "DBus reply parsing failed\n");
                goto out;
            }

            dbus_message_unref(reply);
            reply = NULL;

            dbus_message_iter_recurse(&iter, &iter);
            dbus_message_iter_get_basic(&iter, &time_stamp);

            break;
        }
    }

    if (0 == time_stamp)
        goto out;

    time_t t1 = time_stamp / 1000000;
    struct tm *timeDate = localtime(&t1);

    login_time = calloc(32, sizeof(char));
    sprintf(login_time, "%04d-%02d-%02d %02d:%02d:%02d", timeDate->tm_year + 1900, timeDate->tm_mon + 1,
            timeDate->tm_mday, timeDate->tm_hour, timeDate->tm_min, timeDate->tm_sec);

out:
    if (users)
        g_strfreev(users);
    if (dbus_error_is_set(&error))
        dbus_error_free(&error);
    if (message)
        dbus_message_unref(message);
    if (reply)
        dbus_message_unref(reply);
    if (connection)
        dbus_connection_unref(connection);
    return login_time;
}

char *kdk_system_get_projectName()
{
    char *project_codename = NULL;
#ifdef __linux__
    FILE *fp = fopen("/etc/lsb-release", "rt");
    if (fp)
    {
        project_codename = get_val_from_file(fp, "PROJECT_CODENAME");
        fclose(fp);
    }

    if (!project_codename)
    {
        fp = fopen("/etc/os-release", "rt");
        ASSERT_NOT_NULL(fp, NULL);
        project_codename = get_val_from_file(fp, "PROJECT_CODENAME");
        fclose(fp);
    }
    if (project_codename)
        strstripspace(project_codename);
#endif
    return project_codename;
}

char *kdk_system_get_projectSubName()
{
    char *project_subcodename = NULL;
#ifdef __linux__
    FILE *fp = fopen("/etc/lsb-release", "rt");
    if (fp)
    {
        project_subcodename = get_val_from_file(fp, "SUB_PROJECT_CODENAME");
        fclose(fp);
    }

    if (!project_subcodename)
    {
        fp = fopen("/etc/os-release", "rt");
        ASSERT_NOT_NULL(fp, NULL);
        project_subcodename = get_val_from_file(fp, "SUB_PROJECT_CODENAME");
        fclose(fp);
    }
    if (project_subcodename)
        strstripspace(project_subcodename);
#endif
    return project_subcodename;
}

unsigned int kdk_system_get_productFeatures()
{
    char *product_features = NULL;
    unsigned int res = 0;
#ifdef __linux__
    FILE *fp = fopen("/etc/lsb-release", "rt");
    if (fp)
    {
        product_features = get_val_from_file(fp, "PRODUCT_FEATURES");
        fclose(fp);
    }

    if (!product_features)
    {
        fp = fopen("/etc/os-release", "rt");
        ASSERT_NOT_NULL(fp, 0);
        product_features = get_val_from_file(fp, "PRODUCT_FEATURES");
        fclose(fp);
    }
    if (product_features)
    {
        strstripspace(product_features);
        res = atoi(product_features);
    }
#endif

    free(product_features);
    return res;
}

char *kdk_system_get_hostVirtType()
{
    char *virtType = (char *)malloc(sizeof(char) * 65);
    ASSERT_NOT_NULL(virtType, NULL);
#ifdef __linux__
    FILE *pipeLine = popen("systemd-detect-virt", "r"); // 建立流管道
    if (!pipeLine)
    {
        free(virtType);
        return NULL;
    }
    if (fgets(virtType, 64 * sizeof(char), pipeLine) == NULL)
    {
        SAFE_FREE(virtType);
        pclose(pipeLine);
        return NULL;
    }
    pclose(pipeLine);
    strstripspace(virtType);
    if (strcmp(virtType, "microsoft") == 0)
        strcpy(virtType, "hyper-v");
    else if (strcmp(virtType, "oracle") == 0)
        strcpy(virtType, "orcale virtualbox");
#endif
    return virtType;
}

char *kdk_system_get_hostCloudPlatform()
{
    char *cloudplat = (char *)malloc(sizeof(char) * 65);
    ASSERT_NOT_NULL(cloudplat, NULL);
#ifdef __linux__
    cloudplat[0] = 0;
    char buf[256] = {0};
    bool res_flag = false;
    char *type = NULL;

    if (!access("/usr/local/ctyun/clink/Mirror/Registry/Default", F_OK))
    {
        strcpy(cloudplat, "ctyun");
        return cloudplat;
    }

    FILE *fp = fopen("/etc/hw-cloud.conf", "rt");
    if (fp)
    {
        type = get_val_from_file(fp, "platform");
        fclose(fp);
    }
    if(type)
    {
        strstripspace(type);
        return type;
    }

    if (geteuid() == 0) // root 用户，可以用dmidecode
    {
        FILE *pipeLine = popen("dmidecode -s chassis-manufacturer", "r");
        if (__glibc_likely(pipeLine != NULL))
        {
            fgets(buf, 255 * sizeof(char), pipeLine);
            strstripspace(buf);
            if (strcmp(buf, "Huawei Inc.") == 0) // 华为云
            {
                strcpy(cloudplat, "huawei");
                res_flag = true;
            }
            pclose(pipeLine);
        }
        if (!res_flag)
        {
            pipeLine = popen("dmidecode -s chassis-asset-tag", "r");
            if (__glibc_likely(pipeLine != NULL))
            {
                fgets(buf, 255 * sizeof(char), pipeLine);
                strstripspace(buf);
                if (strcmp(buf, "HUAWEICLOUD") == 0) // 华为云
                {
                    strcpy(cloudplat, "huawei");
                }
                pclose(pipeLine);
            }
        }
    }
    else // 普通用户，只能读取文件
    {
        FILE *fp = fopen("/sys/devices/virtual/dmi/id/chassis_vendor", "rt");
        if (__glibc_likely(fp != NULL))
        {
            fgets(buf, 255 * sizeof(char), fp);
            strstripspace(buf);
            if (strcmp(buf, "Huawei Inc.") == 0) // 华为云
            {
                strcpy(cloudplat, "huawei");
                res_flag = true;
            }
            fclose(fp);
        }
        if (!res_flag)
        {
            fp = fopen("/sys/devices/virtual/dmi/id/chassis_asset_tag", "r");
            if (__glibc_likely(fp != NULL))
            {
                fgets(buf, 255 * sizeof(char), fp);
                strstripspace(buf);
                if (strcmp(buf, "HUAWEICLOUD") == 0) // 华为云
                {
                    strcpy(cloudplat, "huawei");
                }
                fclose(fp);
            }
        }
    }

    if (strlen(cloudplat) == 0)
        strcpy(cloudplat, "none");
#endif
    return cloudplat;
}

bool kdk_system_is_zyj(void)
{
    bool bool_value = false;
    char *project_codename = kdk_system_get_projectName();

    if (project_codename && (strstr(project_codename, "zyj") || strstr(project_codename, "ZYJ")))
        bool_value = TRUE;

    SAFE_FREE(project_codename);
    return bool_value;
}

char *readReleaseFile()
{
    char buf[MAX_LINE];
    FILE *fp;
    int len;
    static char *os_version = NULL;
    char version[32] = "\0";

    if ((fp = fopen(OS_RELEASE_PATH, "r")) == NULL)
    {
        return os_version;
    }

    os_version = get_val_from_file(fp, "LINGMO_RELEASE_ID");
    if (!os_version)
    {
        fclose(fp);
        return NULL;
    }
    strstrip(os_version, '\"');
    strstripspace(os_version);
    fclose(fp);
    return os_version;
}

char *parseJsonText(char *text)
{
    char *out;
    cJSON *json;
    cJSON *childJson = NULL;
    char *version;

    // 获取补丁版本
    json = cJSON_Parse(text);
    if (!json)
    {
        version = NULL;
    }
    else
    {
        childJson = cJSON_GetObjectItem(json, "version");
        if (!childJson)
        {
            version = NULL;
        }
        else
        {
            version = strdup(childJson->valuestring);
        }
        out = cJSON_Print(json);
        cJSON_Delete(childJson);
        free(out);
    }
    return version;
}

char *readJsonFile(char *json_file)
{
    FILE *fp;
    long len;
    char *data;
    static char *update_version;

    fp = fopen(json_file, "rb");
    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data = (char *)malloc(len + 1);
    fread(data, 1, len, fp);
    fclose(fp);
    update_version = parseJsonText(data);
    free(data);
    return update_version;
}

void getDefaultVersion(version_t *ver)
{
    char *update_version, *os_version;
    os_version = readReleaseFile(); // os-version
    asprintf(&ver->os_version, "%s", os_version);
    if ((access(UPDATE_VERSION_PATH_REPLACE "lingmo-update-desktop-system.json", F_OK) == 0) ||
        (access(UPDATE_VERSION_PATH "lingmo-update-desktop-system.json", F_OK) == 0))
    {
        if (access(UPDATE_VERSION_PATH_REPLACE "lingmo-update-desktop-system.json", F_OK) == 0)
        {
            update_version = readJsonFile(UPDATE_VERSION_PATH_REPLACE "lingmo-update-desktop-system.json"); // config update version
        }
        else if (access(UPDATE_VERSION_PATH "lingmo-update-desktop-system.json", F_OK) == 0)
        {
            update_version = readJsonFile(UPDATE_VERSION_PATH "lingmo-update-desktop-system.json"); // data update version
        }
        asprintf(&ver->update_version, "%s", update_version);
    }
    else
    {
        ;
    }
}

/** 函数名：      GetIniKeyString
*** 入口参数：    section :配置文件中一组数据的标识
***              key :这组数据中要读出的值的标识
***             filename :要读取的文件路径
*** 返回值：      找到需要查的值则返回正确结果 0
***              否则返回-1     */
int GetIniKeyString(char *section, char *key, char *filename, char *buf)
{
    FILE *fp;
    int flag = 0;
    char sTitle[64], *wTmp;
    char sLine[1024];
    sprintf(sTitle, "[%s]", section);

    if (NULL == (fp = fopen(filename, "r")))
    {
        perror("fopen");
        return -1;
    }
    while (NULL != fgets(sLine, 1024, fp))
    {
        // 这是注释行
        if (0 == strncmp("//", sLine, 2))
            continue;
        if ('#' == sLine[0])
            continue;
        wTmp = strchr(sLine, '=');
        if ((NULL != wTmp) && (1 == flag))
        {
            if (0 == strncmp(key, sLine, strlen(key)))
            { // 长度依文件读取的为准
                sLine[strlen(sLine) - 1] = '\0';
                fclose(fp);
                while (*(wTmp + 1) == ' ')
                {
                    wTmp++;
                }
                strcpy(buf, wTmp + 1);
                return 0;
            }
        }
        else
        {
            if (0 == strncmp(sTitle, sLine, strlen(sTitle)))
            {             // 长度依文件读取的为准
                flag = 1; // 找到标题位置
            }
        }
    }
    fclose(fp);
    return -1;
}

/** 函数名:         PutIniKeyString
*** 入口参数:        section:配置文件中一组数据的标识
***                key:这组数据中要读出的值的标识
***                val:更改后的值
***                filename:要读取的文件路径
*** 返回值:         成功返回  0
***                否则返回 -1      */
int PutIniKeyString(char *section, char *key, char *val, char *filename)
{
    FILE *fpr, *fpw;
    int flag = 0;
    char sLine[1024], sTitle[32], *wTmp;
    sprintf(sTitle, "[%s]", section);
    if (NULL == (fpr = fopen(filename, "r")))
        return -1; // 读取原文件
    sprintf(sLine, "%s.tmp", filename);
    if (NULL == (fpw = fopen(sLine, "w")))
    {
        fclose(fpr);
        return -1; // 写入临时文件
    }
    while (NULL != fgets(sLine, 1024, fpr))
    {
        if (2 != flag)
        { // 如果找到要修改的那一行，则不会执行内部的操作
            wTmp = strchr(sLine, '=');
            if ((NULL != wTmp) && (1 == flag))
            {
                if (0 == strncmp(key, sLine, strlen(key)))
                {             // 长度依文件读取的为准
                    flag = 2; // 更改值，方便写入文件
                    sprintf(wTmp + 1, " %s\n", val);
                }
            }
            else
            {
                if (0 == strncmp(sTitle, sLine, strlen(sTitle)))
                {             // 长度依文件读取的为准
                    flag = 1; // 找到标题位置
                }
            }
        }
        fputs(sLine, fpw); // 写入临时文件
    }
    fclose(fpr);
    fclose(fpw);
    sprintf(sLine, "%s.tmp", filename);
    return rename(sLine, filename); // 将临时文件更新到原文件
}

void getCurrentVersion(version_t *ver)
{
    char buff[100];

    if (0 == GetIniKeyString("SYSTEM", "os_version", VERSION_FILE_PATH, buff))
    {
        if (ver->os_version == NULL)
            asprintf(&ver->os_version, "%s", buff);
    }

    if (0 == GetIniKeyString("SYSTEM", "update_version", VERSION_FILE_PATH, buff))
    {
        if (ver->update_version == NULL)
            asprintf(&ver->update_version, "%s", buff);
    }
}

version_t kdk_system_get_version_detaile()
{
    version_t version = {0};

    if (access(SYSTEM_VERSION_PATH, F_OK) == 0)
    {
        // get current version
        getCurrentVersion(&version);
    }
    else
    {
        // get default version
        getDefaultVersion(&version);
    }

    if (((version.os_version == NULL) || (strstr(version.os_version, "null") != NULL)) && version.update_version != NULL)
    {
        asprintf(&version.os_version, "%s", version.update_version);
    }
    else if (((version.update_version == NULL) || (strstr(version.update_version, "null") != NULL)) && version.os_version != NULL)
    {
        asprintf(&version.update_version, "%s", version.os_version);
    }
    else if (((version.os_version == NULL) || (strstr(version.os_version, "null") != NULL)) && ((version.update_version == NULL) || (strstr(version.update_version, "null") != NULL)))
    {
        asprintf(&version.os_version, "%s", "");
        asprintf(&version.update_version, "%s", "");
    }

    strstrip(version.os_version, '"');
    strstrip(version.update_version, '"');

    return version;
}

char **kdk_system_get_resolving_power()
{
    Display *disp = NULL;
    XRRScreenResources *screen = NULL;
    XRROutputInfo *info;
    XRRCrtcInfo *crtc_info;
    int iscres;
    int icrtc;
    char tmp[1024] = "\0";

    char **res = NULL;
    char **tmpres = NULL;
    int count = 0;
    char temp[512] = {0};

    disp = XOpenDisplay(0);
    screen = XRRGetScreenResources (disp, DefaultRootWindow(disp));
    for (iscres = screen->noutput; iscres > 0;)
    {
        --iscres;

        info = XRRGetOutputInfo(disp, screen, screen->outputs[iscres]);
        if (info->connection == RR_Connected)
        {
            for (icrtc = info->ncrtc; icrtc > 0;)
            {
                memset(tmp, 0, sizeof(tmp));
                sprintf(tmp, "%s,", info->name);

                --icrtc;

                crtc_info = XRRGetCrtcInfo(disp, screen, screen->crtcs[icrtc]);
                // fprintf(stderr, "==> %dx%d+%dx%d\n", crtc_info->x, crtc_info->y, crtc_info->width, crtc_info->height);
                if (crtc_info->width != 0 || crtc_info->height != 0)
                {
                    sprintf(tmp, "%s%dx%d,", tmp, crtc_info->width, crtc_info->height);

                    for (int m = 0; m < screen->nmode; m++)
                    {
                        XRRModeInfo *mode = &screen->modes[m];
                        if (!strstr(temp, mode->name))
                        {
                            strcat(temp, mode->name);
                            strcat(temp, " ");
                        }
                    }
                    strcat(tmp, temp);

                    tmpres = realloc(res, (count + 2) * sizeof(char *));
                    if (!tmpres)
                    {
                        klog_err("内存申请失败：%s\n", strerror(errno));
                        XRRFreeCrtcInfo(crtc_info);
                        XRRFreeOutputInfo(info);
                        goto err_out;
                    }
                    res = tmpres;
                    res[count] = strdup(tmp);
                    count++;
                }
                XRRFreeCrtcInfo(crtc_info);
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
    if (screen)
        XRRFreeScreenResources(screen);
    if (disp)
        XCloseDisplay(disp);
    return NULL;
out:
    return res;
}

char *kdk_system_get_systemCategory()
{
    char *vername = (char *)malloc(sizeof(char) * 50);
#ifdef __linux__
    FILE *fp = fopen("/etc/LICENSE", "r");
    if (!fp)
    {
        strcpy(vername, "none");
        return vername;
    }

    char *tmp = get_val_from_file(fp, "HW_TYPE");
    if (!tmp)
    {
        fclose(fp);
        strcpy(vername, "none");
        return vername;
    }
    strstrip(tmp, '\n');
    strstrip(tmp, '\"');
    strcpy(vername, tmp);
    free(tmp);
    fclose(fp);
#endif
    return vername;
}

char *kdk_system_get_cloudPlatformType()
{
    char *cloudplat = (char *)malloc(sizeof(char) * 65);
    ASSERT_NOT_NULL(cloudplat, NULL);
#ifdef __linux__
    cloudplat[0] = 0;
    char buf[256] = {0};
    bool res_flag = false;
    if (geteuid() == 0) // root 用户，可以用dmidecode
    {
        FILE *pipeLine = popen("dmidecode -s chassis-manufacturer", "r");
        if (__glibc_likely(pipeLine != NULL))
        {
            fgets(buf, 255 * sizeof(char), pipeLine);
            strstripspace(buf);
            if (strcmp(buf, "Huawei Inc.") == 0) // 华为云
            {
                strcpy(cloudplat, "Huawei Private Cloud");
                res_flag = true;
            }
            pclose(pipeLine);
        }
        if (!res_flag)
        {
            pipeLine = popen("dmidecode -s chassis-asset-tag", "r");
            if (__glibc_likely(pipeLine != NULL))
            {
                fgets(buf, 255 * sizeof(char), pipeLine);
                strstripspace(buf);
                if (strcmp(buf, "HUAWEICLOUD") == 0) // 华为云
                {
                    strcpy(cloudplat, "Huawei Public Cloud");
                }
                pclose(pipeLine);
            }
        }
    }
    else // 普通用户，只能读取文件
    {
        FILE *fp = fopen("/sys/devices/virtual/dmi/id/chassis_vendor", "rt");
        if (__glibc_likely(fp != NULL))
        {
            fgets(buf, 255 * sizeof(char), fp);
            strstripspace(buf);
            if (strcmp(buf, "Huawei Inc.") == 0) // 华为云
            {
                strcpy(cloudplat, "Huawei Private Cloud");
                res_flag = true;
            }
            fclose(fp);
        }
        if (!res_flag)
        {
            fp = fopen("/sys/devices/virtual/dmi/id/chassis_asset_tag", "r");
            if (__glibc_likely(fp != NULL))
            {
                fgets(buf, 255 * sizeof(char), fp);
                strstripspace(buf);
                if (strcmp(buf, "HUAWEICLOUD") == 0) // 华为云
                {
                    strcpy(cloudplat, "Huawei Public Cloud");
                }
                fclose(fp);
            }
        }
    }

    if (strlen(cloudplat) == 0)
        strcpy(cloudplat, "none");
#endif
    return cloudplat;
}

char **kdk_system_get_startup_time()
{
    struct utmp *utmp_buf;
    time_t boot_time, rawtime;
    struct tm *info, *res;
    char *result = (char *)malloc(128 * sizeof(char));
    char *tmpe = (char *)malloc(20 * sizeof(char));
    if (!tmpe || !result)
    {
        goto err_out;
    }
    char **ret = NULL;
    char **tt = realloc(ret, 520 * sizeof(char *));
    if (!tt)
    {
        goto err_out;
    }
    ret = tt;
    int count = 0;

    boot_time = 0;
    time(&rawtime);
    res = localtime(&rawtime);
    int l_year = res->tm_year;
    int l_mon = res->tm_mon;
    int l_day = res->tm_mday;
    utmpname(WTMP_FILE);

    setutent();

    while ((utmp_buf = getutent()) != NULL)
    {
        if (utmp_buf->ut_type == BOOT_TIME)
        {
            boot_time = utmp_buf->ut_time;
            info = gmtime(&boot_time);

            if ((l_year == info->tm_year) && (l_day == info->tm_mday) && (l_mon == info->tm_mon))
            {
                sprintf(tmpe, "%d/%d/%d,%d:%d:%d", info->tm_mon + 1, info->tm_mday,
                        info->tm_year + 1900, info->tm_hour, info->tm_min, info->tm_sec);

                strcpy(result, tmpe);

                ret[count] = malloc(256 * sizeof(char) + 1);
                if (!ret[count])
                {
                    free(tt);
                    goto err_out;
                }
                strcpy(ret[count], result);
                count++;
            }
        }
    }
    ret[count] = NULL;
    free(tmpe);
    free(result);
    endutent();
    return ret;
err_out:
    if (result)
        free(result);
    if (tmpe)
        free(tmpe);
    return NULL;
}

char **kdk_system_get_shutdown_time()
{
    time_t rawtime;
    struct utmp *u;
    struct tm *info, *res;
    char *result = (char *)malloc(128 * sizeof(char));
    char *tmpe = (char *)malloc(20 * sizeof(char));
    if (!tmpe || !result)
    {
        goto err_out;
    }
    char **ret = NULL;
    char **tt = realloc(ret, 520 * sizeof(char *));
    if (!tt)
    {
        goto err_out;
    }
    ret = tt;
    int count = 0;

    time(&rawtime);
    res = localtime(&rawtime);
    int l_year = res->tm_year;
    int l_mon = res->tm_mon;
    int l_day = res->tm_mday;

    utmpname("/var/log/wtmp");
    while ((u = getutent()))
    {
        if ((strncmp(u->ut_user, "shutdown", 8) == 0))
        {
#if __WORDSIZE_TIME64_COMPAT32
            time_t tmp = (time_t)(u->ut_tv.tv_sec);
            info = gmtime(&tmp);
#else
                info = gmtime((time_t *)(&u->ut_tv.tv_sec));
#endif
            if ((l_year == info->tm_year) && (l_day == info->tm_mday) && (l_mon == info->tm_mon))
            {
                sprintf(tmpe, "%d/%d/%d,%d:%d:%d", info->tm_mon + 1, info->tm_mday,
                        info->tm_year + 1900, info->tm_hour + 8, info->tm_min, info->tm_sec);

                strcpy(result, tmpe);
                ret[count] = malloc(256 * sizeof(char) + 1);
                if (!ret[count])
                {
                    free(tt);
                    goto err_out;
                }
                strcpy(ret[count], result);
                count++;
            }
        }
    }
    ret[count] = NULL;
    free(tmpe);
    free(result);
    endutent();
    return ret;

err_out:
    if (result)
        free(result);
    if (tmpe)
        free(tmpe);
    return NULL;
}

char *kdk_get_host_vendor()
{
    FILE *fp = NULL;
    char line[64] = "\0";
    char *vendor = (char *)malloc(sizeof(char) * 64);
    fp = fopen("/sys/class/dmi/id/sys_vendor", "r");
    if (fp == NULL)
    {
        free(vendor);
        return NULL;
    }
    fgets(line, sizeof(line), fp);
    if (line[0] != '\0')
    {
        strcpy(vendor, line);
        strstripspace(vendor);
    }
    return vendor;
}

char *kdk_get_host_product()
{
    FILE *fp = NULL;
    char line[64] = "\0";
    char *product = (char *)malloc(sizeof(char) * 64);
    fp = fopen("/sys/class/dmi/id/product_name", "r");
    if (fp == NULL)
    {
        free(product);
        return NULL;
    }
    fgets(line, sizeof(line), fp);
    if (line[0] != '\0')
    {
        strcpy(product, line);
        strstripspace(product);
    }
    return product;
}

char *_kdk_get_host_serial()
{
    FILE *fp = NULL;
    char line[64] = "\0";
    char *serial = (char *)malloc(sizeof(char) * 64);
    fp = fopen("/sys/class/dmi/id/product_serial", "r");
    if (fp == NULL)
    {
        free(serial);
        return NULL;
    }
    fgets(line, sizeof(line), fp);
    if (line[0] != '\0')
    {
        strcpy(serial, line);
        strstripspace(serial);
    }
    return serial;
}

char *kdk_get_host_serial()
{
    char *ret = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err);
    if (NULL == conn)
    {
        ret = NULL;
    }

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/sysinfo", // object to call on
                                            "com.lingmo.lingmosdk.sysinfo",  // interface to call on
                                            "getHostSerial");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        ret = NULL;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        ret = NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        ret = NULL;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        ret = NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    dbus_connection_close(conn);

    if(ret == NULL)
    {
        ret = _kdk_get_host_serial();
    }

    char *serial = NULL;
    if(ret != NULL)
    {
        serial = (char *)malloc(sizeof(char) * 64);
        if(!serial)
        {
            serial = NULL;
        }
        strcpy(serial, ret);
    }

    return serial;
}


char *kdk_system_get_hostName()
{
    char *hostName = NULL;
    FILE *fp = fopen("/etc/hostname", "r");
    if (!fp)
        return NULL;

    hostName = (char *)calloc(512, sizeof(char));
    if (hostName)
        fread(hostName, sizeof(char), 512, fp);
    fclose(fp);
    strstrip(hostName, '\n');
    strstrip(hostName, '\t');
    return hostName;
}

int kdk_system_get_word()
{
#if __WORDSIZE == 64
    return 64;
#else
        return 32;
#endif
}

char *kdk_system_get_buildTime()
{
    char *buildTime = NULL;
#ifdef __linux__
    FILE *fp = fopen("/etc/lingmo-build", "r");
    if (!fp)
        return NULL;

    buildTime = get_val_from_file(fp, "Build");
    if (!buildTime)
        return NULL;
    strstrip(buildTime, '\n');
    strstrip(buildTime, '\"');

    fclose(fp);
#endif
    return buildTime;
}

inline void kdk_resolving_freeall(char **list)
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

#define NONRET __attribute__((noreturn))
void NONRET
die(char *msg, ...)
{
  va_list args;

  char errinfo[1024] = "\0";

  va_start(args, msg);
  klog_err("%s: ", "setpci");
  vsprintf(errinfo, msg, args);
  klog_err("%s\n", errinfo);
  exit(1);
}

static void
check_conf_range(struct device *d, unsigned int pos, unsigned int len)
{
  while (len)
    if (!d->present[pos])
      die("Internal bug: Accessing non-read configuration byte at position %x", pos);
    else
      pos++, len--;
}

unsigned char get_conf_byte(struct device *d, unsigned int pos)
{
  check_conf_range(d, pos, 1);
  return d->config[pos];
}

u16 get_conf_word(struct device *d, unsigned int pos)
{
  check_conf_range(d, pos, 2);
  return d->config[pos] | (d->config[pos + 1] << 8);
}

void get_subid(struct device *d, u16 *subvp, u16 *subdp)
{
  u8 htype = get_conf_byte(d, PCI_HEADER_TYPE) & 0x7f;

  if (htype == PCI_HEADER_TYPE_NORMAL)
  {
    *subvp = get_conf_word(d, PCI_SUBSYSTEM_VENDOR_ID);
    *subdp = get_conf_word(d, PCI_SUBSYSTEM_ID);
  }
  else if (htype == PCI_HEADER_TYPE_CARDBUS && d->config_cached >= 128)
  {
    *subvp = get_conf_word(d, PCI_CB_SUBSYSTEM_VENDOR_ID);
    *subdp = get_conf_word(d, PCI_CB_SUBSYSTEM_ID);
  }
  else
    *subvp = *subdp = 0xffff;
}

void *
xmalloc(size_t howmuch)
{
  void *p = malloc(howmuch);
  if (!p)
    die("Unable to allocate %d bytes of memory", (int)howmuch);
  return p;
}

void *
xrealloc(void *ptr, size_t howmuch)
{
  void *p = realloc(ptr, howmuch);
  if (!p)
    die("Unable to allocate %d bytes of memory", (int)howmuch);
  return p;
}

char *
xstrdup(const char *str)
{
  int len = strlen(str) + 1;
  char *copy = xmalloc(len);
  memcpy(copy, str, len);
  return copy;
}

int config_fetch(struct device *d, unsigned int pos, unsigned int len)
{
    unsigned int end = pos + len;
    int result;

    while (pos < d->config_bufsize && len && d->present[pos])
        pos++, len--;
    while (pos + len <= d->config_bufsize && len && d->present[pos + len - 1])
        len--;
    if (!len)
        return 1;

    if (end > d->config_bufsize)
    {
        int orig_size = d->config_bufsize;
        while (end > d->config_bufsize)
            d->config_bufsize *= 2;
        d->config = xrealloc(d->config, d->config_bufsize);
        d->present = xrealloc(d->present, d->config_bufsize);
        memset(d->present + orig_size, 0, d->config_bufsize - orig_size);
    }
    result = pci_read_block(d->dev, pos, d->config + pos, len);
    if (result)
        memset(d->present + pos, 1, len);
    return result;
}

struct device *
scan_device(struct pci_dev *p, struct pci_filter *filter)
{
    struct device *d;
    
    //  if (p->domain && !opt_domains)
    //    opt_domains = 1;
    if (!pci_filter_match(filter, p))
    {
        return NULL;
    }
    d = xmalloc(sizeof(struct device));
    memset(d, 0, sizeof(*d));
    d->dev = p;
    d->config_cached = d->config_bufsize = 64;
    d->config = xmalloc(64);
    d->present = xmalloc(64);
    memset(d->present, 1, 64);

    if (!pci_read_block(p, 0, d->config, 64))
    {
        klog_err("lspci: Unable to read the standard configuration space header of device %04x:%02x:%02x.%d\n",
                 p->domain, p->bus, p->dev, p->func);
        free(d);
        return NULL;
    }
    if ((d->config[PCI_HEADER_TYPE] & 0x7f) == PCI_HEADER_TYPE_CARDBUS)
    {
        /* For cardbus bridges, we need to fetch 64 bytes more to get the
         * full standard header... */
        if (config_fetch(d, 64, 64))
            d->config_cached += 64;
    }

    pci_setup_cache(p, d->config, d->config_cached);
    pci_fill_info(p, PCI_FILL_IDENT | PCI_FILL_CLASS);
    return d;
}

static void
scan_devices(struct pci_access *pacc, struct device **ret, struct pci_filter *filter)
{
    struct device *d, *first_dev = NULL;
    struct pci_dev *p;

    pci_scan_bus(pacc);
    for (p = pacc->devices; p; p = p->next)
    {
        if ((d = scan_device(p, filter)))
        {
            d->next = first_dev;
            first_dev = d;
        }
    }
  *ret = first_dev;
}

static int
compare_them(const void *A, const void *B)
{
  const struct pci_dev *a = (*(const struct device **)A)->dev;
  const struct pci_dev *b = (*(const struct device **)B)->dev;

  if (a->domain < b->domain)
    return -1;
  if (a->domain > b->domain)
    return 1;
  if (a->bus < b->bus)
    return -1;
  if (a->bus > b->bus)
    return 1;
  if (a->dev < b->dev)
    return -1;
  if (a->dev > b->dev)
    return 1;
  if (a->func < b->func)
    return -1;
  if (a->func > b->func)
    return 1;
  return 0;
}

static void
sort_them(struct device **devices)
{
    struct device **index = NULL, **h = NULL, **last_dev = NULL;
    int cnt;
    struct device *d = NULL;

    struct device *first_dev = *devices;

    // 计算个数
    cnt = 0;
    for (d = first_dev; d; d = d->next)
        cnt++;

    // 申请一个数组空间，存放所有元素
    //  h = index = alloca(sizeof(struct device *) * cnt);
    struct device *tmp[cnt];
    h = index = tmp;
    for (d = first_dev; d; d = d->next)
        *h++ = d;

    // 排序
    qsort(index, cnt, sizeof(struct device *), compare_them);

    // 重新组成链表
    last_dev = devices;
    h = index;
    while (cnt--)
    {
        *last_dev = *h;
        last_dev = &(*h)->next;
        h++;
    }
    *last_dev = NULL;
}

#ifdef PCI_OS_LINUX

#include <sys/utsname.h>

#ifdef PCI_USE_LIBKMOD

#include <libkmod.h>

static struct kmod_ctx *kmod_ctx;

static int
show_kernel_init(void)
{
    static int show_kernel_inited = -1;
    if (show_kernel_inited >= 0)
        return show_kernel_inited;

    kmod_ctx = kmod_new(NULL, NULL);
    if (!kmod_ctx)
    {
        klog_err("lspci: Unable to initialize libkmod context\n");
        goto failed;
    }

    int err;
    if ((err = kmod_load_resources(kmod_ctx)) < 0)
    {
        klog_err("lspci: Unable to load libkmod resources: error %d\n", err);
        goto failed;
    }

    show_kernel_inited = 1;
    return 1;

failed:
    show_kernel_inited = 0;
    return 0;
}

void show_kernel_cleanup(void)
{
    if (kmod_ctx)
        kmod_unref(kmod_ctx);
}

static const char *next_module(struct device *d)
{
    static struct kmod_list *klist, *kcurrent;
    static struct kmod_module *kmodule;

    if (kmodule)
    {
        kmod_module_unref(kmodule);
        kmodule = NULL;
    }

    if (!klist)
    {
        pci_fill_info(d->dev, PCI_FILL_MODULE_ALIAS);
        if (!d->dev->module_alias)
            goto out;

        int err = kmod_module_new_from_lookup(kmod_ctx, d->dev->module_alias, &klist);
        if (err < 0)
        {
            fprintf(stderr, "lspci: libkmod lookup failed: error %d\n", err);
            goto out;
        }
        kcurrent = klist;
    }
    else
    {
        kcurrent = kmod_list_next(klist, kcurrent);
    }

    if (kcurrent)
    {
        kmodule = kmod_module_get_module(kcurrent);
        char *module = kmod_module_get_name(kmodule);
        return module;
    }
    kmod_module_unref_list(klist);
    klist = NULL;
out:
    return NULL;
}

#else // PCI_USE_LIBKMOD

struct pcimap_entry
{
  struct pcimap_entry *next;
  unsigned int vendor, device;
  unsigned int subvendor, subdevice;
  unsigned int class, class_mask;
  char module[1];
};

static struct pcimap_entry *pcimap_head;
static int
show_kernel_init(void)
{
    struct pcimap_entry *pcimap_head;
    static int tried_pcimap = 0;
    struct utsname uts;
    char *name, line[1024];
    FILE *f;

    if (tried_pcimap)
        return 1;
    tried_pcimap = 1;

    if (uname(&uts) < 0)
        die("uname() failed: %m");
    name = alloca(64 + strlen(uts.release));
    sprintf(name, "/lib/modules/%s/modules.pcimap", uts.release);
    f = fopen(name, "r");
    if (!f)
        return 1;

    while (fgets(line, sizeof(line), f))
    {
        char *c = strchr(line, '\n');
        struct pcimap_entry *e;

        if (!c)
            die("Unterminated or too long line in %s", name);
        *c = 0;
        if (!line[0] || line[0] == '#')
            continue;

        c = line;
        while (*c && *c != ' ' && *c != '\t')
            c++;
        if (!*c)
            continue; /* FIXME: Emit warnings! */
        *c++ = 0;

        e = xmalloc(sizeof(*e) + strlen(line));
        if (sscanf(c, "%i%i%i%i%i%i",
                   &e->vendor, &e->device,
                   &e->subvendor, &e->subdevice,
                   &e->class, &e->class_mask) != 6)
            continue;
        e->next = pcimap_head;
        pcimap_head = e;
        strcpy(e->module, line);
    }
    fclose(f);

    return 1;
}

static int
match_pcimap(struct device *d, struct pcimap_entry *e)
{
  struct pci_dev *dev = d->dev;
  unsigned int class = get_conf_long(d, PCI_REVISION_ID) >> 8;
  u16 subv, subd;

#define MATCH(x, y) ((y) > 0xffff || (x) == (y))
    get_subid(d, &subv, &subd);
    return MATCH(dev->vendor_id, e->vendor) &&
           MATCH(dev->device_id, e->device) &&
           MATCH(subv, e->subvendor) &&
           MATCH(subd, e->subdevice) &&
           (class & e->class_mask) == e->class;
#undef MATCH
}

static const char *next_module(struct device *d)
{
  static struct pcimap_entry *current;

  if (!current)
    current = pcimap_head;
  else
    current = current->next;

  while (current)
  {
    if (match_pcimap(d, current))
      return current->module;
    current = current->next;
  }

  return NULL;
}

static void show_kernel_cleanup()
{
  struct pcimap_entry *tmp = NULL;
  if (pcimap_head)
  {
    tmp = pcimap_head;
    pcimap_head = pcimap_head->next;
    free(tmp);
  }
}

#endif // PCI_USE_LIBKMOD

static char *
find_driver(struct device *d, char *buf)
{
  struct pci_dev *dev = d->dev;
  char name[1024], *drv, *base = NULL;
  int n;

  if (dev->access->method != PCI_ACCESS_SYS_BUS_PCI)
    return NULL;

  struct pci_param *p;
  for (p = dev->access->params; p; p = p->next)
    if (!strcmp(p->param, "sysfs.path"))
      base = p->value;

  if (!base || !base[0])
    return NULL;

    n = snprintf(name, sizeof(name), "%s/devices/%04x:%02x:%02x.%d/driver",
                 base, dev->domain, dev->bus, dev->dev, dev->func);
    if (n < 0 || n >= (int)sizeof(name))
        die("show_driver: sysfs device name too long, why?");

  n = readlink(name, buf, 1024);
  if (n < 0)
    return NULL;
  if (n >= 1024)
    return "<name-too-long>";
  buf[n] = 0;

  if ((drv = strrchr(buf, '/')))
    return drv + 1;
  else
    return buf;
}

static const char *
next_module_filtered(struct device *d)
{
  static char prev_module[256];
  const char *module;

  while ((module = next_module(d)))
  {
    if (strcmp(module, prev_module))
    {
      strncpy(prev_module, module, sizeof(prev_module));
      prev_module[sizeof(prev_module) - 1] = 0;
      return module;
    }
  }
  prev_module[0] = 0;
  return NULL;
}
#endif // PCI_OS_LINUX

struct KPci *kdk_hw_get_pci_info()
{

    struct KPci *result = NULL, *node = NULL;

    struct pci_access *pacc = NULL;
    struct device *first_dev = NULL;
    struct pci_filter filter; /* Device filter */

    pacc = pci_alloc();
    pacc->error = die;
    pci_filter_init(pacc, &filter);

    pci_init(pacc);
    scan_devices(pacc, &first_dev, &filter);
    sort_them(&first_dev);

    struct device *d;

    for (d = first_dev; d; d = d->next)
    {
        if (!pci_filter_match(&filter, d->dev))
        {
            continue;
        }
        struct pci_dev *p = d->dev;

        node = (struct KPci *)calloc(1, sizeof *node);
        if (!node)
        {
            klog_err("Failed to request memory %02x:%02x.%d", p->bus, p->dev, p->func);
            kdk_hw_free_pci_info(result);
            result = NULL;
            goto out;
        }
        node->next = result;
        result = node;

        sprintf(node->slot_path, "%02x:%02x.%d", p->bus, p->dev, p->func);

        pci_lookup_name(pacc, node->class_name, sizeof(node->class_name),
                                 PCI_LOOKUP_CLASS,
                                 p->device_class);

        pci_lookup_name(pacc, node->product_name, sizeof(node->product_name),
                                 PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE,
                                 p->vendor_id, p->device_id);

        node->rev = get_conf_byte(d, PCI_REVISION_ID);

        // Subsystem Name
        u16 subsys_v, subsys_d;

        //    pci_fill_info(p, PCI_FILL_LABEL);
        //    if (p->label)
        //      printf("\tDeviceName: %s", p->label);

        get_subid(d, &subsys_v, &subsys_d);
        if (subsys_v && subsys_v != 0xffff){
            char* pp = pci_lookup_name(pacc, node->ss_name, sizeof(node->ss_name),
                                     PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE | PCI_LOOKUP_SUBSYSTEM,
                                     p->vendor_id, p->device_id, subsys_v, subsys_d);
        }
        else
            strcpy(node->ss_name, "None");

        // 获取dirver
        const char *driver, *module;

        char buf[1024];
        strcpy(node->driver_use, (driver = find_driver(d, buf)) ? driver : "None");

        // 获取Kernel Modules
        if (!show_kernel_init()) // Kernel Init失败,modules返回空，进入下一轮循环
        continue;

        while ((module = next_module_filtered(d)))
        {
            // node->module_count++;

            char **tmp = NULL;
            tmp = realloc(node->modules, (node->module_count + 1) * sizeof(char *));
            if (!tmp)
            {
                klog_err("Failed to request memory modules");
                kdk_hw_free_pci_info(result);
                result = NULL;
                show_kernel_cleanup();
                goto out;
            }
            node->module_count++;
            node->modules = tmp;
            node->modules[node->module_count - 1] = (char *)malloc(strlen(module) + 1);
            if (!node->modules[node->module_count - 1])
            {
                klog_err("Failed to request memory %d module", node->module_count);
                kdk_hw_free_pci_info(result);
                result = NULL;
                goto out;
            }

            strcpy(node->modules[node->module_count - 1], module);
            // node->modules[node->module_count] = NULL;
        }
    }
out:
    while (first_dev)
    {
        d = first_dev;
        first_dev = first_dev->next;
        free(d->config);
        free(d->present);
        free(d);
    }
    show_kernel_cleanup();
    pci_cleanup(pacc);
    return result;
}

void kdk_hw_free_pci_info(struct KPci *info)
{
  struct KPci *tmp = NULL;
  while (info)
  {
    tmp = info;
    info = info->next;
    for (int i = 0; i < tmp->module_count; i++)
      free(tmp->modules[i]);
    free(tmp->modules);
    free(tmp);
  }
}

char *kdk_system_get_appScene()
{
    char *scene = (char *)malloc(sizeof(char) * 50);
    ASSERT_NOT_NULL(scene, NULL);
#ifdef __linux__
    FILE *fp = fopen("/etc/LICENSE", "r");
    if (!fp)
    {
        strcpy(scene, "none");
        return scene;
    }

    char *tmp = get_val_from_file(fp, "APP_SCENE");
    if (!tmp)
    {
        fclose(fp);
        strcpy(scene, "none");
        return scene;
    }
    strstrip(tmp, '\n');
    strstrip(tmp, '\"');
    strcpy(scene, tmp);
    free(tmp);
    fclose(fp);
#endif
    return scene;
}

static bool kdk_system_check_has_user(char *username)
{
    if (!username)
    {
        return false;
    }

    char path[256] = "\0";
    struct stat buff;
    sprintf(path, "/home/%s", username);
    if (access(path, F_OK) != -1)
    {
        int result = stat(path, &buff);
        if (__S_IFDIR & buff.st_mode)
        {
            return true;
        }
    }
    return false;
}

bool kdk_system_user_logout_status(char *name)
{
    bool status = true;
    if (kdk_system_check_has_user(name) == false)
    {
        return true;
    }

    DBusConnection *connt = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusError error;
    DBusMessageIter iter, args;

    // 初始化DBus连接
    dbus_error_init(&error);
    connt = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connt == NULL)
    {
        klog_err("DBus connection error: %s\n", error.message);
        // goto out;
    }

    // 创建DBus方法调用消息
    message = dbus_message_new_method_call("org.freedesktop.login1",
                                           "/org/freedesktop/login1",
                                           "org.freedesktop.login1.Manager",
                                           "ListUsers");
    if (message == NULL)
    {
        klog_err("DBus message allocation failed\n");
        goto out;
    }

    // 发送DBus消息并等待回复
    reply = dbus_connection_send_with_reply_and_block(connt, message, -1, &error);
    if (reply == NULL)
    {
        klog_err("DBus reply error: %s\n", error.message);
        goto out;
    }

    // 释放message
    dbus_message_unref(message);

    // 解析回复消息

    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_STRUCT)
    {
        klog_err("DBus reply parsing failed\n");
        goto out;
    }

    // 释放reply
    dbus_message_unref(reply);

    // 获取数组长度  获取的长度不对 弃用
    dbus_message_iter_recurse(&iter, &iter);

    // 解析数组中的每个结构体元素
    while (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_INVALID)
    {
        DBusMessageIter struct_iter;

        char *iter_name = NULL;
        dbus_message_iter_recurse(&iter, &struct_iter);

        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_get_basic(&struct_iter, &iter_name); // 获取name
        if (strcmp(iter_name, name) == 0)
        {
            status = false;
        }

        dbus_message_iter_next(&struct_iter);

        dbus_message_iter_next(&iter);
    }

out:
    dbus_error_free(&error);
    if (connt)
        dbus_connection_unref(connt);
    return status;
}

static pthread_t thread = 0;
static volatile int running = 0;
static char *curr_user = NULL;
CallBack _callback = NULL;

static DBusHandlerResult signal_handler(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    DBusError err;
    dbus_error_init(&err);

    if (dbus_message_is_signal(message, "org.freedesktop.DBus.Properties", "PropertiesChanged"))
    {
        // const char *interface_name;
        DBusMessageIter array_iter, dict_iter, struct_iter, value_iter;
        // const char *property_name;
        const char *session_id, *session_path;

        // 迭代到interfacename
        DBusMessageIter iter;
        dbus_message_iter_init(message, &iter);
        // dbus_message_iter_get_basic(&iter, &interface_name);

        // 迭代到propertiename
        dbus_message_iter_next(&iter);
        dbus_message_iter_recurse(&iter, &array_iter);
        dbus_message_iter_recurse(&array_iter, &dict_iter);
        // dbus_message_iter_get_basic(&dict_iter, &property_name);

        // 迭代到propertievalue
        dbus_message_iter_next(&dict_iter);
        dbus_message_iter_recurse(&dict_iter, &struct_iter);
        dbus_message_iter_recurse(&struct_iter, &value_iter);
        dbus_message_iter_get_basic(&value_iter, &session_id);
        dbus_message_iter_next(&value_iter);
        dbus_message_iter_get_basic(&value_iter, &session_path);

        // 切换tty会出发信号，切换到tty1 tty2等终端，但这些没有用户登录时，不相应
        if (0 == strcmp("/", session_path))
        {
            // free(curr_user);
            // curr_user = NULL;
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        // 创建DBus方法调用消息
        DBusMessage *msg = dbus_message_new_method_call("org.freedesktop.login1",
                                                        session_path,
                                                        "org.freedesktop.DBus.Properties",
                                                        "Get");
        char *interface = "org.freedesktop.login1.Session";
        char *name = "Name";
        dbus_message_append_args(msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID);

        // 发送消息并等待响应
        DBusMessage *reply;
        reply = dbus_connection_send_with_reply_and_block(connection, msg, -1, &err);
        if (dbus_error_is_set(&err))
        {
            klog_err("Error sending message: %s\n", err.message);
            dbus_error_free(&err);
            return DBUS_HANDLER_RESULT_HANDLED;
        }

        // 处理响应
        const char *response;
        DBusMessageIter reply_iter;
        dbus_message_iter_init(reply, &reply_iter);
        // 移动到variant参数的内部
        dbus_message_iter_recurse(&reply_iter, &reply_iter);
        dbus_message_iter_get_basic(&reply_iter, &response);

        // 释放消息和连接
        dbus_message_unref(msg);
        dbus_message_unref(reply);

        if (0 != strcmp(response, "lightdm"))
        {
            // struct passwd *pwd = getpwnam(response);
            _callback(curr_user, response, user_data);
            free(curr_user);
            curr_user = strdup(response);
        }
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

void *dbus_listener_thread(void *data)
{
    DBusConnection *connection;
    DBusError error;

    // 初始化DBus错误结构
    dbus_error_init(&error);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        fprintf(stderr, "Connection Error (%s)\n", error.message);
        dbus_error_free(&error);
        pthread_exit(NULL);
    }
    if (connection == NULL)
    {
        fprintf(stderr, "Connection Null\n");
        pthread_exit(NULL);
    }

    // 设置消息处理函数
    dbus_bus_add_match(connection,
                       "type='signal',path='/org/freedesktop/login1/seat/seat0',interface='org.freedesktop.DBus.Properties',member='PropertiesChanged'",
                       &error);
    dbus_connection_add_filter(connection, signal_handler, NULL, NULL);

    // 循环监听DBus消息
    while (true)
    {
        if (running)
            dbus_connection_read_write_dispatch(connection, 20);
        else
        {
            dbus_connection_unref(connection);
            pthread_exit(NULL);
        }
    }

    return NULL;
}

int kdk_system_register_switch_user_handle(CallBack call_back, void *user_data)
{
    DBusConnection *connection;
    DBusError err;

    // 初始化DBus错误结构
    dbus_error_init(&err);

    // 建立与DBus系统总线的连接
    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
        return 1;
    }
    if (connection == NULL)
    {
        fprintf(stderr, "Connection Null\n");
        return 1;
    }

    DBusMessage *msg = dbus_message_new_method_call("org.freedesktop.login1",
                                                    "/org/freedesktop/login1",
                                                    "org.freedesktop.login1.Manager",
                                                    "GetSeat");
    const char *arg = "seat1";
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &arg, DBUS_TYPE_INVALID);

    DBusMessage *reply;
    reply = dbus_connection_send_with_reply_and_block(connection, msg, -1, &err);
    if (dbus_error_is_set(&err))
    {
        dbus_error_free(&err);
    }
    else
        return -1;

    dbus_message_unref(msg);
    dbus_connection_unref(connection);

    char *tmp = kdk_system_get_eUser();
    curr_user = strdup(tmp);
    running = 1;
    _callback = call_back;
    pthread_create(&thread, NULL, dbus_listener_thread, user_data);
    return 0;
}

void kdk_system_unregister_switch_user_handle()
{
    if (NULL != _callback)
    {
        running = 0;
        _callback = NULL;
        pthread_join(thread, NULL);
    }
}

char* kdk_system_get_startup_takeTime()
{
    char str[256] = {0};
    char *time = NULL;
    FILE *pipeLine = popen("systemd-analyze", "r");  // 建立流管道
    if (!pipeLine)
    {
        return NULL;
    }
    
    while (fgets(str, 256 * sizeof(char), pipeLine))
    {
        if(strstr(str, "reached after"))
        {
            time = (char*)malloc(sizeof(char) * 256);
            if(!time)
            {
                pclose(pipeLine);
                return NULL;
            }
            sscanf(str, "%*s %*s %*s %s %*s %*s", time);
        }
    }
    pclose(pipeLine);
    return time;
}

unsigned int kdk_system_get_file_descriptor()
{
    FILE *fp = NULL;
    unsigned int file_nr = 0;

    errno = 0;
    if ((fp = fopen("/proc/sys/fs/file-nr", "r"))) {
        char buf[128] = {0};
        if (fgets(buf, sizeof(buf), fp)) {
            int nm = sscanf(buf, "%u", &file_nr);
            if (nm == 1)
            {
                return file_nr;
            }
        }
    }
    fclose(fp);
    return 0;
}

unsigned int kdk_system_get_process_nums()
{
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    // 打开/proc目录
    dir = opendir("/proc");
    if (dir == NULL) {
        // perror("无法打开/proc目录");
        return 1;
    }

    // 遍历目录并计算进程总数
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            // 判断目录名是否为数字（代表进程）
            char *name = entry->d_name;
            int is_process = 1;
            for (int i = 0; name[i] != '\0'; i++) {
                if (!isdigit(name[i])) {
                    is_process = 0;
                    break;
                }
            }

            if (is_process) {
                count++;
            }
        }
    }

    closedir(dir);

    return count;
}

unsigned int kdk_system_get_thread_nums()
{
    DIR *dir;
    struct dirent *entry;
    char path[128] = {0};
    struct stat fileStat;
    int threadCount = 0;
    char buffer[1024] = {0};

    // 打开/proc目录
    dir = opendir("/proc");
    if (dir == NULL) {
        perror("无法打开/proc目录");
        return 1;
    }

    // 遍历目录并计算进程总数
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            // 判断目录名是否为数字（代表进程）
            char *name = entry->d_name;
            int is_process = 1;
            for (int i = 0; name[i] != '\0'; i++) {
                if (!isdigit(name[i])) {
                    is_process = 0;
                    break;
                }
            }

            if (is_process) {
                // count++;
                sprintf(path, "/proc/%s/status", entry->d_name);
                char canonical_filename[4096] = {"\0"};
                FILE *fp = NULL;
                if (!realpath(path, canonical_filename) || !verify_file(canonical_filename))
                {
                    closedir(dir);
                    return -1;
                }
                fp = fopen(canonical_filename, "r");
                if(!fp)
                {
                    closedir(dir);
                    return -1;
                }
                char *threads = get_val_from_file(fp, "Threads");
                strstripspace(threads);
                threadCount += atoi(threads);
                fclose(fp);
                free(threads);
            }
        }
    }

    closedir(dir);

    return threadCount;
}

#define LOAD_AVG_INIT {0.0, 0.0, 0.0}
kdk_loadavg kdk_system_get_loadavg()
{
    struct sysinfo info;
    kdk_loadavg loadAvg = LOAD_AVG_INIT;
    int rc = sysinfo(&info);
    if (!rc) {
        loadAvg.loadavg_1m = info.loads[0];
        loadAvg.loadavg_5m = info.loads[1];
        loadAvg.loadavg_15m = info.loads[2];
    }
    return loadAvg;
}
#undef LOAD_AVG_INIT
