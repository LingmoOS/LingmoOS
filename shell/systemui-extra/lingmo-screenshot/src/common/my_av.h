/*
 *
 * Copyright:  2020 KylinSoft Co., Ltd.
 * Authors:
 *   wenjie bi <biwenjie@cs2c.com.cn>
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 */
#pragma once

extern "C" {
#include <libavutil/rational.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/pixfmt.h>
#include <libavutil/samplefmt.h>
}
#include <exception>

// convert weird types from libav/ffmpeg to doubles
inline double ToDouble(const AVRational &r)
{
    return (double)r.num / (double)r.den;
}

class LibavException : public std::exception
{
public:
    inline virtual const char *what() const throw() override
    {
        return "LibavException";
    }
};
class ResamplerException : public std::exception
{
public:
    inline virtual const char *what() const throw() override
    {
        return "ResamplerException";
    }
};

#if SSR_USE_PULSEAUDIO
class PulseAudioException : public std::exception
{
public:
    inline virtual const char *what() const throw() override
    {
        return "PulseAudioException";
    }
};
#endif
