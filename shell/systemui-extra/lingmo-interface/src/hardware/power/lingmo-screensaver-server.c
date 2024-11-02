/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */


#include <stdio.h>
#include <gio/gio.h>
#include <glib-2.0/glib.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "screensaver-dbus.h"
#include "src/common/lingmo-gsettings-set.h"
#include "src/common/lingmo-print.h"
#include "screensaver-generated.h"
#include <string.h>
#include <stdbool.h>

#define schemaID "org.mate.screensaver"

static GMainLoop         *pLoop     = NULL;
static Screensaver *pSkeleton = NULL;

static gboolean lingmo_set_bool(Screensaver* object,
                               GDBusMethodInvocation* invocation,
                               GVariant *in_arg,
                               gpointer user_data)
{
    gchar *key=NULL;
    gboolean value;
    gint i;
    g_variant_get(in_arg, "(sb)", &key, &value);
    if(value==false)i=0;
    else i=1;

    screensaver_complete_transfer_bool_value(object, invocation);

    GSettings *setting=gsettings_get_schemaID(schemaID);
    gsettings_set_boolean(setting, key, value);
    Dec_ref(setting);

    print_info("(%s, %d)\n", key, i);

    return TRUE;
}

static void bus_acquired_cb(GDBusConnection* connection,
                            const gchar* bus_name,
                            gpointer user_data)
{

    GError *pError = NULL;

    /** Second step: Try to get a connection to the given bus. */
    pSkeleton = screensaver_skeleton_new();

    /** Third step: Attach to dbus signals. */
    (void) g_signal_connect(pSkeleton, "handle-transfer-bool-value", G_CALLBACK(lingmo_set_bool), NULL);

    /** Fourth step: Export interface skeleton. */
    (void)g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(pSkeleton),
                                              connection,
                                              SCREENSAVER_OBJECT_PATH,
                                              &pError);

    if(pError != NULL){
        print_error("Error: Failed to export object. Reason: %s.\n", pError->message);
        g_error_free(pError);
        g_main_loop_quit(pLoop);
    }
}

static void name_acquired_cb(GDBusConnection* connection,
                             const gchar* bus_name,
                             gpointer user_data)
{
    print_info("name_acquired_cb call, Acquired bus name: %s.\n", SCREENSAVER_BUS_NAME);
}

static void name_lost_cb(GDBusConnection* connection,
                         const gchar* bus_name,
                         gpointer user_data)
{
    if(connection == NULL)
        {
            print_error("name_lost_cb call, Error: Failed to connect to dbus.\n");
        }
        else
        {
            print_error("name_lost_cb call, Error: Failed to obtain bus name: %s.\n", SCREENSAVER_BUS_NAME);
        }

        g_main_loop_quit(pLoop);
}

void* run(void* para)
{
    /** Start the Main Event Loop which manages all available sources of events */
    g_main_loop_run( pLoop );

    return ((void*)0);
}

int thread_create(void)
{
    int err;
    pthread_t thr;

    err = pthread_create(&thr, NULL, run, NULL);

    if (err != 0)
    {

        print_error("Can't create thread: %s\n", strerror(err));
    }

    return err;
}

bool InitDBusCommunicationServer(void)
{
    bool bRet = TRUE;

    g_print("InitDBusCommunicationServer: Server started.\n");

    /** create main loop, but do not start it. */
    pLoop = g_main_loop_new(NULL, FALSE);

    /** first step: connect to dbus */
    (void)g_bus_own_name(SCREENSAVER_BUS,
                        SCREENSAVER_BUS_NAME,
                        G_BUS_NAME_OWNER_FLAGS_NONE,
                        &bus_acquired_cb,
                        &name_acquired_cb,
                        &name_lost_cb,
                        NULL,
                        NULL);

    thread_create();

    return bRet;
}

bool DeinitDBusCommunicationServer(void)
{
    bool bRet = FALSE;
    if(0 != pLoop){
        g_main_loop_quit(pLoop);
        g_main_loop_unref(pLoop);
        bRet = TRUE;
    }

    return bRet;
}

int main(void)
{
    InitDBusCommunicationServer();

    sleep(100);

    DeinitDBusCommunicationServer();
    return 0;
}
