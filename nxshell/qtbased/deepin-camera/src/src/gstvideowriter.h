// Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/base/gstbaseparse.h>
#include <gst/gstvalue.h>
#include <gst/gstminiobject.h>

#include <QString>

typedef void (*mvideo_gst_init)(int *argc, char **argv[]);
typedef GstElement* (*mvideo_gst_parse_launch) (const gchar *pipeline_description, GError **error) G_GNUC_MALLOC;
typedef GstElement*	(*mvideo_gst_bin_get_by_name) (GstBin *bin, const gchar *name);
typedef GstBus* (*mvideo_gst_pipeline_get_bus) (GstPipeline *pipeline);
typedef void (*mvideo_gst_app_src_set_caps) (GstAppSrc *appsrc, const GstCaps *caps);
typedef GstBuffer* (*mvideo_gst_buffer_new_wrapped) (gpointer data, gsize size);
typedef GstStateChangeReturn (*mvideo_gst_element_set_state) (GstElement *element, GstState state);
typedef void (*mvideo_gst_message_parse_error) (GstMessage *message, GError **gerror, gchar **debug);
typedef void (*mvideo_gst_object_unref) (gpointer object);
typedef GstClockTime (*mvideo_gst_clock_get_time) (GstClock *clock);
typedef void (*mvideo_gst_buffer_unref) (GstBuffer * buf);
typedef guint (*mvideo_gst_bus_add_watch) (GstBus * bus, GstBusFunc func, gpointer user_data);
typedef GType (*mvideo_gst_bin_get_type) (void);
typedef GstCaps* (*mvideo_gst_caps_new_simple) (const char *media_type, const char    *fieldname,
                                                ...) G_GNUC_NULL_TERMINATED G_GNUC_WARN_UNUSED_RESULT;
typedef GType (*mvideo_gst_pipeline_get_type) (void);
typedef GType (*mvideo_gst_app_src_get_type)  (void);
typedef void (*mvideo_gst_mini_object_unref) (GstMiniObject *mini_object);
typedef GType (*mvideo_gst_fraction_type);

class GstVideoWriter
{

public:
    GstVideoWriter(const QString& videoPath = "test.webm");
    ~GstVideoWriter();

    // 开始录制
    void start();
    // 停止录制
    void stop();
    // 设置视频保存路径
    void setVideoPath(const QString& videoPath);
    // 设置视频成像质量
    void setQuantizer(uint quantizer);
    // 写入视频帧数据，格式为rgb
    bool writeFrame(uchar *rgb, uint size);
    // 写入音频帧数据，格式为f32le
    bool writeAudio(uchar *audio, uint size);
    // 获取当前录制时长
    float getRecrodTime();

    // 获取管道对象
    inline GstElement* getPipeLine() { return m_pipeline; };
    // 获取GStreamer主循环对象
    inline GMainLoop* getGloop() { return m_gloop; };

protected:
    void init();
    void loadAppSrcCaps();

private:
    QString libPath(const QString &strlib);
    GstBin *getGstBin(GstElement *element);
    GstPipeline *getGstPipline(GstElement *element);
    GstAppSrc *getGstAppSrc(GstElement *element);

private:
    QString                     m_videoPath; //视频保存路径
    uint                        m_nWidth; // 录制视频分辨率宽度
    uint                        m_nHeight; // 录制视频分辨率高度
    uint                        m_nFrameRate; // 帧率
    uint                        m_nQuantizer; // 成像质量 范围4-63, 默认为44 值约小，成像质量越高，视频画面越清晰，编码耗时越长，视频文件占用磁盘控件越大

    GstElement                 *m_pipeline; // 管道实例
    GMainLoop                  *m_gloop;
    GstElement                 *m_appsrc;
    GstElement                 *m_audsrc;
    GstElement                 *m_vp8enc;
    GstElement                 *m_filesink;
    GstBus                     *m_bus;

    mvideo_gst_init            g_mvideo_gst_init = nullptr;
    mvideo_gst_parse_launch    g_mvideo_gst_parse_launch = nullptr;
    mvideo_gst_bin_get_by_name g_mvideo_gst_bin_get_by_name = nullptr;
    mvideo_gst_pipeline_get_bus g_mvideo_gst_pipeline_get_bus = nullptr;
    mvideo_gst_app_src_set_caps g_mvideo_gst_app_src_set_caps = nullptr;
    mvideo_gst_buffer_new_wrapped g_mvideo_gst_buffer_new_wrapped = nullptr;
    mvideo_gst_object_unref    g_mvideo_gst_object_unref = nullptr;
    mvideo_gst_clock_get_time  g_mvideo_gst_clock_get_time = nullptr;
    mvideo_gst_buffer_unref    g_mvideo_gst_buffer_unref = nullptr;
    mvideo_gst_bus_add_watch   g_mvideo_gst_bus_add_watch = nullptr;
    mvideo_gst_bin_get_type    g_mvideo_gst_bin_get_type = nullptr;
    mvideo_gst_caps_new_simple g_mvideo_gst_caps_new_simple = nullptr;
    mvideo_gst_pipeline_get_type g_mvideo_gst_pipeline_get_type = nullptr;
    mvideo_gst_app_src_get_type g_mvideo_gst_app_src_get_type = nullptr;
    mvideo_gst_mini_object_unref g_mvideo_gst_mini_object_unref = nullptr;
    mvideo_gst_fraction_type   g_mvideo_gst_fraction_type = nullptr;
};
