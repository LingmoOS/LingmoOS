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


#include "background-dbus.h"
#include "lingmo-background-interface.h"
#include "src/common/lingmo-print.h"
#include "background-generated.h"
#include <gio/gio.h>
#include <glib-2.0/glib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#define DB "draw-background"
#define SDI "show-desktop-icons"
#define BF "background-fade"
#define POPT "picture-options"
#define PF "picture-filename"
#define POPA "picture-opacity"
#define PC "primary-color"
#define SC "secondary-color"
#define CST "color-shading-type"

static GMainLoop *pLoop = NULL;
static GDBusConnection *pConnection = NULL;
static Background *pProxy = NULL;

void lingmo_personal_personal_drawbackground(const bool in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=DB;
    arg=g_variant_new("(sb)", key, in_arg);
    background_call_transfer_bool_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_personal_personal_drawbackground. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return;
}

void lingmo_personal_personal_showdesktopicons(const bool in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=SDI;
    arg=g_variant_new("(sb)", key, in_arg);
    background_call_transfer_bool_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_personal_personal_showdesktopicons. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return;
}

void lingmo_personal_personal_backgroundfade(const bool in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=BF;
    arg=g_variant_new("(sb)", key, in_arg);
    background_call_transfer_bool_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_personal_personal_backgroundfade. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return;
}

void lingmo_personal_personal_pictureoptions(const int in_arg)
{
    GError* pError=NULL;
    GVariant *arg=NULL;
    gchar *key=POPT;
    arg=g_variant_new("(si)",key,in_arg);
    background_call_transfer_int_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
       print_error("Failed to call method lingmo_personal_personal_pictureoptions. Reason: %s.\n", pError->message);
       g_error_free(pError);
       }

    return;

}

void lingmo_personal_personal_picturefilename(const char *in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=PF;
    arg=g_variant_new("(ss)", key, in_arg);
    background_call_transfer_string_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_personal_personal_picturefilename. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return;
}

void lingmo_personal_personal_pictureopacity(const int in_arg)
{
    GError* pError=NULL;
    GVariant *arg=NULL;
    gchar *key=POPA;
    arg=g_variant_new("(si)",key,in_arg);
    background_call_transfer_int_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
       print_error("Failed to call method lingmo_personal_personal_pictureopacity. Reason: %s.\n", pError->message);
       g_error_free(pError);
       }

    return;
}

void lingmo_personal_personal_primarycolor(const char *in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=PC;
    arg=g_variant_new("(ss)", key, in_arg);
    background_call_transfer_string_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_personal_personal_primarycolor. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return;
}

void lingmo_personal_personal_secondarycolor(const char *in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=SC;
    arg=g_variant_new("(ss)", key, in_arg);
    background_call_transfer_string_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_personal_personal_secondarycolor. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return;
}

void lingmo_personal_personal_colorshadingtype(const int in_arg)
{
    GError* pError=NULL;
    GVariant *arg=NULL;
    gchar *key=CST;
    arg=g_variant_new("(si)",key,in_arg);
    background_call_transfer_int_value_sync(pProxy, arg, NULL, &pError);

    if (pError != NULL){
       print_error("Failed to call method lingmo_personal_personal_colorshadingtype. Reason: %s.\n", pError->message);
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

bool InitDBusBackground(void)
 {
     bool bRet = TRUE;
     GError *pConnError = NULL;
     GError *pProxyError = NULL;

     do{
         bRet = TRUE;
         pLoop = g_main_loop_new(NULL, FALSE);   /** create main loop, but do not start it.*/

         /** First step: get a connection */
         pConnection = g_bus_get_sync(BACKGROUND_BUS, NULL, &pConnError);

         if (NULL == pConnError){
             /** Second step: try to get a connection to the given bus.*/
             pProxy = background_proxy_new_sync(pConnection,
                                         G_DBUS_PROXY_FLAGS_NONE,
                                         BACKGROUND_BUS_NAME,
                                         BACKGROUND_OBJECT_PATH,
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

bool DeInitDBusBackground(void)
{
    bool bRet = FALSE;

    if(0 != pLoop){
        g_main_loop_quit(pLoop);
        g_main_loop_unref(pLoop);
        bRet = TRUE;
    }

    return bRet;
}
