// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avlibinterface.h"
#include <QDebug>
#include <QLibraryInfo>
#include <QDir>

avlibInterface::p_av_gettime avlibInterface::m_av_gettime = nullptr; // libavutil
avlibInterface::p_av_frame_alloc avlibInterface::m_av_frame_alloc = nullptr;
avlibInterface::p_av_frame_free avlibInterface::m_av_frame_free = nullptr;
avlibInterface::p_av_get_default_channel_layout avlibInterface::m_av_get_default_channel_layout = nullptr;
avlibInterface::p_av_get_sample_fmt_name avlibInterface::m_av_get_sample_fmt_name = nullptr;
avlibInterface::p_av_strdup avlibInterface::m_av_strdup = nullptr;
avlibInterface::p_av_get_channel_layout_string avlibInterface::m_av_get_channel_layout_string = nullptr;
avlibInterface::p_av_get_channel_layout_nb_channels avlibInterface::m_av_get_channel_layout_nb_channels = nullptr;
avlibInterface::p_av_audio_fifo_read avlibInterface::m_av_audio_fifo_read = nullptr;
avlibInterface::p_av_audio_fifo_write avlibInterface::m_av_audio_fifo_write = nullptr;
avlibInterface::p_av_frame_get_buffer avlibInterface::m_av_frame_get_buffer = nullptr;
avlibInterface::p_av_frame_apply_cropping avlibInterface::m_av_frame_apply_cropping = nullptr;
avlibInterface::p_av_frame_unref avlibInterface::m_av_frame_unref = nullptr;
avlibInterface::p_av_free avlibInterface::m_av_free = nullptr;
avlibInterface::p_av_audio_fifo_space avlibInterface::m_av_audio_fifo_space = nullptr;
avlibInterface::p_av_audio_fifo_size avlibInterface::m_av_audio_fifo_size = nullptr;
avlibInterface::p_av_audio_fifo_realloc avlibInterface::m_av_audio_fifo_realloc = nullptr;
avlibInterface::p_av_audio_fifo_alloc avlibInterface::m_av_audio_fifo_alloc = nullptr;
avlibInterface::p_av_samples_alloc avlibInterface::m_av_samples_alloc = nullptr;
avlibInterface::p_av_rescale_q avlibInterface::m_av_rescale_q = nullptr;
avlibInterface::p_av_rescale_q_rnd avlibInterface::m_av_rescale_q_rnd = nullptr;
avlibInterface::p_av_dict_set avlibInterface::m_av_dict_set = nullptr;
avlibInterface::p_av_freep avlibInterface::m_av_freep = nullptr;
avlibInterface::p_av_audio_fifo_free avlibInterface::m_av_audio_fifo_free = nullptr;
avlibInterface::p_av_malloc avlibInterface::m_av_malloc = nullptr;
avlibInterface::p_av_opt_set_bin avlibInterface::m_av_opt_set_bin = nullptr;
avlibInterface::p_av_int_list_length_for_size avlibInterface::m_av_int_list_length_for_size = nullptr;

avlibInterface::p_avcodec_register_all avlibInterface::m_avcodec_register_all = nullptr; //libavcodec
avlibInterface::p_av_init_packet avlibInterface::m_av_init_packet = nullptr;
avlibInterface::p_avcodec_decode_audio4 avlibInterface::m_avcodec_decode_audio4 = nullptr;
avlibInterface::p_av_packet_unref avlibInterface::m_av_packet_unref = nullptr;
avlibInterface::p_avcodec_open2 avlibInterface::m_avcodec_open2 = nullptr;
avlibInterface::p_avcodec_find_decoder avlibInterface::m_avcodec_find_decoder = nullptr;
avlibInterface::p_avcodec_alloc_context3 avlibInterface::m_avcodec_alloc_context3 = nullptr;
avlibInterface::p_avcodec_encode_video2 avlibInterface::m_avcodec_encode_video2 = nullptr;
avlibInterface::p_av_free_packet avlibInterface::m_av_free_packet = nullptr;
avlibInterface::p_avcodec_encode_audio2 avlibInterface::m_avcodec_encode_audio2 = nullptr;
avlibInterface::p_avcodec_close avlibInterface::m_avcodec_close = nullptr;
avlibInterface::p_avcodec_find_encoder avlibInterface::m_avcodec_find_encoder = nullptr;
avlibInterface::p_avpicture_get_size avlibInterface::m_avpicture_get_size = nullptr;
avlibInterface::p_avpicture_fill avlibInterface::m_avpicture_fill = nullptr;

avlibInterface::p_av_register_all avlibInterface::m_av_register_all = nullptr; // libavformat
avlibInterface::p_av_read_frame avlibInterface::m_av_read_frame = nullptr;
avlibInterface::p_avformat_close_input avlibInterface::m_avformat_close_input = nullptr;
avlibInterface::p_avformat_free_context avlibInterface::m_avformat_free_context = nullptr;
avlibInterface::p_av_find_input_format avlibInterface::m_av_find_input_format = nullptr;
avlibInterface::p_avformat_open_input avlibInterface::m_avformat_open_input = nullptr;
avlibInterface::p_avformat_find_stream_info avlibInterface::m_avformat_find_stream_info = nullptr;
avlibInterface::p_av_dump_format avlibInterface::m_av_dump_format = nullptr;
avlibInterface::p_avformat_new_stream avlibInterface::m_avformat_new_stream = nullptr;
avlibInterface::p_av_interleaved_write_frame avlibInterface::m_av_interleaved_write_frame = nullptr;
avlibInterface::p_av_write_trailer avlibInterface::m_av_write_trailer = nullptr;
avlibInterface::p_avio_close avlibInterface::m_avio_close = nullptr;
avlibInterface::p_avformat_write_header avlibInterface::m_avformat_write_header = nullptr;
avlibInterface::p_avio_open avlibInterface::m_avio_open = nullptr;
avlibInterface::p_avformat_alloc_output_context2 avlibInterface::m_avformat_alloc_output_context2 = nullptr;

avlibInterface::p_avdevice_register_all avlibInterface::m_avdevice_register_all = nullptr;//libavdevice

avlibInterface::p_avfilter_get_by_name avlibInterface::m_avfilter_get_by_name = nullptr; // libavfilter
avlibInterface::p_avfilter_inout_alloc avlibInterface::m_avfilter_inout_alloc = nullptr;
avlibInterface::p_avfilter_graph_alloc avlibInterface::m_avfilter_graph_alloc = nullptr;
avlibInterface::p_avfilter_graph_create_filter avlibInterface::m_avfilter_graph_create_filter = nullptr;
avlibInterface::p_avfilter_graph_parse_ptr avlibInterface::m_avfilter_graph_parse_ptr = nullptr;
avlibInterface::p_avfilter_graph_config avlibInterface::m_avfilter_graph_config = nullptr;
avlibInterface::p_avfilter_inout_free avlibInterface::m_avfilter_inout_free = nullptr;
avlibInterface::p_avfilter_graph_free avlibInterface::m_avfilter_graph_free = nullptr;
avlibInterface::p_av_buffersrc_add_frame_flags avlibInterface::m_av_buffersrc_add_frame_flags = nullptr;
avlibInterface::p_av_buffersink_get_frame avlibInterface::m_av_buffersink_get_frame = nullptr;

avlibInterface::p_sws_scale avlibInterface::m_sws_scale = nullptr;
avlibInterface::p_sws_getContext avlibInterface::m_sws_getContext = nullptr;

avlibInterface::p_swr_convert avlibInterface::m_swr_convert = nullptr;
avlibInterface::p_swr_alloc_set_opts avlibInterface::m_swr_alloc_set_opts = nullptr;
avlibInterface::p_swr_init avlibInterface::m_swr_init = nullptr;
avlibInterface::p_swr_free avlibInterface::m_swr_free = nullptr; //新增，解决内存泄露

QLibrary avlibInterface::m_libavutil;
QLibrary avlibInterface::m_libavcodec;
QLibrary avlibInterface::m_libavformat;
QLibrary avlibInterface::m_libavfilter;
QLibrary avlibInterface::m_libswscale;
QLibrary avlibInterface::m_libswresample;
QLibrary avlibInterface::m_libavdevice;

bool avlibInterface::m_isInitFunction = false;


avlibInterface::avlibInterface()
{

}

QString avlibInterface::libPath(const QString &sLib)
{
    QDir dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    qDebug() <<  " where is libs? that is " << dir ;
    QStringList list = dir.entryList(QStringList() << (sLib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    if (list.isEmpty()) {
        qWarning() << dir <<  "has not any lib with " << (sLib + "*") << ",so the list is empty!";
    }
    if (list.contains(sLib)) {
        return sLib;
    } else {
        list.sort();
    }

    //Q_ASSERT(list.size() > 0);
    return list.last();
}

void avlibInterface::initFunctions()
{
    if (m_isInitFunction)
        return;

    m_libavformat.setFileName(libPath("libavformat.so"));
    m_libavfilter.setFileName(libPath("libavfilter.so"));
    m_libswscale.setFileName(libPath("libswscale.so"));
    m_libswresample.setFileName(libPath("libswresample.so"));
    m_libavdevice.setFileName(libPath("libavdevice.so"));
    m_libavutil.setFileName(libPath("libavutil.so"));
    m_libavcodec.setFileName(libPath("libavcodec.so"));

    qDebug() << m_libavutil.load();
    qDebug() << m_libavcodec.load();
    qDebug() << m_libavformat.load();
    qDebug() << m_libavfilter.load();
    qDebug() << m_libswscale.load();
    qDebug() << m_libswresample.load();
    qDebug() << m_libavdevice.load();

    m_av_gettime = reinterpret_cast<p_av_gettime>(m_libavutil.resolve("av_gettime")); // libavutil
    m_av_frame_alloc = reinterpret_cast<p_av_frame_alloc>(m_libavutil.resolve("av_frame_alloc"));
    m_av_frame_free = reinterpret_cast<p_av_frame_free>(m_libavutil.resolve("av_frame_free"));
    m_av_get_default_channel_layout = reinterpret_cast<p_av_get_default_channel_layout>(m_libavutil.resolve("av_get_default_channel_layout"));
    m_av_get_sample_fmt_name = reinterpret_cast<p_av_get_sample_fmt_name>(m_libavutil.resolve("av_get_sample_fmt_name"));
    m_av_strdup = reinterpret_cast<p_av_strdup>(m_libavutil.resolve("av_strdup"));
    m_av_get_channel_layout_string = reinterpret_cast<p_av_get_channel_layout_string>(m_libavutil.resolve("av_get_channel_layout_string"));
    m_av_get_channel_layout_nb_channels = reinterpret_cast<p_av_get_channel_layout_nb_channels>(m_libavutil.resolve("av_get_channel_layout_nb_channels"));
    m_av_audio_fifo_read = reinterpret_cast<p_av_audio_fifo_read>(m_libavutil.resolve("av_audio_fifo_read"));
    m_av_audio_fifo_write = reinterpret_cast<p_av_audio_fifo_write>(m_libavutil.resolve("av_audio_fifo_write"));
    m_av_frame_get_buffer = reinterpret_cast<p_av_frame_get_buffer>(m_libavutil.resolve("av_frame_get_buffer"));
    m_av_frame_apply_cropping = reinterpret_cast<p_av_frame_apply_cropping>(m_libavutil.resolve("av_frame_apply_cropping"));
    m_av_frame_unref = reinterpret_cast<p_av_frame_unref>(m_libavutil.resolve("av_frame_unref"));
    m_av_free = reinterpret_cast<p_av_free>(m_libavutil.resolve("av_free"));
    m_av_audio_fifo_space = reinterpret_cast<p_av_audio_fifo_space>(m_libavutil.resolve("av_audio_fifo_space"));
    m_av_audio_fifo_size = reinterpret_cast<p_av_audio_fifo_size>(m_libavutil.resolve("av_audio_fifo_size"));
    m_av_audio_fifo_realloc = reinterpret_cast<p_av_audio_fifo_realloc>(m_libavutil.resolve("av_audio_fifo_realloc"));
    m_av_audio_fifo_alloc = reinterpret_cast<p_av_audio_fifo_alloc>(m_libavutil.resolve("av_audio_fifo_alloc"));
    m_av_samples_alloc = reinterpret_cast<p_av_samples_alloc>(m_libavutil.resolve("av_samples_alloc"));
    m_av_rescale_q = reinterpret_cast<p_av_rescale_q>(m_libavutil.resolve("av_rescale_q"));
    m_av_rescale_q_rnd = reinterpret_cast<p_av_rescale_q_rnd>(m_libavutil.resolve("av_rescale_q_rnd"));
    m_av_dict_set = reinterpret_cast<p_av_dict_set>(m_libavutil.resolve("av_dict_set"));
    m_av_freep = reinterpret_cast<p_av_freep>(m_libavutil.resolve("av_freep"));
    m_av_audio_fifo_free = reinterpret_cast<p_av_audio_fifo_free>(m_libavutil.resolve("av_audio_fifo_free"));
    m_av_malloc = reinterpret_cast<p_av_malloc>(m_libavutil.resolve("av_malloc"));
    m_av_opt_set_bin = reinterpret_cast<p_av_opt_set_bin>(m_libavutil.resolve("av_opt_set_bin"));
    m_av_int_list_length_for_size = reinterpret_cast<p_av_int_list_length_for_size>(m_libavutil.resolve("av_int_list_length_for_size"));


    m_avcodec_register_all = reinterpret_cast<p_avcodec_register_all>(m_libavcodec.resolve("avcodec_register_all")); //libavcodec
    m_av_init_packet = reinterpret_cast<p_av_init_packet>(m_libavcodec.resolve("av_init_packet"));
    m_avcodec_decode_audio4 = reinterpret_cast<p_avcodec_decode_audio4>(m_libavcodec.resolve("avcodec_decode_audio4"));
    m_av_packet_unref = reinterpret_cast<p_av_packet_unref>(m_libavcodec.resolve("av_packet_unref"));
    m_avcodec_open2 = reinterpret_cast<p_avcodec_open2>(m_libavcodec.resolve("avcodec_open2"));
    m_avcodec_find_decoder = reinterpret_cast<p_avcodec_find_decoder>(m_libavcodec.resolve("avcodec_find_decoder"));
    m_avcodec_alloc_context3 = reinterpret_cast<p_avcodec_alloc_context3>(m_libavcodec.resolve("avcodec_alloc_context3"));
    m_avcodec_encode_video2 = reinterpret_cast<p_avcodec_encode_video2>(m_libavcodec.resolve("avcodec_encode_video2"));
    m_av_free_packet = reinterpret_cast<p_av_free_packet>(m_libavcodec.resolve("av_free_packet"));
    m_avcodec_encode_audio2 = reinterpret_cast<p_avcodec_encode_audio2>(m_libavcodec.resolve("avcodec_encode_audio2"));
    m_avcodec_close = reinterpret_cast<p_avcodec_close>(m_libavcodec.resolve("avcodec_close"));
    m_avcodec_find_encoder = reinterpret_cast<p_avcodec_find_encoder>(m_libavcodec.resolve("avcodec_find_encoder"));
    m_avpicture_get_size = reinterpret_cast<p_avpicture_get_size>(m_libavcodec.resolve("avpicture_get_size"));
    m_avpicture_fill = reinterpret_cast<p_avpicture_fill>(m_libavcodec.resolve("avpicture_fill"));

    m_av_register_all = reinterpret_cast<p_av_register_all>(m_libavformat.resolve("av_register_all")); // libavformat
    m_av_read_frame = reinterpret_cast<p_av_read_frame>(m_libavformat.resolve("av_read_frame"));
    m_avformat_close_input = reinterpret_cast<p_avformat_close_input>(m_libavformat.resolve("avformat_close_input"));
    m_avformat_free_context = reinterpret_cast<p_avformat_free_context>(m_libavformat.resolve("avformat_free_context"));
    m_av_find_input_format = reinterpret_cast<p_av_find_input_format>(m_libavformat.resolve("av_find_input_format"));
    m_avformat_open_input = reinterpret_cast<p_avformat_open_input>(m_libavformat.resolve("avformat_open_input"));
    m_avformat_find_stream_info = reinterpret_cast<p_avformat_find_stream_info>(m_libavformat.resolve("avformat_find_stream_info"));
    m_av_dump_format = reinterpret_cast<p_av_dump_format>(m_libavformat.resolve("av_dump_format"));
    m_avformat_new_stream = reinterpret_cast<p_avformat_new_stream>(m_libavformat.resolve("avformat_new_stream"));
    m_av_interleaved_write_frame = reinterpret_cast<p_av_interleaved_write_frame>(m_libavformat.resolve("av_interleaved_write_frame"));
    m_av_write_trailer = reinterpret_cast<p_av_write_trailer>(m_libavformat.resolve("av_write_trailer"));
    m_avio_close = reinterpret_cast<p_avio_close>(m_libavformat.resolve("avio_close"));
    m_avformat_write_header = reinterpret_cast<p_avformat_write_header>(m_libavformat.resolve("avformat_write_header"));
    m_avio_open = reinterpret_cast<p_avio_open>(m_libavformat.resolve("avio_open"));
    m_avformat_alloc_output_context2 = reinterpret_cast<p_avformat_alloc_output_context2>(m_libavformat.resolve("avformat_alloc_output_context2"));


    m_avdevice_register_all = reinterpret_cast<p_avdevice_register_all>(m_libavdevice.resolve("avdevice_register_all"));//libavdevice

    m_avfilter_get_by_name = reinterpret_cast<p_avfilter_get_by_name>(m_libavfilter.resolve("avfilter_get_by_name")); // libavfilter
    m_avfilter_inout_alloc = reinterpret_cast<p_avfilter_inout_alloc>(m_libavfilter.resolve("avfilter_inout_alloc"));
    m_avfilter_graph_alloc = reinterpret_cast<p_avfilter_graph_alloc>(m_libavfilter.resolve("avfilter_graph_alloc"));
    m_avfilter_graph_create_filter = reinterpret_cast<p_avfilter_graph_create_filter>(m_libavfilter.resolve("avfilter_graph_create_filter"));
    m_avfilter_graph_parse_ptr = reinterpret_cast<p_avfilter_graph_parse_ptr>(m_libavfilter.resolve("avfilter_graph_parse_ptr"));
    m_avfilter_graph_config = reinterpret_cast<p_avfilter_graph_config>(m_libavfilter.resolve("avfilter_graph_config"));
    m_avfilter_inout_free = reinterpret_cast<p_avfilter_inout_free>(m_libavfilter.resolve("avfilter_inout_free"));
    m_avfilter_graph_free = reinterpret_cast<p_avfilter_graph_free>(m_libavfilter.resolve("avfilter_graph_free"));
    m_av_buffersrc_add_frame_flags = reinterpret_cast<p_av_buffersrc_add_frame_flags>(m_libavfilter.resolve("av_buffersrc_add_frame_flags"));
    m_av_buffersink_get_frame = reinterpret_cast<p_av_buffersink_get_frame>(m_libavfilter.resolve("av_buffersink_get_frame"));

    m_sws_scale = reinterpret_cast<p_sws_scale>(m_libswscale.resolve("sws_scale"));
    m_sws_getContext = reinterpret_cast<p_sws_getContext>(m_libswscale.resolve("sws_getContext"));

    m_swr_convert = reinterpret_cast<p_swr_convert>(m_libswresample.resolve("swr_convert"));
    m_swr_alloc_set_opts = reinterpret_cast<p_swr_alloc_set_opts>(m_libswresample.resolve("swr_alloc_set_opts"));;
    m_swr_init = reinterpret_cast<p_swr_init>(m_libswresample.resolve("swr_init"));
    m_swr_free = reinterpret_cast<p_swr_free>(m_libswresample.resolve("swr_free")); //新增，解决内存泄露

    m_isInitFunction = true;
}

void avlibInterface::unloadFunctions()
{
    if (m_isInitFunction) {
        m_libavutil.unload();
        m_libavcodec.unload();
        m_libavformat.unload();
        m_libavfilter.unload();
        m_libswscale.unload();
        m_libswresample.unload();
        m_libavdevice.unload();
    }
}
