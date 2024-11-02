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
#include <glib-2.0/gio/gio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <glib-2.0/glib.h>
#include "lingmo-date-interface.h"
#include<stdbool.h>

#define USE_24_FORMAT "use-24h-format"
#define schemaID "org.mate.panel.indicator.calendar"
#define TZ_DATA_FILE "/usr/share/zoneinfo/zone_utc"

typedef struct _TimeDate TimeDate;
typedef struct _TzDB TzDB;
//typedef struct _TzLocation TzLocation;
typedef struct _TzUTC TzUTC;
TimeDate timedata;

struct _TzUTC{
    GPtrArray *tz_rtc;
    gchar *tz_utc;
};

struct _TzDB {
    GPtrArray *locations;
};


struct _TimeDate {
    TzDB *tzdb;
};

//Initialize the time zone database.
TzDB *init_timedb (){
    gchar *tz_data_file;
    TzDB *tz_db;
    FILE *tzfile;

    char buf[4096];

    tz_data_file = TZ_DATA_FILE;

    if (!tz_data_file){
        g_warning("Could not get timedb source\n");
        return NULL;
    }
    tzfile = fopen(tz_data_file, "r");
    if (!tzfile) {
        g_warning("Could not open tzfile *%s*\n",tz_data_file);
        //        g_free(tz_data_file);
        return NULL;
    }

    tz_db = g_new0(TzDB, 1);
    tz_db->locations = g_ptr_array_new();

    while(fgets(buf, sizeof(buf), tzfile)){
        gchar **tmpstrarr;
        TzUTC *loc;
        loc = g_new0(TzUTC, 1);
        loc->tz_rtc = g_ptr_array_new();

        g_strchomp(buf);
        tmpstrarr = g_strsplit(buf, "\t",2);
        gchar ***tmp_rtc = g_strsplit(tmpstrarr[0], " ", 6);

        for(int i =0; i!=6; ++i){
            if(tmp_rtc[i] != NULL){
                g_ptr_array_add (loc->tz_rtc, (gpointer)g_strdup(tmp_rtc[i]));
            }
            else
                break;
        }

        loc->tz_utc = g_strdup(tmpstrarr[1]);

        g_ptr_array_add (tz_db->locations, loc);

        g_strfreev(tmp_rtc);
        g_strfreev(tmpstrarr);
    }

    fclose(tzfile);
    return tz_db;
}

//Modify the time zone
void lingmo_date_dt_settz(char * location){
    gchar *rtc;
    TzUTC *tmp;
    GPtrArray *translate;
    timedata.tzdb= init_timedb();
    translate = timedata.tzdb->locations;

    if(!strcmp(getenv("LANG"), "en_US.UTF-8"))
        rtc = location;
    else{
        for(int i=0; i != timedata.tzdb->locations->len; ++i){
            tmp = g_ptr_array_index(timedata.tzdb->locations, i);

            if(tmp->tz_utc && strcmp(tmp->tz_utc, location) == 0){
                rtc = g_ptr_array_index(tmp->tz_rtc,0);
                break;
            }
        }
    }

    GError *error=NULL;
    GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,NULL,
                                                      "org.freedesktop.timedate1",
                                                      "/org/freedesktop/timedate1",
                                                      "org.freedesktop.timedate1",
                                                      NULL,&error);
    if (error !=NULL) {
        g_warning("Error :%s\n",error->message);
    }

    g_dbus_proxy_call_sync(proxy, "SetTimezone", g_variant_new("(sb)", rtc, TRUE),
                           G_DBUS_CALL_FLAGS_NONE, -1,NULL, NULL);

    //    g_free(location);
    return;
}

//Change the system date and time.
void lingmo_date_dt_chgdt(unsigned year,unsigned month,unsigned day, unsigned hour, unsigned minute, unsigned second){
    GDateTime *now;
    gint64 newtime;

    if(hour>23)
        hour = hour -12;

    now = g_date_time_new_local(year,month,day,hour,minute,(gdouble)second);
    newtime = g_date_time_to_unix(now);

    GError *error=NULL;
    GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,NULL, "org.freedesktop.timedate1",
                                                      "/org/freedesktop/timedate1",
                                                      "org.freedesktop.timedate1",
                                                      NULL,&error);
    if (error !=NULL) {
        g_warning("Error :%s\n",error->message);
    }

    g_dbus_proxy_call_sync(proxy, "SetTime", g_variant_new("(xbb)", (newtime * G_TIME_SPAN_SECOND), FALSE,TRUE),
                           G_DBUS_CALL_FLAGS_NONE,-1,NULL,NULL);
}

//Whether to enable network time synchronization.
void lingmo_date_dt_setnetsync(int timesetting)
{
    GError *error=NULL;
    GDBusProxy *proxy = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,NULL,
                                                      "org.freedesktop.timedate1",
                                                      "/org/freedesktop/timedate1",
                                                      "org.freedesktop.timedate1",
                                                      NULL,&error);
    if (error !=NULL) {
        g_warning("Error :%s\n",error->message);
    }

    if (timesetting == 0){
        g_dbus_proxy_call_sync(proxy, "SetNTP", g_variant_new("(bb)", FALSE, TRUE),
                               G_DBUS_CALL_FLAGS_NONE, -1,NULL, NULL);
    }else {
        g_dbus_proxy_call_sync(proxy, "SetNTP", g_variant_new("(bb)", TRUE, TRUE),
                               G_DBUS_CALL_FLAGS_NONE, -1,NULL, NULL);
    }
}

//Whether to enable time format.
void lingmo_date_dt_sethrfmt(bool value)
{
    GSettings *setting=g_settings_new(schemaID);
    g_settings_set_boolean(setting,USE_24_FORMAT,value);
    g_object_unref(setting);
    g_settings_sync();
}
