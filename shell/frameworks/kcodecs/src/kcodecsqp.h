/*  -*- c++ -*-
    SPDX-FileCopyrightText: 2001-2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the @ref QuotedPrintable, @ref  RFC2047Q, and
  @ref RFC2231 @ref Codec classes.

  @brief
  Defines the classes QuotedPrintableCodec, Rfc2047QEncodingCodec, and
  Rfc2231EncodingCodec.

  @authors Marc Mutz \<mutz@kde.org\>

  @glossary @anchor QuotedPrintable @anchor quotedprintable @b quoted-printable:
  a binary to text encoding scheme based on Section 6.7 of @ref RFC2045.

  @glossary @anchor RFC2047Q @anchor rfc2047q @b RFC @b 2047Q:
  Section 4.2 of @ref RFC2047.

  @glossary @anchor RFC2231 @anchor rfc2231 @b RFC @b 2231:
  RFC that defines the <a href="http://tools.ietf.org/html/rfc2231">
  MIME Parameter Value and Encoded Word Extensions: Character Sets, Languages,
  and Continuations</a>.
*/

#ifndef KCODECS_QP_H
#define KCODECS_QP_H

#include "kcodecs.h"

namespace KCodecs
{
/**
  @brief
  A class representing the @ref codec for @ref QuotedPrintable as specified in
  @ref RFC2045 (section 6.7).
*/
class QuotedPrintableCodec : public Codec
{
public:
    /**
      Constructs a QuotedPrintable codec.
    */
    QuotedPrintableCodec()
        : Codec()
    {
    }

    /**
      Destroys the codec.
    */
    ~QuotedPrintableCodec() override
    {
    }

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const override
    {
        return "quoted-printable";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    qsizetype maxEncodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override
    {
        // all chars encoded:
        qsizetype result = 3 * insize;
        // then after 25 hexchars comes a soft linebreak: =(\r)\n
        result += (newline == Codec::NewlineCRLF ? 3 : 2) * (insize / 25);

        return result;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    qsizetype maxDecodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override;

    /**
      @copydoc
      Codec::makeEncoder()
    */
    Encoder *makeEncoder(NewlineType newline = Codec::NewlineLF) const override;

    /**
      @copydoc
      Codec::makeDecoder()
    */
    Decoder *makeDecoder(NewlineType newline = Codec::NewlineLF) const override;
};

/**
  @brief
  A class representing the @ref codec for the Q encoding as specified
  in @ref RFC2047Q.
*/
class Rfc2047QEncodingCodec : public Codec
{
public:
    /**
      Constructs a RFC2047Q codec.
    */
    Rfc2047QEncodingCodec()
        : Codec()
    {
    }

    /**
      Destroys the codec.
    */
    ~Rfc2047QEncodingCodec() override
    {
    }

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const override
    {
        return "q";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    qsizetype maxEncodedSizeFor(qsizetype insize, Codec::NewlineType newline = Codec::NewlineLF) const override
    {
        Q_UNUSED(newline);
        // this one is simple: We don't do linebreaking, so all that can
        // happen is that every char needs encoding, so:
        return 3 * insize;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    qsizetype maxDecodedSizeFor(qsizetype insize, Codec::NewlineType newline = Codec::NewlineLF) const override;

    /**
      @copydoc
      Codec::makeEncoder()
    */
    Encoder *makeEncoder(Codec::NewlineType newline = Codec::NewlineLF) const override;

    /**
      @copydoc
      Codec::makeDecoder()
    */
    Decoder *makeDecoder(Codec::NewlineType newline = Codec::NewlineLF) const override;
};

/**
  @brief
  A class representing the @ref codec for @ref RFC2231.
*/
class Rfc2231EncodingCodec : public Codec
{
public:
    /**
      Constructs a RFC2231 codec.
    */
    Rfc2231EncodingCodec()
        : Codec()
    {
    }

    /**
      Destroys the codec.
    */
    ~Rfc2231EncodingCodec() override
    {
    }

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const override
    {
        return "x-kmime-rfc2231";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    qsizetype maxEncodedSizeFor(qsizetype insize, Codec::NewlineType newline = Codec::NewlineLF) const override
    {
        Q_UNUSED(newline);
        // same as for "q" encoding:
        return 3 * insize;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    qsizetype maxDecodedSizeFor(qsizetype insize, Codec::NewlineType newline = Codec::NewlineLF) const override;

    /**
      @copydoc
      Codec::makeEncoder()
    */
    Encoder *makeEncoder(Codec::NewlineType newline = Codec::NewlineLF) const override;

    /**
      @copydoc
      Codec::makeDecoder()
    */
    Decoder *makeDecoder(Codec::NewlineType newline = Codec::NewlineLF) const override;
};

} // namespace KCodecs

#endif // KCODECS_QP_H
