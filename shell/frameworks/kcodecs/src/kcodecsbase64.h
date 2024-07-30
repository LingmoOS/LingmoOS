/*  -*- c++ -*-
    SPDX-FileCopyrightText: 2001-2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling @ref MIME data and
  defines the @ref Base64 and @ref RFC2047B @ref Codec classes.

  @brief
  Defines the Base64Codec and Rfc2047BEncodingCodec classes.

  @authors Marc Mutz \<mutz@kde.org\>

  @glossary @anchor Base64 @anchor base64 @b base64:
  a binary to text encoding scheme based on @ref RFC1421.

  @glossary @anchor RFC1421 @anchor rfc1421 @b RFC @b 1421:
  RFC that defines the <a href="http://tools.ietf.org/html/rfc1421">
  Privacy Enhancement for Internet Electronic Mail:  Part I:
  Message Encryption and Authentication Procedures</a>.

  @glossary @anchor RFC2045 @anchor rfc2045 @b RFC @b 2045:
  RFC that defines the <a href="http://tools.ietf.org/html/rfc2045">
  MIME Part One: Format of Internet Message Bodies</a>.

  @glossary @anchor RFC2047 @anchor rfc2047 @b RFC @b 2047:
  RFC that defines the <a href="http://tools.ietf.org/html/rfc2047">
  MIME Part Three: Message Header Extensions for Non-ASCII Text</a>.

  @glossary @anchor RFC2047B @anchor rfc2047b @b RFC @b 2047B:
  Section 4.1 of @ref RFC2047.
*/

#ifndef KCODECS_BASE64_H
#define KCODECS_BASE64_H

#include "kcodecs.h"

namespace KCodecs
{
/**
  @brief
  A class representing the @ref codec for @ref Base64 as specified in
  @ref RFC2045
*/
class Base64Codec : public Codec
{
public:
    /**
      Constructs a Base64 codec.
    */
    Base64Codec()
        : Codec()
    {
    }

    /**
      Destroys the codec.
    */
    ~Base64Codec() override
    {
    }

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const override
    {
        return "base64";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    qsizetype maxEncodedSizeFor(qsizetype insize, NewlineType newline) const override
    {
        // first, the total number of 4-char packets will be:
        qsizetype totalNumPackets = (insize + 2) / 3;
        // now, after every 76/4'th packet there needs to be a linebreak:
        qsizetype numLineBreaks = totalNumPackets / (76 / 4);
        // and at the very end, too:
        ++numLineBreaks;
        // putting it all together, we have:
        return 4 * totalNumPackets + (newline == Codec::NewlineCRLF ? 2 : 1) * numLineBreaks;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    qsizetype maxDecodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override
    {
        // assuming all characters are part of the base64 stream (which
        // does almost never hold due to required linebreaking; but
        // additional non-base64 chars don't affect the output size), each
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
      Codec::makeDecoder()
    */
    Decoder *makeDecoder(NewlineType newline = Codec::NewlineLF) const override;
};

/**
  @brief
  A class representing the @ref codec for the B encoding as specified
  in @ref RFC2047B.
*/
class Rfc2047BEncodingCodec : public Base64Codec
{
public:
    /**
      Constructs a RFC2047B codec.
    */
    Rfc2047BEncodingCodec()
        : Base64Codec()
    {
    }

    /**
      Destroys the codec.
    */
    ~Rfc2047BEncodingCodec() override
    {
    }

    /**
      @copydoc
      Codec::name()
    */
    const char *name() const override
    {
        return "b";
    }

    /**
      @copydoc
      Codec::maxEncodedSizeFor()
    */
    qsizetype maxEncodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override
    {
        Q_UNUSED(newline);
        // Each (begun) 3-octet triple becomes a 4 char quartet, so:
        return ((insize + 2) / 3) * 4;
    }

    /**
      @copydoc
      Codec::maxDecodedSizeFor()
    */
    qsizetype maxDecodedSizeFor(qsizetype insize, NewlineType newline = Codec::NewlineLF) const override
    {
        Q_UNUSED(newline);
        // Each 4-char quartet becomes a 3-octet triple, the last one
        // possibly even less. So:
        return ((insize + 3) / 4) * 3;
    }

    /**
      @copydoc
      Codec::makeEncoder()
    */
    Encoder *makeEncoder(NewlineType newline = Codec::NewlineLF) const override;
};

} // namespace KCodecs

#endif // KCODECS_BASE64_H
