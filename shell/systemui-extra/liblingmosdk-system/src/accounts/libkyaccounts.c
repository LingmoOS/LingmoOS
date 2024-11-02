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

#include "libkyaccounts.h"
#include <dbus-1.0/dbus/dbus.h>
#include <libkylog.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include <polkit/polkit.h>
#include <syslog.h>

char *kdk_system_create_user(char *name, char *fullName, int accountType)
{
    if (kdk_system_check_has_user(name) == 1)
    {
        klog_err("用户已存在 !\n");
        return NULL;
    }

    char *userpath = (char *)malloc(sizeof(char) * 256);
    if (!userpath)
    {
        return NULL;
    }
    memset(userpath, 0, 256);

    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        goto err_out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.freedesktop.Accounts",  // target for the method call
                                            "/org/freedesktop/Accounts", // object to call on
                                            "org.freedesktop.Accounts",  // interface to call on
                                            "CreateUser");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto err_out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        goto err_out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto err_out;
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
        goto err_out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;
    char *ret = NULL;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        goto err_out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    if (!strstr(ret, "/org/freedesktop/Accounts/User"))
    {
        klog_err("用户名命名不规范 !\n");
        goto err_out;
    }
    sprintf(userpath, "/home/%s", name);
    if (!userpath)
    {
        goto err_out;
    }
    return userpath;

err_out:
    free(userpath);
    return NULL;
}

int kdk_authority_check_by_polkit()
{
    PolkitAuthority *pAuthority = NULL;
    GError *error = NULL;
    pid_t pid_of_caller;
    PolkitSubject *subject;
    PolkitCheckAuthorizationFlags flags;
    PolkitAuthorizationResult *polkit_ret = NULL;
    pid_of_caller = getppid();
    // if (pid_of_caller == 1)
    // {
    //     g_printerr("Refusing to render service to dead parents.\n");
    //     return 0;
    // }
    subject = polkit_unix_process_new_for_owner(pid_of_caller,
                                                0,
                                                getuid());
    gchar *actid = "org.lingmo.lingmosdk.chpasswd";
    flags = POLKIT_CHECK_AUTHORIZATION_FLAGS_NONE;
    flags |= POLKIT_CHECK_AUTHORIZATION_FLAGS_ALLOW_USER_INTERACTION;
    pAuthority = polkit_authority_get_sync(NULL, &error);
    polkit_ret = polkit_authority_check_authorization_sync(pAuthority,
                                                           subject,
                                                           actid,
                                                           NULL,
                                                           flags,
                                                           NULL,
                                                           NULL);

    if (polkit_authorization_result_get_is_authorized(polkit_ret))
    {
        syslog(LOG_ERR,"polkit_ret is failed !");
        g_object_unref(polkit_ret);
        g_object_unref(subject);
        return 1;
    };
    g_object_unref(polkit_ret);
    g_object_unref(subject);

    return 0;
}

bool _kdk_system_change_password(char *username, char *password, unsigned int *err_num)
{
    // if (geteuid() != 0) // 非root 用户
    // {
    //     int result = kdk_authority_check_by_polkit();
    //     if (result != 1)
    //     {
    //         klog_err("身份认证失败！错误码：%d\n");
    //         syslog(LOG_ERR,"身份认证失败！");
    //         *err_num = 0x01;
    //         return false;
    //     }
    // }

    syslog(LOG_ERR,"passwd = %s\n", password);

    if (!username && !password)
    {
        *err_num = 0x04;
        return false;
    }

    if (kdk_system_check_has_user(username) != 1)
    {
        klog_err("用户不存在 !\n");
        syslog(LOG_ERR,"用户不存在！");
        *err_num = 0x02;
        return false;
    }

    char command[2048] = "\0";

    int len = strlen(password);
    if (len < 8)
    {
        klog_err("密码少于 8 个字符!\n");
        syslog(LOG_ERR,"密码少于 8 个字符！");
        *err_num = 0x03;
        return false;
    }

    if(len > 1024)
    {
        klog_err("密码大于 1024 个字符!\n");
        syslog(LOG_ERR,"密码大于 1024 个字符！");
        *err_num = 0x03;
        return false;
    }
    
    if(strstr(password, "$") || strstr(password, "\""))
    {
        char str[1024] = "\0";
        strncpy(str, password, sizeof(password));
        int i, j;
        int lenght = strlen(str);
        char tmp[1024] = "\0";

        for (i = 0, j = 0; i < lenght; i++) {
            if (str[i] == '$') {
                tmp[j] = '\\';
                j++;
                tmp[j] = '$';  
            }
            else if(str[i] == '\"') {
                tmp[j] = '\\';
                j++;
                tmp[j] = '\"';
            }
            else {
                tmp[j] = str[i];
            }
            j++;
        }
        tmp[j] = '\0';
        sprintf(command, "echo \"%s:%s\" | chpasswd", username, tmp);
    }
    else{
        sprintf(command, "echo \"%s:%s\" | chpasswd", username, password);
    }
    syslog(LOG_ERR,"command = %s\n", command);
    int ret = system(command);
    if (ret != 0)
    {
        klog_err("无效的密码，密码不规范!\n");
        syslog(LOG_ERR,"无效的密码，密码不规范！");
        *err_num = 0x03;
        return false;
    }
    // *err_num = 0;
    syslog(LOG_ERR,"susscul\n");
    return true;
}

bool kdk_system_change_password(char *username, char *password, unsigned int *err_num)
{
    if (!username && !password)
    {
        *err_num = 0x04;
        return false;
    }

    if (kdk_system_check_has_user(username) != 1)
    {
        klog_err("用户不存在 !\n");
        syslog(LOG_ERR,"用户不存在！");
        *err_num = 0x02;
        return false;
    }

    char command[2048] = "\0";

    int len = strlen(password);
    if (len < 8)
    {
        klog_err("密码少于 8 个字符!\n");
        syslog(LOG_ERR,"密码少于 8 个字符！");
        *err_num = 0x03;
        return false;
    }

    if(len > 1024)
    {
        klog_err("密码大于 1024 个字符!\n");
        syslog(LOG_ERR,"密码大于 1024 个字符！");
        *err_num = 0x03;
        return false;
    }

    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        goto err_out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("com.lingmo.lingmosdk.service",  // target for the method call
                                            "/com/lingmo/lingmosdk/accounts", // object to call on
                                            "com.lingmo.lingmosdk.accounts",  // interface to call on
                                            "changePassword");               // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        goto err_out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &username, DBUS_TYPE_STRING, &password, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        goto err_out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto err_out;
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
        goto err_out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;
    bool *ret = false;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        goto err_out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &ret);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }
    *err_num = 0;
    return ret;
err_out:
    *err_num = 0x03;
    return false;
}

bool kdk_system_check_has_user(char *username)
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

char *kdk_login_get_public_encrypt()
{
    char *pencrypt = NULL;

    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        return NULL;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.lingmo.UniauthBackend",  // target for the method call
                                            "/org/lingmo/UniauthBackend", // object to call on
                                            "org.lingmo.UniauthBackend",  // interface to call on
                                            "getPublicEncrypt");        // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        return NULL;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID)) {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        return NULL;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        return NULL;
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
        return NULL;
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
        return NULL;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &pencrypt);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    if (!pencrypt)
    {
        return NULL;
    }

    return pencrypt;
}

bool kdk_login_send_password(const char *username, unsigned char *password, int length)
{
    if (!username && !password)
    {
        return false;
    }
    unsigned char *passwd = password;

    bool result;

    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        fprintf(stderr, "Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        return false;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;
    DBusMessageIter datatoSendIter, subArrayIter;

    //创建用户s
    info_msg = dbus_message_new_method_call("org.lingmo.UniauthBackend",  // target for the method call
                                            "/org/lingmo/UniauthBackend", // object to call on
                                            "org.lingmo.UniauthBackend",  // interface to call on
                                            "sendPassword");            // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg：dbus_message_new_method_call调用失败\n");
        return false;
    }

    dbus_message_iter_init_append(info_msg, &datatoSendIter);
    dbus_message_iter_append_basic(&datatoSendIter, DBUS_TYPE_STRING, &username);
    dbus_message_iter_open_container(&datatoSendIter,  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING,  &subArrayIter);
    dbus_message_iter_append_fixed_array(&subArrayIter, DBUS_TYPE_BYTE, &passwd, length + 1);
    dbus_message_iter_close_container(&datatoSendIter, &subArrayIter);

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        return false;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        return false;
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
        return false;
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
        return false;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &result);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    return result;
}
