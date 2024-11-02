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


#include "mouse-dbus.h"
#include "lingmo-mouse-interface.h"
#include "src/common/lingmo-print.h"
#include "mouse-generated.h"
#include <gio/gio.h>
#include <glib-2.0/glib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#define LH "left-handed"
#define MA "motion-acceleration"
#define MT "motion-threshold"
#define DT "drag-threshold"
#define DC "double-click"
#define MBE "middle-button-enabled"
#define LP "locate-pointer"
#define CF "cursor-font"
#define CT "cursor-theme"
#define CS "cursor-size"

static GMainLoop *pLoop = NULL;
static GDBusConnection *pConnection = NULL;
static Mouse *pProxy = NULL;

bool lingmo_hardware_mouse_set_lefthanded(const bool in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=LH;
    gboolean retv;
    arg=g_variant_new("(sb)", key, in_arg);
    mouse_call_transfer_bool_value_sync(pProxy, arg, &retv,NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_hardware_mouse_lefthanded. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return retv;
}

bool lingmo_hardware_mouse_get_lefthanded()
{
    gchar *key=LH;
    gboolean value;
    GError* pError=NULL;
    mouse_call_get_bool_value_sync(pProxy, key,&value, NULL, &pError);
    return value;

}


bool lingmo_hardware_mouse_set_motionacceleration(const double in_arg)
{
    GError* pError=NULL;
    GVariant *arg=NULL;
    gchar *key=MA;
    gboolean retv;
    arg=g_variant_new("(sd)",key,in_arg);
    mouse_call_transfer_double_value_sync(pProxy, arg, &retv, NULL, &pError);

    if (pError != NULL){
       print_error("Failed to call method lingmo_hardware_mouse_motionacceleration. Reason: %s.\n", pError->message);
       g_error_free(pError);
       }

    return retv;
}

double lingmo_hardware_mouse_get_motionacceleration()
{
    gchar *key=MA;
    double value;
    GError* pError=NULL;
    mouse_call_get_double_value_sync(pProxy, key,&value, NULL, &pError);
    return value;
}

bool lingmo_hardware_mouse_set_motionthreshold(const int in_arg)
{
    GError* pError=NULL;
    GVariant *arg=NULL;
    gchar *key=MT;
    gboolean retv;
    arg=g_variant_new("(si)",key,in_arg);
    mouse_call_transfer_int_value_sync(pProxy, arg, &retv, NULL, &pError);

    if (pError != NULL){
       print_error("Failed to call method lingmo_hardware_mouse_motionthreshold. Reason: %s.\n", pError->message);
       g_error_free(pError);
       }

    return retv;

}

int lingmo_hardware_mouse_get_motionthreshold()
{
    gchar *key=MT;
    int value;
    GError* pError=NULL;
    mouse_call_get_int_value_sync(pProxy, key,&value, NULL, &pError);
    return value;
}

bool lingmo_hardware_mouse_set_dragthreshold(const int in_arg)
{
    GError* pError=NULL;
    GVariant *arg=NULL;
    gchar *key=DT;
    gboolean retv;
    arg=g_variant_new("(si)",key,in_arg);
    mouse_call_transfer_int_value_sync(pProxy, arg, &retv, NULL, &pError);

    if (pError != NULL){
       print_error("Failed to call method lingmo_hardware_mouse_dragthreshold. Reason: %s.\n", pError->message);
       g_error_free(pError);
       }

    return retv;
}

int lingmo_hardware_mouse_get_dragthreshold()
{
    gchar *key=DT;
    int value;
    GError* pError=NULL;
    mouse_call_get_int_value_sync(pProxy, key,&value, NULL, &pError);
    return value;

}

bool lingmo_hardware_mouse_set_doubleclick(const int in_arg)
{
    GError* pError=NULL;
    GVariant *arg=NULL;
    gchar *key=DC;
    gboolean retv;
    arg=g_variant_new("(si)",key,in_arg);
    mouse_call_transfer_int_value_sync(pProxy, arg, &retv, NULL, &pError);

    if (pError != NULL){
       print_error("Failed to call method lingmo_hardware_mouse_doubleclick. Reason: %s.\n", pError->message);
       g_error_free(pError);
       }

    return retv;
}

int lingmo_hardware_mouse_get_doubleclick()
{
    gchar *key=DC;
    int value;
    GError* pError=NULL;
    mouse_call_get_int_value_sync(pProxy, key,&value, NULL, &pError);
    return value;
}

bool lingmo_hardware_mouse_set_middlebuttonenabled(const bool in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=MBE;
    gboolean retv;
    arg=g_variant_new("(sb)", key, in_arg);
    mouse_call_transfer_bool_value_sync(pProxy, arg, &retv, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_hardware_mouse_middlebuttonenabled. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return retv;
}

bool lingmo_hardware_mouse_get_middlebuttonenabled()
{
    gchar *key=MBE;
    gboolean value;
    GError* pError=NULL;
    mouse_call_get_bool_value_sync(pProxy, key,&value, NULL, &pError);
    return value;

}

bool lingmo_hardware_mouse_set_locatepointer(const bool in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=LP;
    gboolean retv;
    arg=g_variant_new("(sb)", key, in_arg);
    mouse_call_transfer_bool_value_sync(pProxy, arg, &retv, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_hardware_mouse_locatepointer. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return retv;
}

bool lingmo_hardware_mouse_get_locatepointer()
{
    gchar *key=LP;
    gboolean value;
    GError* pError=NULL;
    mouse_call_get_bool_value_sync(pProxy, key,&value, NULL, &pError);
    return value;

}

bool lingmo_hardware_mouse_set_cursorfont(const char *in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=CF;
    gboolean retv;
    arg=g_variant_new("(ss)", key, in_arg);
    mouse_call_transfer_string_value_sync(pProxy, arg, &retv, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_hardware_mouse_cursorfont. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return retv;
}

char *lingmo_hardware_mouse_get_cursorfont()
{
    gchar *key=CF;
    char *value;
    GError* pError=NULL;
    mouse_call_get_string_value_sync(pProxy, key,&value, NULL, &pError);
    return value;

}

bool lingmo_hardware_mouse_set_cursortheme(const char *in_arg)
{
    GError* pError=NULL;
    GVariant* arg=NULL;
    gchar *key=CT;
    gboolean retv;
    arg=g_variant_new("(ss)", key, in_arg);
    mouse_call_transfer_string_value_sync(pProxy, arg, &retv, NULL, &pError);

    if (pError != NULL){
        print_error("Failed to call method lingmo_hardware_mouse_cursortheme. Reason: %s.\n", pError->message);
        g_error_free(pError);
        }

    return retv;
}

char *lingmo_hardware_mouse_get_cursortheme()
{
    gchar *key=CT;
    char *value;
    GError* pError=NULL;
    mouse_call_get_string_value_sync(pProxy, key,&value, NULL, &pError);
    return value;
}

bool lingmo_hardware_mouse_set_cursorsize(const int in_arg)
{
    GError* pError=NULL;
    GVariant *arg=NULL;
    gchar *key=CS;
    gboolean retv;
    arg=g_variant_new("(si)",key,in_arg);
    mouse_call_transfer_int_value_sync(pProxy, arg, &retv, NULL, &pError);

    if (pError != NULL){
       print_error("Failed to call method lingmo_hardware_mouse_cursorsize. Reason: %s.\n", pError->message);
       g_error_free(pError);
       }

    return retv;
}

int lingmo_hardware_mouse_get_cursorsize()
{
    gchar *key=CS;
    int value;
    GError* pError=NULL;
    mouse_call_get_int_value_sync(pProxy, key,&value, NULL, &pError);
    return value;
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

bool InitDBusMouse(void)
 {
     bool bRet = TRUE;
     GError *pConnError = NULL;
     GError *pProxyError = NULL;

     do{
         bRet = TRUE;
         pLoop = g_main_loop_new(NULL, FALSE);   /** create main loop, but do not start it.*/

         /** First step: get a connection */
         pConnection = g_bus_get_sync(MOUSE_BUS, NULL, &pConnError);

         if (NULL == pConnError){
             /** Second step: try to get a connection to the given bus.*/
             pProxy =mouse_proxy_new_sync(pConnection,
                                         G_DBUS_PROXY_FLAGS_NONE,
                                         MOUSE_BUS_NAME,
                                         MOUSE_OBJECT_PATH,
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

bool DeInitDBusMouse(void)
{
    bool bRet = FALSE;

    if(0 != pLoop){
        g_main_loop_quit(pLoop);
        g_main_loop_unref(pLoop);
        bRet = TRUE;
    }

    return bRet;
}
