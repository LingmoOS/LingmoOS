// Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gstvideowriter.h"
extern "C" {
#include <gst/app/gstappsrc.h>
#include "v4l2_core.h"
#include "camview.h"
#include "gviewaudio.h"
#include "audio.h"
}

#include <QtConcurrent>

GST_DEBUG_CATEGORY(appsrc_pipeline_debug);

#define NORMAL_QUANTIZER 30

static mvideo_gst_element_set_state g_mvideo_gst_element_set_state = nullptr;
static mvideo_gst_message_parse_error g_mvideo_gst_message_parse_error = nullptr;
static gboolean
bus_message(GstBus * bus, GstMessage  * message, GstVideoWriter * app)
{
    switch (message->type) {
    case GST_MESSAGE_ERROR: {
        GError *err = NULL;
        gchar *dbg_info = NULL;

        g_mvideo_gst_message_parse_error(message, &err, &dbg_info);

        gchar* elename = message->src->name;

        g_printerr("ERROR from element %s: %s\n", elename, err->message);
        g_printerr("Debugging info: %s\n", (dbg_info) ? dbg_info : "none");

        g_error_free(err);
        g_free(dbg_info);

        // 结束录制任务
        g_main_loop_quit(app->getGloop());
        // 停止管道
        g_mvideo_gst_element_set_state(app->getPipeLine(), GST_STATE_NULL);
        break;
    }
    case GST_MESSAGE_EOS: {
        // 结束录制任务
        g_main_loop_quit(app->getGloop());
        // 停止管道
        g_mvideo_gst_element_set_state(app->getPipeLine(), GST_STATE_NULL);
        break;
    }
    default:
        break;
    }

    return TRUE;
}

GstVideoWriter::GstVideoWriter(const QString& videoPath):
    m_videoPath(videoPath)
  , m_nWidth(1920)
  , m_nHeight(1080)
  , m_nFrameRate(30)
  , m_nQuantizer(NORMAL_QUANTIZER)
  , m_pipeline(nullptr)
  , m_gloop(nullptr)
  , m_appsrc(nullptr)
  , m_audsrc(nullptr)
  , m_vp8enc(nullptr)
  , m_filesink(nullptr)
  , m_bus(nullptr)
{
    QLibrary gstreamerLibrary(libPath("libgstreamer-1.0.so"));
    QLibrary gstreamerAppLibrary(libPath("libgstapp-1.0.so"));

    g_mvideo_gst_init = (mvideo_gst_init) gstreamerLibrary.resolve("gst_init");
    g_mvideo_gst_parse_launch = (mvideo_gst_parse_launch) gstreamerLibrary.resolve("gst_parse_launch");
    g_mvideo_gst_bin_get_by_name = (mvideo_gst_bin_get_by_name) gstreamerLibrary.resolve("gst_bin_get_by_name");
    g_mvideo_gst_pipeline_get_bus = (mvideo_gst_pipeline_get_bus) gstreamerLibrary.resolve("gst_pipeline_get_bus");
    g_mvideo_gst_app_src_set_caps = (mvideo_gst_app_src_set_caps) gstreamerAppLibrary.resolve("gst_app_src_set_caps");
    g_mvideo_gst_buffer_new_wrapped = (mvideo_gst_buffer_new_wrapped) gstreamerLibrary.resolve("gst_buffer_new_wrapped");
    g_mvideo_gst_object_unref = (mvideo_gst_object_unref) gstreamerLibrary.resolve("gst_object_unref");
    g_mvideo_gst_clock_get_time = (mvideo_gst_clock_get_time) gstreamerLibrary.resolve("gst_clock_get_time");
    g_mvideo_gst_buffer_unref = (mvideo_gst_buffer_unref) gstreamerLibrary.resolve("gst_buffer_unref");
    g_mvideo_gst_bus_add_watch = (mvideo_gst_bus_add_watch) gstreamerLibrary.resolve("gst_bus_add_watch");
    g_mvideo_gst_bin_get_type = (mvideo_gst_bin_get_type) gstreamerLibrary.resolve("gst_bin_get_type");
    g_mvideo_gst_caps_new_simple = (mvideo_gst_caps_new_simple) gstreamerLibrary.resolve("gst_caps_new_simple");
    g_mvideo_gst_pipeline_get_type = (mvideo_gst_pipeline_get_type) gstreamerLibrary.resolve("gst_pipeline_get_type");
    g_mvideo_gst_app_src_get_type = (mvideo_gst_app_src_get_type) gstreamerAppLibrary.resolve("gst_app_src_get_type");
    g_mvideo_gst_mini_object_unref = (mvideo_gst_mini_object_unref) gstreamerAppLibrary.resolve("gst_mini_object_unref");
    g_mvideo_gst_fraction_type = (mvideo_gst_fraction_type) gstreamerLibrary.resolve("_gst_fraction_type");

    g_mvideo_gst_element_set_state = (mvideo_gst_element_set_state) gstreamerLibrary.resolve("gst_element_set_state");
    g_mvideo_gst_message_parse_error = (mvideo_gst_message_parse_error) gstreamerLibrary.resolve("gst_message_parse_error");

    init();
}

GstVideoWriter::~GstVideoWriter()
{
    g_mvideo_gst_object_unref(m_pipeline);
    g_mvideo_gst_object_unref(m_appsrc);
    g_mvideo_gst_object_unref(m_audsrc);
    g_mvideo_gst_object_unref(m_vp8enc);
    g_mvideo_gst_object_unref(m_filesink);
    g_mvideo_gst_object_unref(m_bus);
    g_main_loop_unref(m_gloop);
}

void GstVideoWriter::start()
{
    // 设置视频帧数据格式
    loadAppSrcCaps();

#if defined(__mips__) || defined(__aarch64__)
    // mips/arm下，牺牲了成像质量
    if (m_nWidth >= 1920) {
        setQuantizer(30);
    }
    else if (m_nWidth >= 1280) {
        setQuantizer(10);
    }
    else
        setQuantizer(5);
#endif

    // 启动管道
    g_mvideo_gst_element_set_state(m_pipeline, GST_STATE_PLAYING);

    // 将录制任务放到线程中运行，bus回调收到eos信号，结束录制任务
    QtConcurrent::run(g_main_loop_run, m_gloop);
}

void GstVideoWriter::stop()
{
    GstFlowReturn ret;

    // 停止音频流
    if (m_audsrc)
        g_signal_emit_by_name(m_audsrc, "end-of-stream", &ret);

    // 停止视频流
    if (m_appsrc)
        g_signal_emit_by_name(m_appsrc, "end-of-stream", &ret);
}

void GstVideoWriter::setVideoPath(const QString &videoPath)
{
    if (!videoPath.isEmpty() && m_videoPath != videoPath)
        m_videoPath = videoPath;

    if (m_filesink)
        g_object_set(m_filesink, "location", videoPath.toStdString().c_str(), NULL);
}

void GstVideoWriter::setQuantizer(uint quantizer)
{
    m_nQuantizer = quantizer;

    if (m_vp8enc)
        g_object_set(m_vp8enc, "min-quantizer", m_nQuantizer, NULL);
}

bool GstVideoWriter::writeFrame(uchar *rgb, uint size)
{
    GstFlowReturn ret = GST_FLOW_CUSTOM_ERROR;
    guint8 *ptr = (guint8 *)g_malloc(size * sizeof(uchar));
    if (ptr) {
        memcpy(ptr, rgb, size);
        GstBuffer *buffer = g_mvideo_gst_buffer_new_wrapped((void*)ptr, size);

        //设置时间戳
        if (m_pipeline)
            GST_BUFFER_PTS(buffer) = g_mvideo_gst_clock_get_time(m_pipeline->clock) - m_pipeline->base_time;

        //注入视频帧数据
        if (m_appsrc)
            g_signal_emit_by_name(m_appsrc, "push-buffer", buffer, &ret);

        g_mvideo_gst_mini_object_unref(GST_MINI_OBJECT_CAST(buffer));
    }

    return ret == GST_FLOW_OK;
}

bool GstVideoWriter::writeAudio(uchar *audio, uint size)
{
    GstFlowReturn ret = GST_FLOW_CUSTOM_ERROR;
    guint8 *ptr = (guint8 *)g_malloc(size * sizeof(uchar));
    if (ptr) {
        memcpy(ptr, audio, size);
        GstBuffer *buffer = g_mvideo_gst_buffer_new_wrapped((void*)ptr, size);

        // 设置时间戳
        if (m_pipeline)
            GST_BUFFER_PTS(buffer) = g_mvideo_gst_clock_get_time(m_pipeline->clock) - m_pipeline->base_time;

        // 注入音频帧数据
        if (m_audsrc) {
            g_signal_emit_by_name(m_audsrc, "push-buffer", buffer, &ret);
        }

        g_mvideo_gst_mini_object_unref(GST_MINI_OBJECT_CAST(buffer));
    }

    return ret == GST_FLOW_OK;
}

float GstVideoWriter::getRecrodTime()
{
    float pts = 0;
    if (m_pipeline)
        pts = g_mvideo_gst_clock_get_time(m_pipeline->clock) - m_pipeline->base_time;

    return pts / 1000 / 1000 / 1000;
}

void GstVideoWriter::init()
{
    g_mvideo_gst_init(nullptr, nullptr);
    // 使用vp8编码录制视频裸流数据、使用vorbis编码录制音频裸流数据
    QString pipDesc = QString("webmmux name=mux ! filesink name=filename "
                              "appsrc name=source ! queue ! videoconvert primaries-mode=2  name=convert ! queue ! "
                              "vp8enc nd-usage=vbr min-quantizer=1 max-quantizer=50 undershoot=95 cpu-used=5 deadline=1 static-threshold=50 error-resilient=1 name=encoder ! queue ! mux.video_0 "
                              "appsrc name=audiosource ! queue ! audioconvert ! audioresample ! vorbisenc ! queue ! mux.audio_0");
    m_pipeline = g_mvideo_gst_parse_launch(pipDesc.toStdString().c_str(), NULL);
    g_assert(m_pipeline);

    m_gloop = g_main_loop_new(NULL, TRUE);

    // 设置视频保存路径
    m_filesink = g_mvideo_gst_bin_get_by_name(getGstBin(m_pipeline), "filename");
    setVideoPath(m_videoPath);

    // 设置vp8编码器线程数和成像质量
    m_vp8enc = g_mvideo_gst_bin_get_by_name(getGstBin(m_pipeline), "encoder");

    m_bus = g_mvideo_gst_pipeline_get_bus(getGstPipline(m_pipeline));
    if (m_bus)
        g_mvideo_gst_bus_add_watch(m_bus, (GstBusFunc)bus_message, this);

    /* 设置视频src属性 */
    m_appsrc = g_mvideo_gst_bin_get_by_name(getGstBin(m_pipeline), "source");
    if (m_appsrc) {
        loadAppSrcCaps();
        g_object_set(m_appsrc, "format", GST_FORMAT_TIME, NULL);
        g_object_set(m_appsrc, "is-live", TRUE, NULL);
    }

    /* 设置音频src属性 */
    m_audsrc = g_mvideo_gst_bin_get_by_name(getGstBin(m_pipeline), "audiosource");
    if (m_audsrc) {
        GstCaps *audiocaps = g_mvideo_gst_caps_new_simple("audio/x-raw",
            "format", G_TYPE_STRING, "F32LE",
            "layout", G_TYPE_STRING, "interleaved",
            "channels", G_TYPE_INT, 2,
            "rate", G_TYPE_INT, 44100,
            NULL);
        g_mvideo_gst_app_src_set_caps(getGstAppSrc(m_audsrc), audiocaps);
        g_object_set(m_audsrc, "format", GST_FORMAT_TIME, NULL);
        g_object_set(m_audsrc, "is-live", TRUE, NULL);
    }
}

void GstVideoWriter::loadAppSrcCaps()
{
    // 获取当前相机分辨率
    m_nWidth = v4l2core_get_frame_width(get_v4l2_device_handler());
    m_nHeight = v4l2core_get_frame_height(get_v4l2_device_handler());
    if (m_nWidth > 0 && m_nHeight > 0 && m_appsrc) {
        GstCaps *caps = g_mvideo_gst_caps_new_simple("video/x-raw",
            "format", G_TYPE_STRING, "RGB",
            "width", G_TYPE_INT, m_nWidth,
            "height", G_TYPE_INT, m_nHeight,
            "framerate", (*g_mvideo_gst_fraction_type), m_nFrameRate, 1,
            NULL);
        g_mvideo_gst_app_src_set_caps(getGstAppSrc(m_appsrc), caps);
    }
}

QString GstVideoWriter::libPath(const QString &strlib)
{
    QDir  dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    if (list.contains(strlib)) {
        return strlib;
    } else {
        list.sort();
    }

    if(list.size() > 0)
        return list.last();

    // Qt LibrariesPath 不包含，返回默认名称
    return strlib;
}

GstBin *GstVideoWriter::getGstBin(GstElement *element)
{
    return (GstBin *)g_type_check_instance_cast((GTypeInstance *)element, g_mvideo_gst_bin_get_type());
}

GstPipeline* GstVideoWriter::getGstPipline(GstElement *element)
{
    return (GstPipeline*) g_type_check_instance_cast((GTypeInstance *)element, g_mvideo_gst_pipeline_get_type());
}

GstAppSrc* GstVideoWriter::getGstAppSrc(GstElement *element)
{
    return (GstAppSrc*) g_type_check_instance_cast((GTypeInstance *)element, g_mvideo_gst_app_src_get_type());
}
