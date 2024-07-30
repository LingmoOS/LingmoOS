/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MEDIATYPES_H
#define BLUEZQT_MEDIATYPES_H

namespace BluezQt
{
/** Assigned number of codec that the endpoint/transport supports.
    Currently limited to most common codecs. Further codecs reserved. */
enum class AudioCodec {
    Invalid = 0x0000,
    Sbc = 0x0001,
    // Mp3 = 0x0002,
    Aac = 0x0004,
    // Atrac = 0x0008,
    // AptX = 0x0010,
    // AptXLl = 0x0020,
    // AptXHd = 0x0040,
    // FastStream = 0x0080,
    // Ldac = 0x0100,
};

/** Assigned number of sample rate that the endpoint/transport supports.
    Currently limited to most common rates. Further rates reserved. */
enum class AudioSampleRate {
    Invalid = 0x0000,
    // Rate8000  = 0x0001,
    // Rate11025 = 0x0002,
    // Rate12000 = 0x0004,
    // Rate16000 = 0x0008,
    // Rate22050 = 0x0010,
    // Rate24000 = 0x0020,
    // Rate32000 = 0x0040,
    Rate44100 = 0x0080,
    Rate48000 = 0x0100,
    // Rate64000 = 0x0200,
    // Rate88200 = 0x0400,
    // Rate96000 = 0x0800
};

struct AudioConfiguration {
    AudioCodec codec = AudioCodec::Invalid;
    AudioSampleRate sampleRate = AudioSampleRate::Invalid;
};

} // namespace BluezQt

#endif
