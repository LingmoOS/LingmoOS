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

#include "libkyboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dbus-1.0/dbus/dbus.h>

const char *kdk_board_get_name()
{
    char *name = (char *)calloc(64,sizeof(char));
    FILE *fd = fopen("/sys/class/dmi/id/board_name","r");
    if(fd)
    {
        fgets(name,64,fd);
        fclose(fd);
    }
    return name;
}

const char *kdk_board_get_date()
{
    char *date = (char *)calloc(64,sizeof(char));
    FILE *fd = fopen("/sys/class/dmi/id/bios_date","r");
    if(fd)
    {
        fgets(date,64,fd);
        fclose(fd);
    }
    return date;
}

const char *_kdk_board_get_serial()
{
    char *serial = (char *)calloc(64,sizeof(char));
    FILE *fd = fopen("/sys/class/dmi/id/board_serial","r");
    if(fd)
    {
        fgets(serial,64,fd);
        fclose(fd);
    }
    return serial;
}

const char *kdk_board_get_serial()
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
                                            "/com/lingmo/lingmosdk/mainboard", // object to call on
                                            "com.lingmo.lingmosdk.mainboard",  // interface to call on
                                            "getMainboardSerial");               // method name
    if (!info_msg)
    { // -1 is default timeout
        ret = NULL;
    }

    // if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &name, DBUS_TYPE_STRING, &fullName, DBUS_TYPE_INT32, &accountType, DBUS_TYPE_INVALID))
    // {
    //     klog_err("kdk : d-bus append args fail !\n");
    //     goto err_out;
    // }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        ret = NULL;
    }

    if (sendMsgPending == NULL)
    {
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
        ret = _kdk_board_get_serial();
    }

    char *serial = NULL;
    if(ret != NULL)
    {
        serial = (char *)malloc(sizeof(char) * 512);
        if(!serial)
        {
            serial = NULL;
        }
        strcpy(serial, ret);
    }

    return serial;
}

const char *kdk_board_get_vendor()
{
    char *vendor = (char *)calloc(64,sizeof(char));
    FILE *fd = fopen("/sys/class/dmi/id/board_vendor","r");
    if(fd)
    {
        fgets(vendor,64,fd);
        fclose(fd);
    }
    return vendor;
}

void kdk_board_free(char* info)
{
    free(info);
}
