// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gstutils.h"
#include "compositing_manager.h"

#include <QDebug>

namespace dmr {

static mvideo_gst_discoverer_info_get_uri g_mvideo_gst_discoverer_info_get_uri = nullptr;
static mvideo_gst_discoverer_info_get_result g_mvideo_gst_discoverer_info_get_result = nullptr;
static mvideo_gst_discoverer_info_get_misc g_mvideo_gst_discoverer_info_get_misc = nullptr;
static mvideo_gst_structure_to_string g_mvideo_gst_structure_to_string = nullptr;
static mvideo_gst_discoverer_info_get_video_streams g_mvideo_gst_discoverer_info_get_video_streams = nullptr;
static mvideo_gst_discoverer_info_get_audio_streams g_mvideo_gst_discoverer_info_get_audio_streams = nullptr;
static mvideo_gst_discoverer_video_info_get_width g_mvideo_gst_discoverer_video_info_get_width = nullptr;
static mvideo_gst_discoverer_video_info_get_height g_mvideo_gst_discoverer_video_info_get_height = nullptr;
static mvideo_gst_discoverer_video_info_get_framerate_num g_mvideo_gst_discoverer_video_info_get_framerate_num = nullptr;
static mvideo_gst_discoverer_video_info_get_framerate_denom g_mvideo_gst_discoverer_video_info_get_framerate_denom = nullptr;
static mvideo_gst_discoverer_video_info_get_bitrate g_mvideo_gst_discoverer_video_info_get_bitrate = nullptr;
static mvideo_gst_discoverer_info_get_duration g_mvideo_gst_discoverer_info_get_duration = nullptr;
static mvideo_gst_discoverer_audio_info_get_sample_rate g_mvideo_gst_discoverer_audio_info_get_sample_rate = nullptr;
static mvideo_gst_discoverer_audio_info_get_bitrate g_mvideo_gst_discoverer_audio_info_get_bitrate = nullptr;
static mvideo_gst_discoverer_audio_info_get_channels g_mvideo_gst_discoverer_audio_info_get_channels = nullptr;
static mvideo_gst_discoverer_audio_info_get_depth g_mvideo_gst_discoverer_audio_info_get_depth = nullptr;

MovieInfo GstUtils::m_movieInfo = MovieInfo();
GstUtils* GstUtils::m_pGstUtils = new GstUtils;

GstUtils::GstUtils()
{
    QLibrary gstreamerLibrary(CompositingManager::libPath("libgstreamer-1.0.so"));
    QLibrary gstpbutilsLibrary(CompositingManager::libPath("libgstpbutils-1.0.so"));

    g_mvideo_gst_init = (mvideo_gst_init) gstreamerLibrary.resolve("gst_init");
    g_mvideo_gst_discoverer_new = (mvideo_gst_discoverer_new) gstpbutilsLibrary.resolve("gst_discoverer_new");
    g_mvideo_gst_discoverer_start = (mvideo_gst_discoverer_start) gstpbutilsLibrary.resolve("gst_discoverer_start");
    g_mvideo_gst_discoverer_stop = (mvideo_gst_discoverer_stop) gstpbutilsLibrary.resolve("gst_discoverer_stop");
    g_mvideo_gst_discoverer_discover_uri_async = (mvideo_gst_discoverer_discover_uri_async) gstpbutilsLibrary.resolve("gst_discoverer_discover_uri_async");

    g_mvideo_gst_discoverer_info_get_uri = (mvideo_gst_discoverer_info_get_uri) gstpbutilsLibrary.resolve("gst_discoverer_info_get_uri");
    g_mvideo_gst_discoverer_info_get_result = (mvideo_gst_discoverer_info_get_result) gstpbutilsLibrary.resolve("gst_discoverer_info_get_result");
    g_mvideo_gst_discoverer_info_get_misc = (mvideo_gst_discoverer_info_get_misc) gstpbutilsLibrary.resolve("gst_discoverer_info_get_misc");
    g_mvideo_gst_structure_to_string = (mvideo_gst_structure_to_string) gstreamerLibrary.resolve("gst_structure_to_string");
    g_mvideo_gst_discoverer_info_get_video_streams = (mvideo_gst_discoverer_info_get_video_streams) gstpbutilsLibrary.resolve("gst_discoverer_info_get_video_streams");
    g_mvideo_gst_discoverer_info_get_audio_streams = (mvideo_gst_discoverer_info_get_audio_streams) gstpbutilsLibrary.resolve("gst_discoverer_info_get_audio_streams");
    g_mvideo_gst_discoverer_video_info_get_width = (mvideo_gst_discoverer_video_info_get_width) gstpbutilsLibrary.resolve("gst_discoverer_video_info_get_width");
    g_mvideo_gst_discoverer_video_info_get_height = (mvideo_gst_discoverer_video_info_get_height) gstpbutilsLibrary.resolve("gst_discoverer_video_info_get_height");
    g_mvideo_gst_discoverer_audio_info_get_bitrate = (mvideo_gst_discoverer_audio_info_get_bitrate) gstpbutilsLibrary.resolve("gst_discoverer_audio_info_get_bitrate");
    g_mvideo_gst_discoverer_audio_info_get_channels = (mvideo_gst_discoverer_audio_info_get_channels) gstpbutilsLibrary.resolve("gst_discoverer_audio_info_get_channels");
    g_mvideo_gst_discoverer_audio_info_get_depth = (mvideo_gst_discoverer_audio_info_get_depth) gstpbutilsLibrary.resolve("gst_discoverer_audio_info_get_depth");
    g_mvideo_gst_discoverer_info_get_duration = (mvideo_gst_discoverer_info_get_duration) gstpbutilsLibrary.resolve("gst_discoverer_info_get_duration");
    g_mvideo_gst_discoverer_video_info_get_framerate_num = (mvideo_gst_discoverer_video_info_get_framerate_num) gstpbutilsLibrary.resolve("gst_discoverer_video_info_get_framerate_num");
    g_mvideo_gst_discoverer_video_info_get_framerate_denom = (mvideo_gst_discoverer_video_info_get_framerate_denom) gstpbutilsLibrary.resolve("gst_discoverer_video_info_get_framerate_denom");
    g_mvideo_gst_discoverer_video_info_get_bitrate = (mvideo_gst_discoverer_video_info_get_bitrate) gstpbutilsLibrary.resolve("gst_discoverer_video_info_get_bitrate");
    g_mvideo_gst_discoverer_audio_info_get_sample_rate = (mvideo_gst_discoverer_audio_info_get_sample_rate) gstpbutilsLibrary.resolve("gst_discoverer_audio_info_get_sample_rate");

    memset(&m_gstData, 0, sizeof(m_gstData));

    g_mvideo_gst_init(nullptr, nullptr);

    GError *pGErr = nullptr;
    m_gstData.discoverer = g_mvideo_gst_discoverer_new(5 * GST_SECOND, &pGErr);
    m_gstData.loop = g_main_loop_new (nullptr, FALSE);

    if (!m_gstData.discoverer) {
        qInfo() << "Error creating discoverer instance: " << pGErr->message;
        g_clear_error (&pGErr);
    }

    g_signal_connect_data (m_gstData.discoverer, "discovered", (GCallback)discovered, &m_gstData, nullptr, GConnectFlags(0));
    g_signal_connect_data (m_gstData.discoverer, "finished",  (GCallback)(finished), &m_gstData, nullptr, GConnectFlags(0));

    g_mvideo_gst_discoverer_start(m_gstData.discoverer);
}

void GstUtils::discovered(GstDiscoverer *discoverer, GstDiscovererInfo *info, GError *err, CustomData *data)
{
    Q_UNUSED(discoverer);
    Q_UNUSED(data);

    GstDiscovererResult result;
    const gchar *uri;

    uri = g_mvideo_gst_discoverer_info_get_uri (info);
    result = g_mvideo_gst_discoverer_info_get_result (info);

    m_movieInfo.valid = false;
    m_movieInfo.duration = 0;

    switch (result) {
      case GST_DISCOVERER_URI_INVALID:
        qInfo() << "Invalid URI " << uri;
        break;
      case GST_DISCOVERER_ERROR:
        qInfo() << "Discoverer error: " << err->message;
        break;
      case GST_DISCOVERER_TIMEOUT:
        qInfo() << "Timeout";
        break;
      case GST_DISCOVERER_BUSY:
        qInfo() << "Busy";
        break;
      case GST_DISCOVERER_MISSING_PLUGINS:{
        const GstStructure *s;
        gchar *str;

        s = g_mvideo_gst_discoverer_info_get_misc (info);
        str = g_mvideo_gst_structure_to_string (s);

        qInfo() << "Missing plugins: " << str;
        g_free (str);
        break;
      }
      case GST_DISCOVERER_OK:
        qInfo() << "Discovered " << uri;
        break;
    }

    if (result != GST_DISCOVERER_OK) {
      qInfo() << "This URI cannot be played";
      return;
    }

    m_movieInfo.valid = true;
    m_movieInfo.duration = g_mvideo_gst_discoverer_info_get_duration (info) / GST_SECOND;

    // 如果没有时长就当做原始视频格式处理
    if(m_movieInfo.duration == 0) {
#ifdef _MOVIE_USE_
        m_movieInfo.strFmtName = "raw";
#endif
    }

    GList *list;
    list = g_mvideo_gst_discoverer_info_get_video_streams(info);
    if (list)
    {
        GstDiscovererVideoInfo *vInfo = (GstDiscovererVideoInfo *)list->data;

        m_movieInfo.width = static_cast<int>(g_mvideo_gst_discoverer_video_info_get_width(vInfo));
        m_movieInfo.height = static_cast<int>(g_mvideo_gst_discoverer_video_info_get_height(vInfo));
        m_movieInfo.fps = static_cast<int>(g_mvideo_gst_discoverer_video_info_get_framerate_num(vInfo) / g_mvideo_gst_discoverer_video_info_get_framerate_denom(vInfo));
        m_movieInfo.vCodeRate = g_mvideo_gst_discoverer_video_info_get_bitrate(vInfo);
        m_movieInfo.proportion = m_movieInfo.height == 0 ? 0 : (float)m_movieInfo.width / m_movieInfo.height;
        m_movieInfo.resolution = QString::number(m_movieInfo.width) + "x" + QString::number(m_movieInfo.height);
    }

    list = g_mvideo_gst_discoverer_info_get_audio_streams(info);
    if (list)
    {
        GstDiscovererAudioInfo *aInfo = (GstDiscovererAudioInfo *)list->data;

        m_movieInfo.sampling = static_cast<int>(g_mvideo_gst_discoverer_audio_info_get_sample_rate(aInfo));
        m_movieInfo.aCodeRate = g_mvideo_gst_discoverer_audio_info_get_bitrate(aInfo);
        m_movieInfo.channels = static_cast<int>(g_mvideo_gst_discoverer_audio_info_get_channels(aInfo));
        m_movieInfo.aDigit = static_cast<int>(g_mvideo_gst_discoverer_audio_info_get_depth(aInfo));
    }
}

void GstUtils::finished(GstDiscoverer *discoverer, CustomData *data)
{
    Q_UNUSED(discoverer);

    g_main_loop_quit (data->loop);
}

GstUtils::~GstUtils()
{
    g_mvideo_gst_discoverer_stop(m_gstData.discoverer);
    g_object_unref(m_gstData.discoverer);
    g_main_loop_unref (m_gstData.loop);
}

GstUtils* GstUtils::get()
{
    if(!m_pGstUtils) {
        m_pGstUtils = new GstUtils();
    }

    return m_pGstUtils;
}

MovieInfo GstUtils:: parseFileByGst(const QFileInfo &fi)
{
    m_movieInfo = MovieInfo();

    m_movieInfo.title = fi.fileName();
    m_movieInfo.filePath = fi.canonicalFilePath();
    m_movieInfo.creation = fi.created().toString();
    m_movieInfo.fileSize = fi.size();
    m_movieInfo.fileType = fi.suffix();

    QByteArray uri = QUrl::fromLocalFile(fi.filePath()).toString().toLocal8Bit();
    if (!g_mvideo_gst_discoverer_discover_uri_async (m_gstData.discoverer, uri.constData())) {
      qInfo() << "Failed to start discovering URI " << uri;
      g_object_unref (m_gstData.discoverer);
      return m_movieInfo;
    }

    g_main_loop_run (m_gstData.loop);

    return m_movieInfo;
}

}

