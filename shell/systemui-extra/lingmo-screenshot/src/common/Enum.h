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
#ifndef ENUM_H
#define ENUM_H

namespace ssr {

enum class enum_video_area {
    VIDEO_AREA_SCREEN,
    VIDEO_AREA_FIXED,
    VIDEO_AREA_CURSOR,
    VIDEO_AREA_COUNT

};

//output
enum class enum_container {
    CONTAINER_MKV,
    CONTAINER_MP4,
    CONTAINER_WEBM,
    CONTAINER_OGG,
    CONTAINER_OTHER,
    CONTAINER_COUNT // must be last

};
enum class enum_video_codec {
    VIDEO_CODEC_H264,
    VIDEO_CODEC_VP8,
    VIDEO_CODEC_THEORA,
    VIDEO_CODEC_OTHER,
    VIDEO_CODEC_COUNT // must be last
};
enum class enum_audio_codec {
    AUDIO_CODEC_VORBIS,
    AUDIO_CODEC_MP3,
    AUDIO_CODEC_AAC,
    AUDIO_CODEC_UNCOMPRESSED,
    AUDIO_CODEC_OTHER,
    AUDIO_CODEC_COUNT // must be last
};

enum enum_audio_backend {
#if SSR_USE_ALSA
    AUDIO_BACKEND_ALSA,
#endif
#if SSR_USE_PULSEAUDIO
    AUDIO_BACKEND_PULSEAUDIO,
#endif
#if SSR_USE_JACK
    AUDIO_BACKEND_JACK,
#endif
    AUDIO_BACKEND_COUNT // must be last
};

}
#endif // ENUM_H
