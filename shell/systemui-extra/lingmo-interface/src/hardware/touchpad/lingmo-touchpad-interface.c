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


#include "touchpad-dbus.h"
#include "lingmo-touchpad-interface.h"
#include "src/common/lingmo-print.h"
#include "touchpad-generated.h"
#include <gio/gio.h>
#include <glib-2.0/glib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#define TE "touchpad-enabled"
#define TTC "tap-to-click"
#define SM "scroll-method"
#define NS "natural-scroll"

static GMainLoop *pLoop = NULL;
static GDBusConnection *pConnection = NULL;
static Touchpad *pProxy = NULL;

void lingmo_hardware_touchpad_touchpadenabled(const bool in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=TE;
    arg=g_variant_new("(sb)", key, in_arg);
    touchpad_call_transfer_bool_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_hardware_touchpad_touchpadenabled. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return;
}

void lingmo_hardware_touchpad_taptoclick(const bool in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=TTC;
    arg=g_variant_new("(sb)", key, in_arg);
    touchpad_call_transfer_bool_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_hardware_touchpad_taptoclick. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return;
}

void lingmo_hardware_touchpad_scrollmethod(const int in_arg)
{
    GError* pError=NULL;
    GVariant *arg=NULL;
    gchar *key=SM;
    arg=g_variant_new("(si)",key,in_arg);
    touchpad_call_transfer_int_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
       print_error("Failed to call method lingmo_hardware_touchpad_scrollmethod. Reason: %s.\n", pError->message);
       g_error_free(pError);
       }

    return;

}

void lingmo_hardware_touchpad_naturalscroll(const bool in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=NS;
    arg=g_variant_new("(sb)", key, in_arg);
    touchpad_call_transfer_bool_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_hardware_touchpad_naturalscroll. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return;
}

void *run(void* arg)
{
    /** start the main event loop which manages all available sources of events */
    g_main_loop_run(pLoop);

    return ((void*)0);
}

int thread_create(void)
 {
     int err;
     pthread_t thr;

     err = pthread_create(&thr, NULL, run, NULL);

     if(0 != err){
             print_error("Can't create thread: %s\n", strerror(err));
     }

     return err;
 }

bool InitDBusTouchpad(void)
 {
     bool bRet = TRUE;
     GError *pConnError = NULL;
     GError *pProxyError = NULL;

     do{
         bRet = TRUE;
         pLoop = g_main_loop_new(NULL, FALSE);   /** create main loop, but do not start it.*/

         /** First step: get a connection */
         pConnection = g_bus_get_sync(TOUCHPAD_BUS, NULL, &pConnError);

         if (NULL == pConnError){
             /** Second step: try to get a connection to the given bus.*/
             pProxy = touchpad_proxy_new_sync(pConnection,
                                         G_DBUS_PROXY_FLAGS_NONE,
                                         TOUCHPAD_BUS_NAME,
                                         TOUCHPAD_OBJECT_PATH,
                                         NULL,
                                         &pProxyError);
             if (0 == pProxy){
                 print_error("InitDBusCommunication: Failed to create proxy. Reason: %s.\n", pProxyError->message);
                 g_error_free(pProxyError);
                 bRet = FALSE;
             }


         }
         else{
             print_error("InitDBusCommunication: Failed to connect to dbus. Reason: %s.\n", pConnError->message);
             g_error_free(pConnError);
             bRet = FALSE;
         }
     }while(FALSE == bRet);

     thread_create();

     return bRet;
 }

bool DeInitDBusTouchpad(void)
{
    bool bRet = FALSE;

    if(0 != pLoop){
        g_main_loop_quit(pLoop);
        g_main_loop_unref(pLoop);
        bRet = TRUE;
    }

    return bRet;
}
