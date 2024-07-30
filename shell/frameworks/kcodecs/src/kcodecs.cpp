/*
    SPDX-FileCopyrightText: 2000-2001 Dawit Alemayehu <adawit@kde.org>
    SPDX-FileCopyrightText: 2001 Rik Hemsley (rikkus) <rik@kde.org>
    SPDX-FileCopyrightText: 2001-2002 Marc Mutz <mutz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only

    The encoding and decoding utilities in KCodecs with the exception of
    quoted-printable are based on the java implementation in HTTPClient
    package by Ronald Tschalär Copyright (C) 1996-1999.                          // krazy:exclude=copyright

    The quoted-printable codec as described in RFC 2045, section 6.7. is by
    Rik Hemsley (C) 2001.
*/

#include "kcodecs.h"
#include "kcharsets.h"
#include "kcharsets_p.h"
#include "kcodecs_debug.h"
#include "kcodecs_p.h"
#include "kcodecsbase64.h"
#include "kcodecsqp.h"
#include "kcodecsuuencode.h"

#include <array>
#include <cassert>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QDebug>
#include <QStringDecoder>
#include <QStringEncoder>

#if defined(Q_OS_WIN)
#define strncasecmp _strnicmp
#endif

namespace KCodecs
{
static QList<QByteArray> charsetCache;

QByteArray cachedCharset(const QByteArray &name)
{
    auto it = std::find_if(charsetCache.cbegin(), charsetCache.cend(), [&name](const QByteArray &charset) {
        return qstricmp(name.data(), charset.data()) == 0;
    });
    if (it != charsetCache.cend()) {
        return *it;
    }

    charsetCache.append(name.toUpper());
    return charsetCache.last();
}

namespace CodecNames
{
QByteArray utf8()
{
    return QByteArrayLiteral("UTF-8");
}
}

Q_REQUIRED_RESULT
QByteArray updateEncodingCharset(const QByteArray &currentCharset, const QByteArray &nextCharset)
{
    if (!nextCharset.isEmpty()) {
        if (currentCharset.isEmpty()) {
            return nextCharset;
        }
        if (currentCharset != nextCharset) {
            // only one charset per string supported, so change to superset charset UTF-8,
            // which should  cover any possible chars
            return CodecNames::utf8();
        }
    }
    return currentCharset;
}

} // namespace KCodecs

/******************************** KCodecs ********************************/

QByteArray KCodecs::quotedPrintableEncode(QByteArrayView in, bool useCRLF)
{
    Codec *codec = Codec::codecForName("quoted-printable");
    return codec->encode(in, useCRLF ? Codec::NewlineCRLF : Codec::NewlineLF);
}

void KCodecs::quotedPrintableEncode(QByteArrayView in, QByteArray &out, bool useCRLF)
{
    out = quotedPrintableEncode(in, useCRLF ? Codec::NewlineCRLF : Codec::NewlineLF);
}

QByteArray KCodecs::quotedPrintableDecode(QByteArrayView in)
{
    Codec *codec = Codec::codecForName("quoted-printable");
    return codec->decode(in);
}

void KCodecs::quotedPrintableDecode(QByteArrayView in, QByteArray &out)
{
    out = quotedPrintableDecode(in);
}

QByteArray KCodecs::base64Encode(QByteArrayView in)
{
    Codec *codec = Codec::codecForName("base64");
    return codec->encode(in);
}

void KCodecs::base64Encode(QByteArrayView in, QByteArray &out, bool insertLFs)
{
    Q_UNUSED(insertLFs);
    out = base64Encode(in);
}

QByteArray KCodecs::base64Decode(QByteArrayView in)
{
    Codec *codec = Codec::codecForName("base64");
    return codec->decode(in);
}

void KCodecs::base64Decode(const QByteArrayView in, QByteArray &out)
{
    out = base64Decode(in);
}

QByteArray KCodecs::uudecode(QByteArrayView in)
{
    Codec *codec = Codec::codecForName("x-uuencode");
    return codec->decode(in);
}

void KCodecs::uudecode(QByteArrayView in, QByteArray &out)
{
    out = uudecode(in);
}

//@cond PRIVATE

namespace KCodecs
{
// parse the encoded-word (scursor points to after the initial '=')
bool parseEncodedWord(const char *&scursor,
                      const char *const send,
                      QString *result,
                      QByteArray *language,
                      QByteArray *usedCS,
                      const QByteArray &defaultCS,
                      CharsetOption charsetOption)
{
    assert(result);
    assert(language);

    // make sure the caller already did a bit of the work.
    assert(*(scursor - 1) == '=');

    //
    // STEP 1:
    // scan for the charset/language portion of the encoded-word
    //

    char ch = *scursor++;

    if (ch != '?') {
        // qCDebug(KCODECS_LOG) << "first";
        // qCDebug(KCODECS_LOG) << "Premature end of encoded word";
        return false;
    }

    // remember start of charset (i.e. just after the initial "=?") and
    // language (just after the first '*') fields:
    const char *charsetStart = scursor;
    const char *languageStart = nullptr;

    // find delimiting '?' (and the '*' separating charset and language
    // tags, if any):
    for (; scursor != send; scursor++) {
        if (*scursor == '?') {
            break;
        } else if (*scursor == '*' && languageStart == nullptr) {
            languageStart = scursor + 1;
        }
    }

    // not found? can't be an encoded-word!
    if (scursor == send || *scursor != '?') {
        // qCDebug(KCODECS_LOG) << "second";
        // qCDebug(KCODECS_LOG) << "Premature end of encoded word";
        return false;
    }

    // extract the language information, if any (if languageStart is 0,
    // language will be null, too):
    QByteArray maybeLanguage(languageStart, scursor - languageStart);
    // extract charset information (keep in mind: the size given to the
    // ctor is one off due to the \0 terminator):
    QByteArray maybeCharset(charsetStart, (languageStart ? languageStart - 1 : scursor) - charsetStart);

    //
    // STEP 2:
    // scan for the encoding portion of the encoded-word
    //

    // remember start of encoding (just _after_ the second '?'):
    scursor++;
    const char *encodingStart = scursor;

    // find next '?' (ending the encoding tag):
    for (; scursor != send; scursor++) {
        if (*scursor == '?') {
            break;
        }
    }

    // not found? Can't be an encoded-word!
    if (scursor == send || *scursor != '?') {
        // qCDebug(KCODECS_LOG) << "third";
        // qCDebug(KCODECS_LOG) << "Premature end of encoded word";
        return false;
    }

    // extract the encoding information:
    QByteArray maybeEncoding(encodingStart, scursor - encodingStart);

    // qCDebug(KCODECS_LOG) << "parseEncodedWord: found charset == \"" << maybeCharset
    //         << "\"; language == \"" << maybeLanguage
    //         << "\"; encoding == \"" << maybeEncoding << "\"";

    //
    // STEP 3:
    // scan for encoded-text portion of encoded-word
    //

    // remember start of encoded-text (just after the third '?'):
    scursor++;
    const char *encodedTextStart = scursor;

    // find the '?=' sequence (ending the encoded-text):
    for (; scursor != send; scursor++) {
        if (*scursor == '?') {
            if (scursor + 1 != send) {
                if (*(scursor + 1) != '=') { // We expect a '=' after the '?', but we got something else; ignore
                    // qCDebug(KCODECS_LOG) << "Stray '?' in q-encoded word, ignoring this.";
                    continue;
                } else { // yep, found a '?=' sequence
                    scursor += 2;
                    break;
                }
            } else { // The '?' is the last char, but we need a '=' after it!
                // qCDebug(KCODECS_LOG) << "Premature end of encoded word";
                return false;
            }
        }
    }

    if (*(scursor - 2) != '?' || *(scursor - 1) != '=' || scursor < encodedTextStart + 2) {
        // qCDebug(KCODECS_LOG) << "Premature end of encoded word";
        return false;
    }

    // set end sentinel for encoded-text:
    const char *const encodedTextEnd = scursor - 2;

    //
    // STEP 4:
    // setup decoders for the transfer encoding and the charset
    //

    // try if there's a codec for the encoding found:
    Codec *codec = Codec::codecForName(maybeEncoding);
    if (!codec) {
        // qCDebug(KCODECS_LOG) << "Unknown encoding" << maybeEncoding;
        return false;
    }

    // get an instance of a corresponding decoder:
    Decoder *dec = codec->makeDecoder();
    assert(dec);

    // try if there's a (text)codec for the charset found:
    QByteArray cs;
    QStringDecoder textCodec;
    if (charsetOption == KCodecs::ForceDefaultCharset || maybeCharset.isEmpty()) {
        textCodec = QStringDecoder(defaultCS.constData());
        cs = cachedCharset(defaultCS);
    } else {
        textCodec = QStringDecoder(maybeCharset.constData());
        if (!textCodec.isValid()) { // no suitable codec found => use default charset
            textCodec = QStringDecoder(defaultCS.constData());
            cs = cachedCharset(defaultCS);
        } else {
            cs = cachedCharset(maybeCharset);
        }
    }
    if (usedCS) {
        *usedCS = updateEncodingCharset(*usedCS, cs);
    }

    if (!textCodec.isValid()) {
        // qCDebug(KCODECS_LOG) << "Unknown charset" << maybeCharset;
        delete dec;
        return false;
    };

    // qCDebug(KCODECS_LOG) << "mimeName(): \"" << textCodec->name() << "\"";

    // allocate a temporary buffer to store the 8bit text:
    int encodedTextLength = encodedTextEnd - encodedTextStart;
    QByteArray buffer;
    buffer.resize(codec->maxDecodedSizeFor(encodedTextLength));
    char *bbegin = buffer.data();
    char *bend = bbegin + buffer.length();

    //
    // STEP 5:
    // do the actual decoding
    //

    if (!dec->decode(encodedTextStart, encodedTextEnd, bbegin, bend)) {
        qWarning() << codec->name() << "codec lies about its maxDecodedSizeFor(" << encodedTextLength << ")\nresult may be truncated";
    }

    *result = textCodec.decode(QByteArrayView(buffer.data(), bbegin - buffer.data()));

    // qCDebug(KCODECS_LOG) << "result now: \"" << result << "\"";
    // cleanup:
    delete dec;
    *language = maybeLanguage;

    return true;
}

} // namespace KCodecs

//@endcond

QString KCodecs::decodeRFC2047String(QStringView msg)
{
    QByteArray usedCS;
    return decodeRFC2047String(msg.toUtf8(), &usedCS, CodecNames::utf8(), NoOption);
}

QString KCodecs::decodeRFC2047String(QByteArrayView src, QByteArray *usedCS, const QByteArray &defaultCS, CharsetOption charsetOption)
{
    QByteArray result;
    QByteArray spaceBuffer;
    const char *scursor = src.constData();
    const char *send = scursor + src.length();
    bool onlySpacesSinceLastWord = false;
    if (usedCS) {
        usedCS->clear();
    }

    while (scursor != send) {
        // space
        if (isspace(*scursor) && onlySpacesSinceLastWord) {
            spaceBuffer += *scursor++;
            continue;
        }

        // possible start of an encoded word
        if (*scursor == '=') {
            QByteArray language;
            QString decoded;
            ++scursor;
            const char *start = scursor;
            if (parseEncodedWord(scursor, send, &decoded, &language, usedCS, defaultCS, charsetOption)) {
                result += decoded.toUtf8();
                onlySpacesSinceLastWord = true;
                spaceBuffer.clear();
            } else {
                if (onlySpacesSinceLastWord) {
                    result += spaceBuffer;
                    onlySpacesSinceLastWord = false;
                }
                result += '=';
                scursor = start; // reset cursor after parsing failure
            }
            continue;
        } else {
            // unencoded data
            if (onlySpacesSinceLastWord) {
                result += spaceBuffer;
                onlySpacesSinceLastWord = false;
            }
            result += *scursor;
            ++scursor;
        }
    }
    // If there are any chars that couldn't be decoded in UTF-8,
    // fallback to local codec
    const QString tryUtf8 = QString::fromUtf8(result);
    if (tryUtf8.contains(QChar(0xFFFD))) {
        QStringDecoder codec(QStringDecoder::System);
        if (usedCS) {
            *usedCS = updateEncodingCharset(*usedCS, cachedCharset(codec.name()));
        }
        return codec.decode(result);
    } else {
        return tryUtf8;
    }
}

QByteArray KCodecs::encodeRFC2047String(QStringView src, const QByteArray &charset)
{
    QByteArray result;
    int start = 0;
    int end = 0;
    bool nonAscii = false;
    bool useQEncoding = false;

    QStringEncoder codec(charset.constData());

    QByteArray usedCS;
    if (!codec.isValid()) {
        // no codec available => try local8Bit and hope the best ;-)
        codec = QStringEncoder(QStringEncoder::System);
        usedCS = codec.name();
    } else {
        Q_ASSERT(codec.isValid());
        if (charset.isEmpty()) {
            usedCS = codec.name();
        } else {
            usedCS = charset;
        }
    }

    QByteArray encoded8Bit = codec.encode(src);
    if (codec.hasError()) {
        usedCS = CodecNames::utf8();
        codec = QStringEncoder(QStringEncoder::Utf8);
        encoded8Bit = codec.encode(src);
    }

    if (usedCS.contains("8859-")) { // use "B"-Encoding for non iso-8859-x charsets
        useQEncoding = true;
    }

    uint encoded8BitLength = encoded8Bit.length();
    for (unsigned int i = 0; i < encoded8BitLength; i++) {
        if (encoded8Bit[i] == ' ') { // encoding starts at word boundaries
            start = i + 1;
        }

        // encode escape character, for japanese encodings...
        if (((signed char)encoded8Bit[i] < 0) || (encoded8Bit[i] == '\033')) {
            end = start; // non us-ascii char found, now we determine where to stop encoding
            nonAscii = true;
            break;
        }
    }

    if (nonAscii) {
        while ((end < encoded8Bit.length()) && (encoded8Bit[end] != ' ')) {
            // we encode complete words
            end++;
        }

        for (int x = end; x < encoded8Bit.length(); x++) {
            if (((signed char)encoded8Bit[x] < 0) || (encoded8Bit[x] == '\033')) {
                end = x; // we found another non-ascii word

                while ((end < encoded8Bit.length()) && (encoded8Bit[end] != ' ')) {
                    // we encode complete words
                    end++;
                }
            }
        }

        result = encoded8Bit.left(start) + "=?" + usedCS;

        if (useQEncoding) {
            result += "?Q?";

            char hexcode; // "Q"-encoding implementation described in RFC 2047
            for (int i = start; i < end; i++) {
                const char c = encoded8Bit[i];
                if (c == ' ') { // make the result readable with not MIME-capable readers
                    result += '_';
                } else {
                    if (((c >= 'a') && (c <= 'z')) || // paranoid mode, encode *all* special chars to avoid problems
                        ((c >= 'A') && (c <= 'Z')) || // with "From" & "To" headers
                        ((c >= '0') && (c <= '9'))) {
                        result += c;
                    } else {
                        result += '='; // "stolen" from KMail ;-)
                        hexcode = ((c & 0xF0) >> 4) + 48;
                        if (hexcode >= 58) {
                            hexcode += 7;
                        }
                        result += hexcode;
                        hexcode = (c & 0x0F) + 48;
                        if (hexcode >= 58) {
                            hexcode += 7;
                        }
                        result += hexcode;
                    }
                }
            }
        } else {
            result += "?B?" + encoded8Bit.mid(start, end - start).toBase64();
        }

        result += "?=";
        result += encoded8Bit.right(encoded8Bit.length() - end);
    } else {
        result = encoded8Bit;
    }

    return result;
}

/******************************************************************************/
/*                           KCodecs::Codec                                   */

KCodecs::Codec *KCodecs::Codec::codecForName(QByteArrayView name)
{
    struct CodecEntry {
        const char *name;
        std::unique_ptr<KCodecs::Codec> codec;
    };
    // ### has to be sorted by name!
    static const std::array<CodecEntry, 6> s_codecs{{
        {"b", std::make_unique<KCodecs::Rfc2047BEncodingCodec>()},
        {"base64", std::make_unique<KCodecs::Base64Codec>()},
        {"q", std::make_unique<KCodecs::Rfc2047QEncodingCodec>()},
        {"quoted-printable", std::make_unique<KCodecs::QuotedPrintableCodec>()},
        {"x-kmime-rfc2231", std::make_unique<KCodecs::Rfc2231EncodingCodec>()},
        {"x-uuencode", std::make_unique<KCodecs::UUCodec>()},
    }};

    const auto it = std::lower_bound(s_codecs.begin(), s_codecs.end(), name, [](const auto &lhs, auto rhs) {
        return rhs.compare(lhs.name, Qt::CaseInsensitive) > 0;
    });
    if (it == s_codecs.end() || name.compare((*it).name, Qt::CaseInsensitive) != 0) {
        qWarning() << "Unknown codec \"" << name << "\" requested!";
    }
    return (*it).codec.get();
}

bool KCodecs::Codec::encode(const char *&scursor, const char *const send, char *&dcursor, const char *const dend, NewlineType newline) const
{
    // get an encoder:
    std::unique_ptr<Encoder> enc(makeEncoder(newline));
    if (!enc) {
        qWarning() << "makeEncoder failed for" << name();
        return false;
    }

    // encode and check for output buffer overflow:
    while (!enc->encode(scursor, send, dcursor, dend)) {
        if (dcursor == dend) {
            return false; // not enough space in output buffer
        }
    }

    // finish and check for output buffer overflow:
    while (!enc->finish(dcursor, dend)) {
        if (dcursor == dend) {
            return false; // not enough space in output buffer
        }
    }

    return true; // successfully encoded.
}

QByteArray KCodecs::Codec::encode(QByteArrayView src, NewlineType newline) const
{
    // allocate buffer for the worst case:
    QByteArray result;
    result.resize(maxEncodedSizeFor(src.size(), newline));

    // set up iterators:
    QByteArray::ConstIterator iit = src.begin();
    QByteArray::ConstIterator iend = src.end();
    QByteArray::Iterator oit = result.begin();
    QByteArray::ConstIterator oend = result.end();

    // encode
    if (!encode(iit, iend, oit, oend, newline)) {
        qCritical() << name() << "codec lies about it's mEncodedSizeFor()";
    }

    // shrink result to actual size:
    result.truncate(oit - result.begin());

    return result;
}

QByteArray KCodecs::Codec::decode(QByteArrayView src, NewlineType newline) const
{
    // allocate buffer for the worst case:
    QByteArray result;
    result.resize(maxDecodedSizeFor(src.size(), newline));

    // set up iterators:
    QByteArray::ConstIterator iit = src.begin();
    QByteArray::ConstIterator iend = src.end();
    QByteArray::Iterator oit = result.begin();
    QByteArray::ConstIterator oend = result.end();

    // decode
    if (!decode(iit, iend, oit, oend, newline)) {
        qCritical() << name() << "codec lies about it's maxDecodedSizeFor()";
    }

    // shrink result to actual size:
    result.truncate(oit - result.begin());

    return result;
}

bool KCodecs::Codec::decode(const char *&scursor, const char *const send, char *&dcursor, const char *const dend, NewlineType newline) const
{
    // get a decoder:
    std::unique_ptr<Decoder> dec(makeDecoder(newline));
    assert(dec);

    // decode and check for output buffer overflow:
    while (!dec->decode(scursor, send, dcursor, dend)) {
        if (dcursor == dend) {
            return false; // not enough space in output buffer
        }
    }

    // finish and check for output buffer overflow:
    while (!dec->finish(dcursor, dend)) {
        if (dcursor == dend) {
            return false; // not enough space in output buffer
        }
    }

    return true; // successfully encoded.
}

/******************************************************************************/
/*                          KCodecs::Encoder                                  */

KCodecs::EncoderPrivate::EncoderPrivate(Codec::NewlineType newline)
    : outputBufferCursor(0)
    , newline(newline)
{
}

KCodecs::Encoder::Encoder(Codec::NewlineType newline)
    : d(new KCodecs::EncoderPrivate(newline))
{
}

KCodecs::Encoder::~Encoder() = default;

bool KCodecs::Encoder::write(char ch, char *&dcursor, const char *const dend)
{
    if (dcursor != dend) {
        // if there's space in the output stream, write there:
        *dcursor++ = ch;
        return true;
    } else {
        // else buffer the output:
        if (d->outputBufferCursor >= maxBufferedChars) {
            qCritical() << "KCodecs::Encoder: internal buffer overflow!";
        } else {
            d->outputBuffer[d->outputBufferCursor++] = ch;
        }
        return false;
    }
}

// write as much as possible off the output buffer. Return true if
// flushing was complete, false if some chars could not be flushed.
bool KCodecs::Encoder::flushOutputBuffer(char *&dcursor, const char *const dend)
{
    int i;
    // copy output buffer to output stream:
    for (i = 0; dcursor != dend && i < d->outputBufferCursor; ++i) {
        *dcursor++ = d->outputBuffer[i];
    }

    // calculate the number of missing chars:
    int numCharsLeft = d->outputBufferCursor - i;
    // push the remaining chars to the beginning of the buffer:
    if (numCharsLeft) {
        ::memmove(d->outputBuffer, d->outputBuffer + i, numCharsLeft);
    }
    // adjust cursor:
    d->outputBufferCursor = numCharsLeft;

    return !numCharsLeft;
}

bool KCodecs::Encoder::writeCRLF(char *&dcursor, const char *const dend)
{
    if (d->newline == Codec::NewlineCRLF) {
        write('\r', dcursor, dend);
    }
    return write('\n', dcursor, dend);
}

/******************************************************************************/
/*                           KCodecs::Decoder                                 */

KCodecs::DecoderPrivate::DecoderPrivate(Codec::NewlineType newline)
    : newline(newline)
{
}

KCodecs::Decoder::Decoder(Codec::NewlineType newline)
    : d(new KCodecs::DecoderPrivate(newline))
{
}

KCodecs::Decoder::~Decoder() = default;
