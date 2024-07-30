/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pipewirerecord.h"
#include "encoder_p.h"
#include "glhelpers.h"
#include "pipewirerecord_p.h"
#include <logging_record.h>

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <qpa/qplatformnativeinterface.h>

#include <KShell>

#include <unistd.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
}

#undef av_err2str

#ifdef av_ts2str
#undef av_ts2str
char buf[AV_TS_MAX_STRING_SIZE];
#define av_ts2str(ts) av_ts_make_string(buf, ts)
#endif // av_ts2str

#ifdef av_ts2timestr
#undef av_ts2timestr
char timebuf[AV_TS_MAX_STRING_SIZE];
#define av_ts2timestr(ts, tb) av_ts_make_time_string(timebuf, ts, tb)
#endif // av_ts2timestr

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    qCDebug(PIPEWIRERECORD_LOGGING,
            "pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s "
            "stream_index:%d",
            av_ts2str(pkt->pts),
            av_ts2timestr(pkt->pts, time_base),
            av_ts2str(pkt->dts),
            av_ts2timestr(pkt->dts, time_base),
            av_ts2str(pkt->duration),
            av_ts2timestr(pkt->duration, time_base),
            pkt->stream_index);
}

PipeWireRecord::PipeWireRecord(QObject *parent)
    : PipeWireBaseEncodedStream(parent)
    , d(new PipeWireRecordPrivate)
{
}

PipeWireRecord::~PipeWireRecord() = default;

void PipeWireRecord::setOutput(const QString &_output)
{
    const QString output = KShell::tildeExpand(_output);

    if (d->m_output == output)
        return;

    d->m_output = output;
    refresh();
    Q_EMIT outputChanged(output);
}

QString PipeWireRecord::output() const
{
    return d->m_output;
}

QString PipeWireRecord::extension() const
{
    static QHash<PipeWireBaseEncodedStream::Encoder, QString> s_extensions = {
        {PipeWireBaseEncodedStream::H264Main, QStringLiteral("mp4")},
        {PipeWireBaseEncodedStream::H264Baseline, QStringLiteral("mp4")},
        {PipeWireBaseEncodedStream::VP8, QStringLiteral("webm")},
        {PipeWireBaseEncodedStream::VP9, QStringLiteral("webm")},
    };
    return s_extensions.value(encoder());
}

PipeWireRecordProduce::PipeWireRecordProduce(PipeWireBaseEncodedStream::Encoder encoder, uint nodeId, uint fd, const Fraction &framerate, const QString &output)
    : PipeWireProduce(encoder, nodeId, fd, framerate)
    , m_output(output)
{
}

bool PipeWireRecordProduce::setupFormat()
{
    avformat_alloc_output_context2(&m_avFormatContext, nullptr, nullptr, m_output.toUtf8().constData());
    if (!m_avFormatContext) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not deduce output format from file: using WebM." << m_output;
        avformat_alloc_output_context2(&m_avFormatContext, nullptr, "webm", m_output.toUtf8().constData());
    }
    if (!m_avFormatContext) {
        qCDebug(PIPEWIRERECORD_LOGGING) << "could not set stream up";
        return false;
    }

    const Fraction framerate = m_stream->framerate();
    int ret = avio_open(&m_avFormatContext->pb, QFile::encodeName(m_output).constData(), AVIO_FLAG_WRITE);
    if (ret < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Could not open" << m_output << av_err2str(ret);
        return false;
    }

    auto avStream = avformat_new_stream(m_avFormatContext, nullptr);
    avStream->start_time = 0;
    if (framerate) {
        avStream->r_frame_rate.num = framerate.numerator;
        avStream->r_frame_rate.den = framerate.denominator;
        avStream->avg_frame_rate.num = framerate.numerator;
        avStream->avg_frame_rate.den = framerate.denominator;
    }

    ret = avcodec_parameters_from_context(avStream->codecpar, m_encoder->avCodecContext());
    if (ret < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Error occurred when passing the codec:" << av_err2str(ret);
        return false;
    }

    ret = avformat_write_header(m_avFormatContext, nullptr);
    if (ret < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Error occurred when writing header:" << av_err2str(ret);
        return false;
    }

    return true;
}

void PipeWireRecordProduce::processFrame(const PipeWireFrame &frame)
{
    PipeWireProduce::processFrame(frame);
    if (frame.cursor && !frame.dmabuf && !frame.dataFrame && m_frameWithoutMetadataCursor.dataFrame) {
        m_encoder->filterFrame(m_frameWithoutMetadataCursor);
    }
}

void PipeWireRecordProduce::aboutToEncode(PipeWireFrame &frame)
{
    if (!frame.dataFrame) {
        return;
    }

    if (m_cursor.position && !m_cursor.texture.isNull()) {
        auto image = frame.dataFrame->toImage();
        // Do not copy the image if it's already ours
        if (m_frameWithoutMetadataCursor.dataFrame->cleanup != frame.dataFrame->cleanup) {
            m_frameWithoutMetadataCursor.dataFrame = frame.dataFrame->copy();
        }
        QPainter p(&image);
        p.drawImage(*m_cursor.position, m_cursor.texture);
    }
}

void PipeWireRecordProduce::processPacket(AVPacket *packet)
{
    packet->stream_index = (*m_avFormatContext->streams)->index;
    av_packet_rescale_ts(packet, m_encoder->avCodecContext()->time_base, (*m_avFormatContext->streams)->time_base);
    log_packet(m_avFormatContext, packet);
    auto ret = av_interleaved_write_frame(m_avFormatContext, packet);
    if (ret < 0) {
        qCWarning(PIPEWIRERECORD_LOGGING) << "Error while writing output packet:" << av_err2str(ret);
    }
}

std::unique_ptr<PipeWireProduce> PipeWireRecord::makeProduce()
{
    return std::make_unique<PipeWireRecordProduce>(encoder(), nodeId(), fd(), maxFramerate(), d->m_output);
}

int64_t PipeWireRecordProduce::framePts(const std::optional<std::chrono::nanoseconds> &presentationTimestamp)
{
    const auto current = std::chrono::duration_cast<std::chrono::milliseconds>(*presentationTimestamp).count();
    if ((*m_avFormatContext->streams)->start_time == 0) {
        (*m_avFormatContext->streams)->start_time = current;
    }

    return current - (*m_avFormatContext->streams)->start_time;
}

void PipeWireRecordProduce::cleanup()
{
    if (m_encoder) {
        // Clear the queue of encoded packets.
        m_encoder->receivePacket();
    }

    if (m_avFormatContext) {
        if (auto result = av_write_trailer(m_avFormatContext); result < 0) {
            qCWarning(PIPEWIRERECORD_LOGGING) << "Could not write trailer";
        }

        avio_closep(&m_avFormatContext->pb);
        avformat_free_context(m_avFormatContext);
    }
}

#include "moc_pipewirerecord.cpp"

#include "moc_pipewirerecord_p.cpp"
