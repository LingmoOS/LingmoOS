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
#include <stdlib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h> 

#include "libkylog.h"
#include <glib-2.0/glib.h>

#define SERVICE_NAME "org.freedesktop.NetworkManager"
#define OBJECT_PATH "/org/freedesktop/NetworkManager"
#define INTERFACE_NAME "org.freedesktop.NetworkManager"
#define SIGNAL_NAME "StateChanged"

GMainLoop *mainloop = NULL;
DBusConnection *conn = NULL;

DBusHandlerResult server_message_handler(DBusConnection *conn, DBusMessage *message, void *data)
{
// 	// DBusMessage *msg;
// 	// DBusMessageIter args;
// 	DBusError err;
// 	// dbus_uint32_t serial = 0;
// 	// char *time_now = "test";

	DBusHandlerResult result;
// 	DBusMessage *reply = NULL;
// 	bool quit = false;

// 	fprintf(stderr, "Got D-Bus request: %s.%s on %s\n",
// 			dbus_message_get_interface(message),
// 			dbus_message_get_member(message),
// 			dbus_message_get_path(message));

// 	/*
// 	 * Does not allocate any memory; the error only needs to be
// 	 * freed if it is set at some point.
// 	 */
// 	dbus_error_init(&err);

// 	if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
// 	{

// 		if (!(reply = dbus_message_new_method_return(message)))
// 			goto fail;

// 		dbus_message_append_args(reply,
// 								 DBUS_TYPE_STRING, &server_introspection_xml,
// 								 DBUS_TYPE_INVALID);
// 	}
// 	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get"))
// 	{
// 		const char *interface, *property;

// 		if (!dbus_message_get_args(message, &err,
// 								   DBUS_TYPE_STRING, &interface,
// 								   DBUS_TYPE_STRING, &property,
// 								   DBUS_TYPE_INVALID))
// 			goto fail;

// 		if (!(reply = dbus_message_new_method_return(message)))
// 			goto fail;

// 		result = server_get_properties_handler(property, conn, reply);
// 		dbus_message_unref(reply);
// 		return result;
// 	}
// 	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll"))
// 	{

// 		if (!(reply = dbus_message_new_method_return(message)))
// 			goto fail;

// 		result = server_get_all_properties_handler(conn, reply);
// 		dbus_message_unref(reply);
// 		return result;
// 	}
// 	else
// 		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

// fail:
// 	if (dbus_error_is_set(&err))
// 	{
// 		if (reply)
// 			dbus_message_unref(reply);
// 		reply = dbus_message_new_error(message, err.name, err.message);
// 		dbus_error_free(&err);
// 	}

// 	/*
// 	 * In any cases we should have allocated a reply otherwise it
// 	 * means that we failed to allocate one.
// 	 */
// 	if (!reply)
// 		return DBUS_HANDLER_RESULT_NEED_MEMORY;

// 	/* Send the reply which might be an error one too. */
// 	result = DBUS_HANDLER_RESULT_HANDLED;
// 	if (!dbus_connection_send(conn, reply, NULL))
// 		result = DBUS_HANDLER_RESULT_NEED_MEMORY;
// 	dbus_message_unref(reply);

// 	if (quit)
// 	{
// 		fprintf(stderr, "Server exiting...\n");
// 		g_main_loop_quit(mainloop);
// 	}
	return result;
}

// 发送自定义信号
void send_custom_signal(unsigned int state) {
    DBusMessage *message;
    DBusMessageIter args;

    message = dbus_message_new_signal(OBJECT_PATH, INTERFACE_NAME, SIGNAL_NAME);
    if (!message) {
        fprintf(stderr, "Error creating signal message\n");
        return;
    }

    dbus_message_iter_init_append(message, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &state)) {
        fprintf(stderr, "Error appending argument to signal message\n");
        dbus_message_unref(message);
        return;
    }

    dbus_connection_send(conn, message, NULL);
    dbus_message_unref(message);
}

// 处理接收到的信号
DBusHandlerResult signal_handler(DBusConnection *connection, DBusMessage *message, void *user_data) {
    DBusError error;
    unsigned int state = 0;

    dbus_error_init(&error);
    if (dbus_message_get_args(message, &error, DBUS_TYPE_UINT32, &state, DBUS_TYPE_INVALID)) {
        printf("Received StateChanged signal with state: %u\n", state);
        send_custom_signal(state);
    } else {
        fprintf(stderr, "Error reading StateChanged signal: %s\n", error.message);
        dbus_error_free(&error);
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

const DBusObjectPathVTable server_vtable = {
	.message_function = server_message_handler};

int main()
{
    DBusError err;
    int rv;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (!conn)
    {
        fprintf(stderr, "Failed to get a system DBus connection: %s\n", err.message);
        goto fail;
    }

    rv = dbus_bus_request_name(conn, "com.lingmo.lingmosdk.Net", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
    if (rv != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
        fprintf(stderr, "Failed to request name on bus: %s\n", err.message);
        goto fail;
    }

    if (!dbus_connection_register_object_path(conn, "/com/lingmo/lingmosdk/Net", &server_vtable, NULL))
    {
        fprintf(stderr, "Failed to register a object path for 'TestObject'\n");
        goto fail;
    }

    mainloop = g_main_loop_new(NULL, 0);

    g_main_loop_run(mainloop);

    return EXIT_SUCCESS;
fail:
    dbus_error_free(&err);
    return EXIT_FAILURE;
}