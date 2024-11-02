/*
 * Copyright (c) 2012-2020 Maarten Baert <maarten-baert@hotmail.com>

 * This file is part of Lingmo-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef AVWRAPPER_H
#define AVWRAPPER_H

#include <cstdint>
#include <cstddef>
#include <exception>
#include <memory>

#include <QString>

#include "Global.h"
#include "my_av.h"

class AVFrameData
{
private:
    uint8_t *m_data;
    size_t m_size;
public:
    inline AVFrameData(size_t size) {
        m_data = (uint8_t*) av_malloc(size);
        if (m_data == NULL)
            throw std::bad_alloc();
        m_size = size;
    }
    inline ~AVFrameData() {
        av_free(m_data);
    }
    inline uint8_t* GetData() {return m_data; }
    inline size_t GetSize() {return m_size; }
};


class AVFrameWrapper
{
private:
    AVFrame *m_frame;
    std::shared_ptr<AVFrameData> m_refcounted_data;

public:
    AVFrameWrapper(const std::shared_ptr<AVFrameData>& refcounted_data);
    ~AVFrameWrapper();

    AVFrameWrapper(const AVFrameWrapper&) = delete;
    AVFrameWrapper& operator=(const AVFrameWrapper&) = delete;

#if SSR_USE_AVCODEC_SEND_RECEIVE
    AVFame* Release();
#endif

public:
    inline AVFrame* GetFrame() {return m_frame; }
    inline uint8_t* GetRawData() {return m_refcounted_data->GetData(); }
    inline std::shared_ptr<AVFrameData> GetFrameData() {return m_refcounted_data; }

};


class AVPacketWrapper
{
private:
    AVPacket *m_packet;
#if !SSR_USE_AV_PACKET_ALLOC
    bool m_free_on_destruct;
#endif

public:
    AVPacketWrapper();
    AVPacketWrapper(size_t size);
    ~AVPacketWrapper();

    AVPacketWrapper(const AVPacketWrapper&) = delete;
    AVPacketWrapper& operator=(const AVPacketWrapper&) = delete;

public:
    inline AVPacket* GetPacket() {return m_packet; }
    inline void SetFreeOnDestruct(bool free_on_destruct) {
#if !SSR_USE_AV_PACKET_ALLOC
    m_free_on_destruct = free_on_destruct;
#endif
    }

};


bool AVFormatIsInstalled(const QString& format_name);
bool AVCodecIsInstalled(const QString& codec_name);
bool AVCodecSupportsPixelFormat(const AVCodec* codec, AVPixelFormat pixel_fmt);
bool AVCodecSupportsSampleFormat(const AVCodec* codec, AVSampleFormat sample_fmt);

#if !SSR_USE_AV_CODEC_IS_ENCODER
inline int av_codec_is_encoder(const AVCodec* codec) {
    return (codec != NULL && (codec->encode != NULL || codec->encode2 != NULL));
}
#endif



#endif // AVWRAPPER_H
