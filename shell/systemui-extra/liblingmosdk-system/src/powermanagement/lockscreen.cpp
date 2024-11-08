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

#include <iostream>

#include <dbus-1.0/dbus/dbus.h>
#include <libkylog.h>

#include "lockscreen.h"

namespace kdk
{
namespace
{
constexpr char dbusServiceName[] = "org.gnome.SessionManager";
constexpr char dbusObjectPath[] = "/org/gnome/SessionManager";
constexpr char dbusInterfaceName[] = "org.gnome.SessionManager";
constexpr char dbusInhibitLockScreenMethod[] = "Inhibit";
constexpr char dbusUnInhibitLockScreenMethod[] = "Uninhibit";
}

LockScreen::LockScreen() = default;

LockScreen::~LockScreen() = default;

uint32_t LockScreen::setInhibitLockScreen(std::string appName , std::string reason)
{
    if (!appName.size() || !reason.size()) {
        return 0;
    }

    DBusConnection *conn;
    DBusError error;

    dbus_error_init(&error);

    conn = dbus_bus_get(DBUS_BUS_SESSION , &error);
    if (dbus_error_is_set(&error)) {
        klog_err("kdk : d-bus connect fail !\n");
        return 0;
    }

    if (conn == NULL) {
        return 0;
    }

    DBusMessage *sendMsg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    sendMsg = dbus_message_new_method_call(dbusServiceName , dbusObjectPath , dbusInterfaceName , dbusInhibitLockScreenMethod);
    const char *tmpAppName = appName.c_str();
    dbus_int32_t topLevel = 0;
    const char *tmpReason = reason.c_str();
    dbus_int32_t flags = 8;
    if (!dbus_message_append_args(sendMsg , DBUS_TYPE_STRING , &tmpAppName , DBUS_TYPE_UINT32 , &topLevel , DBUS_TYPE_STRING , &tmpReason , DBUS_TYPE_UINT32 , &flags , DBUS_TYPE_INVALID)) {
        klog_err("kdk : d-bus append args fail !\n");
        return 0;
    }

    if (!dbus_connection_send_with_reply(conn , sendMsg , &sendMsgPending , -1)) {
        klog_err("kdk : d-bus send message fail !\n");
        return 0;
    }

    if (sendMsgPending == NULL) {
        klog_err("kdk : d-bus pending message is NULL !\n");
        return 0;
    }

    dbus_connection_flush(conn);

    if (sendMsg) {
        dbus_message_unref(sendMsg);
    }

    dbus_pending_call_block(sendMsgPending);
    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);
    if (replyMsg == NULL) {
        klog_err("kdk : get reply message fail !\n");
        return 0;
    }

    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }

    DBusMessageIter args;
    uint32_t ret = 0;

    if (!dbus_message_iter_init(replyMsg , &args)) {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        return 0;
    } else {
        dbus_message_iter_get_basic(&args , &ret);
    }

    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }

    if (ret) {
        return ret;
    }

    return 0;
}

bool LockScreen::unInhibitLockScreen(uint32_t flag)
{
    if (!flag) {
        return false;
    }

    DBusConnection *conn;
    DBusError error;

    dbus_error_init(&error);

    conn = dbus_bus_get(DBUS_BUS_SESSION , &error);
    if (dbus_error_is_set(&error)) {
        klog_err("kdk : d-bus connect fail !\n");
        return false;
    }

    if (conn == NULL) {
        return false;
    }

    DBusMessage *sendMsg = NULL;
    DBusPendingCall *sendMsgPending = NULL;

    sendMsg = dbus_message_new_method_call(dbusServiceName , dbusObjectPath , dbusInterfaceName , dbusUnInhibitLockScreenMethod);
    if (!dbus_message_append_args(sendMsg , DBUS_TYPE_UINT32 , &flag , DBUS_TYPE_INVALID)) {
        klog_err("kdk : d-bus append args fail !\n");
        return false;
    }

    if (!dbus_connection_send_with_reply(conn , sendMsg , &sendMsgPending , -1)) {
        std::cout << "kdk : d-bus send message fail" << std::endl;
        klog_err("kdk : d-bus send message fail !\n");
        return false;
    }

    if (sendMsgPending == NULL) {
        klog_err("kdk : d-bus pending message is NULL !\n");
        return false;
    }

    dbus_connection_flush(conn);

    if (sendMsg) {
        dbus_message_unref(sendMsg);
    }

    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }

    return true;
}

}
