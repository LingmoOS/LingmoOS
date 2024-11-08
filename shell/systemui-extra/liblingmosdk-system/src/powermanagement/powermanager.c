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

#include <string.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h> /* for glib main loop */

#include <semaphore.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>

#include <gio/gio.h>
#include <gtk/gtk.h>

#include <lingmosdk/lingmosdk-base/libkylog.h>

pthread_mutex_t lock;
u_int8_t g_Flag; // 控制启用常驻定时器还是临时定时器

u_int8_t g_Quit; //退出信号
sem_t g_Wait;

u_int8_t g_StatusChanged; // 发生了文件变更

static void *printStatus(DBusConnection *ptr, char *buf)
{
	DBusConnection *conn = ptr;
	DBusMessage *msg;
	DBusMessageIter args;
	dbus_uint32_t serial = 0;
	klog_info("%s\n", buf);

	klog_info("g_StatusChanged = %d\n", g_StatusChanged);

	if (g_StatusChanged == 1)
	{

		msg = dbus_message_new_signal("/com/lingmo/lingmosdk/PowerManager",
									"com.lingmo.lingmosdk.PowerManagerInterface",
									"PowerStateChangeSignal");
		if (NULL == msg)
		{
			klog_info( "Message Null\n");
		}
		// append arguments onto signal
		dbus_message_iter_init_append(msg, &args);
		dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &buf);
		dbus_connection_send(conn, msg, &serial);
		dbus_connection_flush(conn);
		dbus_message_unref(msg);
		// free(buf);
	}
	return NULL;
}

// 回调函数，当键值变化时被调用
static void
value_changed(GSettings *settings, const gchar *key, gpointer user_data)
{
	GVariant *value;
	gchar *printed;

	DBusConnection *conn = NULL;
	conn = user_data;
	char buffer[256] = {0};

	value = g_settings_get_value(settings, key);
	printed = g_variant_print(value, TRUE);
	// printf("%s: %s\n", key, printed);
	sprintf(buffer, "%s: %s", key, printed);
	klog_info("%s: %s\n", key, printed);

	g_variant_unref(value);
	g_free(printed);
	g_StatusChanged = 1;

	printStatus(conn, buffer);
	return ;
}

void *actionDateChanged(void *ptr)
{
	klog_info("actionDateChanged\n");
	GSettings *settings;
	DBusConnection *conn = NULL;
	conn = ptr;

    // 创建GSettings对象，指定schema名称和路径
    settings = g_settings_new("org.lingmo.power-manager");
	g_signal_connect(settings, "changed::power-policy-ac", G_CALLBACK(value_changed), conn);
	klog_info("g_signal_connect\n");

	for (;;)
		g_main_context_iteration(NULL, TRUE);
	return NULL;
}

const char *version = "0.1";
GMainLoop *mainloop;

const char *server_introspection_xml =
	DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
	"<node>\n"

	"  <interface name='org.freedesktop.DBus.Introspectable'>\n"
	"    <method name='Introspect'>\n"
	"      <arg name='data' type='s' direction='out' />\n"
	"    </method>\n"
	"  </interface>\n"

	"  <interface name='org.freedesktop.DBus.Properties'>\n"
	"    <method name='Get'>\n"
	"      <arg name='interface' type='s' direction='in' />\n"
	"      <arg name='property'  type='s' direction='in' />\n"
	"      <arg name='value'     type='s' direction='out' />\n"
	"    </method>\n"
	"    <method name='GetAll'>\n"
	"      <arg name='interface'  type='s'     direction='in'/>\n"
	"      <arg name='properties' type='a{sv}' direction='out'/>\n"
	"    </method>\n"
	"  </interface>\n"

	"  <interface name='com.lingmo.lingmosdk.PowerManagerInterface'>\n"
	"    <signal name='PowerStateChangeSignal'>\n"
	"       <arg name='date' type='s' />\n"
	"    </signal>"
	"  </interface>\n"

	"</node>\n";

/*
 * This implements 'Get' method of DBUS_INTERFACE_PROPERTIES so a
 * client can inspect the properties/attributes of 'TestInterface'.
 */
DBusHandlerResult server_get_properties_handler(const char *property, DBusConnection *conn, DBusMessage *reply)
{
	if (!strcmp(property, "Version"))
	{
		dbus_message_append_args(reply,
								 DBUS_TYPE_STRING, &version,
								 DBUS_TYPE_INVALID);
	}
	else
		/* Unknown property */
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (!dbus_connection_send(conn, reply, NULL))
		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	return DBUS_HANDLER_RESULT_HANDLED;
}

/*
 * This implements 'GetAll' method of DBUS_INTERFACE_PROPERTIES. This
 * one seems required by g_dbus_proxy_get_cached_property().
 */
DBusHandlerResult server_get_all_properties_handler(DBusConnection *conn, DBusMessage *reply)
{
	DBusHandlerResult result;
	DBusMessageIter array, dict, iter, variant;
	const char *property = "Version";

	/*
	 * All dbus functions used below might fail due to out of
	 * memory error. If one of them fails, we assume that all
	 * following functions will fail too, including
	 * dbus_connection_send().
	 */
	result = DBUS_HANDLER_RESULT_NEED_MEMORY;

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &array);

	/* Append all properties name/value pairs */
	property = "Version";
	dbus_message_iter_open_container(&array, DBUS_TYPE_DICT_ENTRY, NULL, &dict);
	dbus_message_iter_append_basic(&dict, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(&dict, DBUS_TYPE_VARIANT, "s", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &version);
	dbus_message_iter_close_container(&dict, &variant);
	dbus_message_iter_close_container(&array, &dict);

	dbus_message_iter_close_container(&iter, &array);

	if (dbus_connection_send(conn, reply, NULL))
		result = DBUS_HANDLER_RESULT_HANDLED;
	return result;
}

/*
 * This function implements the 'TestInterface' interface for the
 * 'Server' DBus object.
 *
 * It also implements 'Introspect' method of
 * 'org.freedesktop.DBus.Introspectable' interface which returns the
 * XML string describing the interfaces, methods, and signals
 * implemented by 'Server' object. This also can be used by tools such
 * as d-feet(1) and can be queried by:
 *
 * $ gdbus introspect --session --dest org.example.TestServer --object-path /org/example/TestObject
 */
DBusHandlerResult server_message_handler(DBusConnection *conn, DBusMessage *message, void *data)
{
	// DBusMessage *msg;
	// DBusMessageIter args;
	DBusError err;
	// dbus_uint32_t serial = 0;
	// char *time_now = "test";

	DBusHandlerResult result;
	DBusMessage *reply = NULL;
	bool quit = false;

	fprintf(stderr, "Got D-Bus request: %s.%s on %s\n",
			dbus_message_get_interface(message),
			dbus_message_get_member(message),
			dbus_message_get_path(message));

	/*
	 * Does not allocate any memory; the error only needs to be
	 * freed if it is set at some point.
	 */
	dbus_error_init(&err);

	if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
	{

		if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

		dbus_message_append_args(reply,
								 DBUS_TYPE_STRING, &server_introspection_xml,
								 DBUS_TYPE_INVALID);
	}
	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get"))
	{
		const char *interface, *property;

		if (!dbus_message_get_args(message, &err,
								   DBUS_TYPE_STRING, &interface,
								   DBUS_TYPE_STRING, &property,
								   DBUS_TYPE_INVALID))
			goto fail;

		if (!(reply = dbus_message_new_method_return(message)))
			goto fail;
		result = server_get_properties_handler(property, conn, reply);
		dbus_message_unref(reply);
		return result;
	}
	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll"))
	{

		if (!(reply = dbus_message_new_method_return(message)))
			goto fail;

		result = server_get_all_properties_handler(conn, reply);
		dbus_message_unref(reply);
		return result;
	}
	else
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

fail:
	if (dbus_error_is_set(&err))
	{
		if (reply)
			dbus_message_unref(reply);
		reply = dbus_message_new_error(message, err.name, err.message);
		dbus_error_free(&err);
	}

	/*
	 * In any cases we should have allocated a reply otherwise it
	 * means that we failed to allocate one.
	 */
	if (!reply)
		return DBUS_HANDLER_RESULT_NEED_MEMORY;

	/* Send the reply which might be an error one too. */
	result = DBUS_HANDLER_RESULT_HANDLED;
	if (!dbus_connection_send(conn, reply, NULL))
		result = DBUS_HANDLER_RESULT_NEED_MEMORY;
	dbus_message_unref(reply);

	if (quit)
	{
		fprintf(stderr, "Server exiting...\n");
		g_main_loop_quit(mainloop);
	}
	return result;
}

const DBusObjectPathVTable server_vtable = {
	.message_function = server_message_handler};

int main()
{
	DBusConnection *conn;
	DBusError err;
	int rv;

	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (!conn)
	{
		fprintf(stderr, "Failed to get a system DBus connection: %s\n", err.message);
		goto fail;
	}

	rv = dbus_bus_request_name(conn, "com.lingmo.lingmosdk.PowerManagerServer", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (rv != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
		fprintf(stderr, "Failed to request name on bus: %s\n", err.message);
		goto fail;
	}

	if (!dbus_connection_register_object_path(conn, "/com/lingmo/lingmosdk/PowerManager", &server_vtable, NULL))
	{
		fprintf(stderr, "Failed to register a object path for 'TestObject'\n");
		goto fail;
	}

	sem_init(&g_Wait, 0, 1);

	pthread_mutex_init(&lock, NULL);
	pthread_attr_t attr;
	pthread_t tid;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&tid, &attr, actionDateChanged, conn);

	mainloop = g_main_loop_new(NULL, false);
	/* Set up the DBus connection to work in a GLib event loop */
	dbus_connection_setup_with_g_main(conn, NULL);
	/* Start the glib event loop */
	g_main_loop_run(mainloop);

	return EXIT_SUCCESS;
fail:
	dbus_error_free(&err);
	return EXIT_FAILURE;
}
