// Copyright (C) 2020 ~ now Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GSTINTERFACE_H
#define GSTINTERFACE_H
#include <QObject>
#include <QLibrary>

#include <gst/app/gstappsrc.h>
#include <gobject/gtype.h>
#include <gst/gst.h>

class gstInterface
{
public:
    gstInterface();
    /**
     * @brief libPath
     * @param sLib
     * @return
     */
    static QString libPath(const QString &sLib);
    /**
     * @brief 初始化gstreamer函数
     */
    static void initFunctions();
    /**
     * @brief 释放gstreamer函数
     */
    static void unloadFunctions();

    typedef void (*p_gst_message_parse_error)(GstMessage *, GError **, gchar **);//-lglib-2.0
    typedef void (*p_g_free)(gpointer);//-lglib-2.0
    typedef GError *(*p_g_error_copy)(const GError *);//-lglib-2.0
    typedef void(*p_g_main_loop_quit)(GMainLoop *);//-lglib-2.0
    typedef void(*p_g_main_loop_run)(GMainLoop *);//-lglib-2.0
    typedef GMainLoop *(*p_g_main_loop_new)(GMainContext *, gboolean);//-lglib-2.0
    typedef gpointer(*p_g_malloc)(gsize); //-lglib-2.0

    typedef void(*p_gst_init)(int *, char **[]);//-lgstreamer-1.0
    typedef GstStateChangeReturn(*p_gst_element_set_state)(GstElement *, GstState);//-lgstreamer-1.0
    typedef void(*p_gst_object_unref)(gpointer);//-lgstreamer-1.0
    typedef GstElement *(*p_gst_bin_get_by_name)(GstBin *, const gchar *); //-lgstreamer-1.0
    typedef GstBus *(*p_gst_pipeline_get_bus)(GstPipeline *); //-lgstreamer-1.0
    typedef guint(*p_gst_bus_add_watch)(GstBus *, GstBusFunc, gpointer); //-lgstreamer-1.0
    typedef GstBuffer *(*p_gst_buffer_new_wrapped)(gpointer, gsize); //-lgstreamer-1.0
    typedef GstClockTime(*p_gst_clock_get_time)(GstClock *); //-lgstreamer-1.0
    typedef void(*p_gst_buffer_unref)(GstBuffer *); //-lgstreamer-1.0
    typedef void(*p_gst_mini_object_unref)(GstMiniObject *); //-lgstreamer-1.0
    typedef gboolean(*p_gst_element_send_event)(GstElement *, GstEvent *); //-lgstreamer-1.0
    typedef GstEvent *(*p_gst_event_new_eos)(void); //-lgstreamer-1.0
    typedef GstMessage *(*p_gst_bus_timed_pop_filtered)(GstBus *, GstClockTime, GstMessageType);    //-lgstreamer-1.0
    typedef GstElement *(*p_gst_parse_launch)(const gchar *, GError **);     //-lgstreamer-1.0
    typedef GType(*p_gst_bin_get_type)(void);     //-lgstreamer-1.0

    typedef GType(*p_g_type_check_instance_cast)(GTypeInstance *, GType);     //-lgobject-2.0
    typedef void(*p_g_object_set)(gpointer, const gchar *, ...);//-lgobject-2.0
    typedef void(*p_g_signal_emit_by_name)(gpointer, const gchar *, ...);//-lgobject-2.0

    //-lglib-2.0
    static p_gst_message_parse_error m_gst_message_parse_error;
    static p_g_free m_g_free;
    static p_g_error_copy m_g_error_copy;
    static p_g_main_loop_quit m_g_main_loop_quit;
    static p_g_main_loop_run m_g_main_loop_run;
    static p_g_main_loop_new m_g_main_loop_new;
    static p_g_malloc m_g_malloc;

    //-lgstreamer-1.0
    static p_gst_init m_gst_init;
    static p_gst_element_set_state m_gst_element_set_state;
    static p_gst_object_unref m_gst_object_unref;
    static p_gst_bin_get_by_name m_gst_bin_get_by_name;
    static p_gst_pipeline_get_bus m_gst_pipeline_get_bus;
    static p_gst_bus_add_watch m_gst_bus_add_watch;
    static p_gst_buffer_new_wrapped m_gst_buffer_new_wrapped;
    static p_gst_clock_get_time m_gst_clock_get_time;
    static p_gst_buffer_unref m_gst_buffer_unref;
    static p_gst_mini_object_unref m_gst_mini_object_unref;
    static p_gst_element_send_event m_gst_element_send_event;
    static p_gst_event_new_eos m_gst_event_new_eos;
    static p_gst_bus_timed_pop_filtered m_gst_bus_timed_pop_filtered;
    static p_gst_parse_launch m_gst_parse_launch;
    static p_gst_bin_get_type m_gst_bin_get_type;

    //-lgobject-2.0
    static p_g_type_check_instance_cast m_g_type_check_instance_cast;
    static p_g_object_set m_g_object_set;
    static p_g_signal_emit_by_name m_g_signal_emit_by_name;


public:
    static bool m_isInitFunction;

    //-lgstreamer-1.0 -lgobject-2.0 -lglib-2.0
    static QLibrary m_libgstreamer;
    static QLibrary m_libglib;
    static QLibrary m_libgobject;


};

#endif // GSTINTERFACE_H
