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

#include "m_systime.h"
#include <libkytimer.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/timerfd.h>
#include <errno.h>
// 20020721新增时区变化监听
#include <sys/inotify.h>

#ifndef TFD_TIMER_CANCEL_ON_SET
	#define TFD_TIMER_CANCEL_ON_SET 1 << 1
#endif

pthread_mutex_t lock;
u_int8_t g_Flag;	// 控制启用常驻定时器还是临时定时器

u_int8_t g_Quit;	//退出信号
sem_t g_Wait;

u_int8_t g_TimeChanged;	// 发生了时间变更
u_int8_t g_TimeSync;	// 需要进行对时

void sig_Handler(int sig)
{
	g_Quit = 1;
}

static void *printClock(void *ptr)
{
	DBusConnection *conn = ptr;
	DBusMessage *msg;
	DBusMessageIter args;
	// DBusError err;
	dbus_uint32_t serial = 0;
	struct tm *now;
	time_t current;
	time(&current);
	now = localtime(&current);
	// struct timeval tx;
	// struct timezone tz;
	// gettimeofday(&tx,&tz);
	// zone = tz.tz_minuteswest/60;
	// printf("时差：%d\n",zone);
	// 如果时间发生改变发送TimeChangeSignal信号
	if (g_TimeChanged == 1)
	{
		char *buf = calloc(1, 128);
		sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d", now->tm_year + 1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

		msg = dbus_message_new_signal("/com/lingmo/lingmosdk/Timer",
									  "com.lingmo.lingmosdk.TimeInterface",
									  "TimeChangeSignal");
		if (NULL == msg)
		{
			fprintf(stderr, "Message Null\n");
		}
		// append arguments onto signal
		dbus_message_iter_init_append(msg, &args);
		dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &buf);
		dbus_connection_send(conn, msg, &serial);
		dbus_connection_flush(conn);
		dbus_message_unref(msg);
		free(buf);
	}

	// 非整点情况
	if (now->tm_sec != 0)
	{
		pthread_mutex_lock(&lock);

		// 如果出现了非整点的报时，且
		// 之前处于整分钟报时的情况，说明
		// 出现了唤醒偏差。重新执行一下对时
		if (g_Flag == 1)
		{
			g_Flag = 0;
			g_TimeSync = 1;
		}
		pthread_mutex_unlock(&lock);
		sem_post(&g_Wait);
	}
	// 整点的情况发送TimeSignal
	else
	{
		char *buf = calloc(1, 128);
		sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d", now->tm_year + 1900, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
		// printf("%s\n", buf);

		msg = dbus_message_new_signal("/com/lingmo/lingmosdk/Timer",
									  "com.lingmo.lingmosdk.TimeInterface",
									  "TimeSignal");
		if (NULL == msg)
		{
			fprintf(stderr, "Message Null\n");
		}
		// append arguments onto signal
		dbus_message_iter_init_append(msg, &args);
		dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &buf);
		dbus_connection_send(conn, msg, &serial);
		dbus_connection_flush(conn);
		dbus_message_unref(msg);
		if (g_Flag == 0)
		{
			pthread_mutex_lock(&lock);
			g_Flag = (1 & ~g_TimeChanged);
			pthread_mutex_unlock(&lock);
			sem_post(&g_Wait);
		}
		free(buf);
	}

	return NULL;
	
}

void *startBroadcastSystemTimePerMin(void *tmp)
{
	DBusConnection *conn = tmp;
	size_t periodicTimerID = 0;
	while (!g_Quit)
	{
		sem_wait(&g_Wait);

		if (g_TimeChanged || g_TimeSync)
		{
			// 若临时定时器已启动，则不做处理
			// 时钟发生变化，需要进行对时调整；关闭常驻定时器timerID，启动临时定时器
			// printf("Get Time Changed signal or mis-synced. stop timerID %zd\n", periodicTimerID);
			kdk_timer_stop(periodicTimerID);
			g_TimeChanged = 0;
			g_TimeSync = 0;
			periodicTimerID = 0;
		}

		if (!g_Flag)
			kdk_timer_start(200, printClock, KTIMER_SINGLESHOT, KTIMER_RELATIVE, conn, 0);
		else
		{
			// 当启动常驻定时器时，临时定时器肯定不需要再存在了
			periodicTimerID = kdk_timer_start(1000 * 60, printClock, KTIMER_PERIODIC, KTIMER_RELATIVE, conn, 0);
			// printf("start periodic timer with ID %zd\n", periodicTimerID);
		}
	}

	return NULL;
}

int monitorSystemTimeChange()
{
#define TIME_T_MAX (time_t)((1UL << ((sizeof(time_t) << 3) - 1)) - 1)
	// printf("monitorSystemTimeChange\n");
	struct itimerspec its = {.it_value.tv_sec = TIME_T_MAX};
	int fd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
	if (fd < 0)
	{
		return -1;
	}

	if (timerfd_settime(fd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET, &its, NULL) < 0)
	{
		close(fd);
		return -1;
	}

	// u_int64_t dep;
	char dep[8] = "\0";
	ssize_t ret = read(fd, dep, sizeof(dep));
	close(fd);
	if (ret == -1 && errno == ECANCELED)
		return 1;

	return 0;
}

void *actionTimeChanged(void *ptr)
{
	DBusConnection *conn = ptr;
	while (!g_Quit)
	{
		if (monitorSystemTimeChange() == 1)
		{
			// printf("System Time Changed.\n");
			g_Flag = 1;
			if (g_Flag)
			{
				g_TimeChanged = 1;
				g_Flag = 0;
				printClock(conn);
			}
		}
	}

	return NULL;
}
// 20020721新增时区变化监听
int monitorSystemTimeZoneChange(){

	char buf[BUFSIZ];
	int fd = inotify_init();
	buf[sizeof(buf) - 1] = 0;

	struct inotify_event *event;

	if (fd < 0)
	{
		return -1;
	}

	int ftimezone = inotify_add_watch(fd, "/etc/localtime", IN_DONT_FOLLOW);

	if (ftimezone < 0)
	{
		close(fd);
		return -1;
	}
	int ret = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	event = (struct inotify_event *)&buf[0];
	if (ret)
	{
		fprintf(stdout, "%s --- %s\n", event->name, "IN_DONT_FOLLOW");
		return 1;
	}
	return 0;
}
// 20020721新增时区变化监听
void *actionTimeZoneChanged(void *ptr)
{
	DBusConnection *conn = ptr;
	while (!g_Quit)
	{
		if (monitorSystemTimeZoneChange() == 1)
		{
			// printf("System Time Changed.\n");
			g_Flag = 1;
			if (g_Flag)
			{
				g_TimeChanged = 1;
				g_Flag = 0;
				printClock(conn);
			}
		}
	}

	return NULL;
}

const char *version = "0.1";
GMainLoop *mainloop;

/*
 * This implements 'Get' method of DBUS_INTERFACE_PROPERTIES so a
 * client can inspect the properties/attributes of 'TestInterface'.
 */

/*
 * This is the XML string describing the interfaces, methods and
 * signals implemented by our 'Server' object. It's used by the
 * 'Introspect' method of 'org.freedesktop.DBus.Introspectable'
 * interface.
 *
 * Currently our tiny server implements only 3 interfaces:
 *
 *    - org.freedesktop.DBus.Introspectable
 *    - org.freedesktop.DBus.Properties
 *    - org.example.TestInterface
 *
 * 'org.example.TestInterface' offers 3 methods:
 *
 *    - Ping(): makes the server answering the string 'Pong'.
 *              It takes no arguments.
 *
 *    - Echo(): replies the passed string argument.
 *
 *    - EmitSignal(): send a signal 'OnEmitSignal'
 *
 *    - Quit(): makes the server exit. It takes no arguments.
 */
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

	"  <interface name='com.lingmo.lingmosdk.TimeInterface'>\n"
	"    <signal name='TimeSignal'>\n"
	"       <arg name='tim' type='s' />\n"
	"    </signal>"
	"    <signal name='TimeChangeSignal'>\n"
	"       <arg name='tim' type='s' />\n"
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

int main(void)
{
	DBusConnection *conn;
	DBusError err;
	int rv;

	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (!conn)
	{
		fprintf(stderr, "Failed to get a system DBus connection: %s\n", err.message);
		goto fail;
	}

	rv = dbus_bus_request_name(conn, "com.lingmo.lingmosdk.TimeServer", DBUS_NAME_FLAG_REPLACE_EXISTING, &err);
	if (rv != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
		fprintf(stderr, "Failed to request name on bus: %s\n", err.message);
		goto fail;
	}

	if (!dbus_connection_register_object_path(conn, "/com/lingmo/lingmosdk/Timer", &server_vtable, NULL))
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
	pthread_create(&tid, &attr, actionTimeChanged, conn);
	
	// 20020721新增时区变化监听
	pthread_attr_t timezone_attr;
	pthread_t timezone_id;
	pthread_attr_init(&timezone_attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&timezone_id, &timezone_attr, actionTimeZoneChanged, conn);

	/* connect to the daemon bus */


	/*
	 * For the sake of simplicity we're using glib event loop to
	 * handle DBus messages. This is the only place where glib is
	 * used.
	 */
	pthread_attr_t tt;
	pthread_t dd;
	pthread_attr_init(&tt);
	pthread_attr_setdetachstate(&tt, PTHREAD_CREATE_DETACHED);
	pthread_create(&dd, &tt, startBroadcastSystemTimePerMin, conn);
	// printf("Starting dbus tiny server v%s\n", version);
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
