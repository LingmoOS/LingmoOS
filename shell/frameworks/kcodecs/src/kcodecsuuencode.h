/*  -*- c++ -*-
    SPDX-FileCopyrightText: 2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines a @ref uuencode @ref Codec class.

  @brief
  Defines the UUCodec class.

  @authors Marc Mutz \<mutz@kde.org\>

  @glossary @anchor UUEncode @anchor uuencode @b uuencode:
  a binary to text encoding scheme. For more information, see the
  <a href="http://en.wikipedia.org/wiki/Uuencode"> Wikipedia Uuencode page</a>.
*/

#ifndef KCODECS_UUENCODE_H
#define KCODECS_UUENCODE_H

#include "kcodecs.h"

namespace KCodecs
{
/**
  @brief
  A class representing the @ref UUEncode @ref codec.
*/
class UUCodec : public Codec
{
public:
    /**
      Constructs a UUEncode codec.
    */
    UUCodec()
        : Codec()
    {
    }

    /**
      Destroys the codec.
    */
    ~UUCodec() override
    {
    }

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const override
    {
        return "x-uuencode";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    qsizetype maxEncodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override
    {
        Q_UNUSED(newline);
        return insize; // we have no encoder!
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    qsizetype maxDecodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override
    {
        // assuming all characters are part of the uuencode stream (which
        // does almost never hold due to required linebreaking; but
        // additional non-uu chars don't affect the output size), each
        // 4-tupel of them becomes a 3-tupel in the decoded octet
        // stream. So:
        qsizetype result = ((insize + 3) / 4) * 3;
        // but all of them may be \n, so
        if (newline == Codec::NewlineCRLF) {
            result *= 2; // :-o
        }
        return result;
    }

    /**
      @copydoc
      Codec::makeEncoder()
    */
    Encoder *makeEncoder(NewlineType newline = Codec::NewlineLF) const override;

    /**
      @copydoc
      Codec::makeEncoder()
    */
    Decoder *makeDecoder(NewlineType newline = Codec::NewlineLF) const override;
};

} // namespace KCodecs

#endif // KCODECS_UUENCODE_H
