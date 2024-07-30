/*
    SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCODECS_P_H
#define KCODECS_P_H

#include "kcodecs.h"

namespace KCodecs
{
class EncoderPrivate
{
public:
    explicit EncoderPrivate(Codec::NewlineType newline);

    /**
      An output buffer to simplify some codecs.
      Used with write() and flushOutputBuffer().
    */
    char outputBuffer[Encoder::maxBufferedChars];

    uchar outputBufferCursor;
    const Codec::NewlineType newline;
};

class DecoderPrivate
{
public:
    explicit DecoderPrivate(Codec::NewlineType newline);

    const Codec::NewlineType newline;
};

}

#endif // KCODECS_P_H
