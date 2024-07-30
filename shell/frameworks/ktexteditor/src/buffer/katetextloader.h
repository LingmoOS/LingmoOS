/*
    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_TEXTLOADER_H
#define KATE_TEXTLOADER_H

#include <QCryptographicHash>
#include <QFile>
#include <QMimeDatabase>
#include <QString>
#include <QStringDecoder>

#include <KCompressionDevice>
#include <KEncodingProber>

#include "katetextbuffer.h"

namespace Kate
{
/**
 * loader block size, load 256 kb at once per default
 * if file size is smaller, fall back to file size
 * must be a multiple of 2
 */
static const qint64 KATE_FILE_LOADER_BS = 256 * 1024;

/**
 * File Loader, will handle reading of files + detecting encoding
 */
class TextLoader
{
public:
    /**
     * Construct file loader for given file.
     * @param filename file to open
     * @param proberType prober type
     * @param lineLengthLimit limit for lines to load, else we break them up in smaller ones
     */
    TextLoader(const QString &filename, KEncodingProber::ProberType proberType, int lineLengthLimit)
        : m_eof(false) // default to not eof
        , m_lastWasEndOfLine(true) // at start of file, we had a virtual newline
        , m_lastWasR(false) // we have not found a \r as last char
        , m_position(0)
        , m_lastLineStart(0)
        , m_eol(TextBuffer::eolUnknown) // no eol type detected atm
        , m_buffer(KATE_FILE_LOADER_BS, 0)
        , m_digest(QCryptographicHash::Sha1)
        , m_bomFound(false)
        , m_firstRead(true)
        , m_proberType(proberType)
        , m_fileSize(0)
        , m_lineLengthLimit(lineLengthLimit)
    {
        // try to get mimetype for on the fly decompression, don't rely on filename!
        QFile testMime(filename);
        if (testMime.open(QIODevice::ReadOnly)) {
            m_fileSize = testMime.size();
        }
        m_mimeType = QMimeDatabase().mimeTypeForFileNameAndData(filename, &testMime).name();

        // construct filter device
        KCompressionDevice::CompressionType compressionType = KCompressionDevice::compressionTypeForMimeType(m_mimeType);
        m_file = new KCompressionDevice(filename, compressionType);
    }

    /**
     * Destructor
     */
    ~TextLoader()
    {
        delete m_file;
    }

    /**
     * open file with given codec
     * @param codec codec to use, if 0, will do some auto-detect or fallback
     * @return success
     */
    bool open(const QString &codec)
    {
        m_codec = codec;
        m_eof = false;
        m_lastWasEndOfLine = true;
        m_lastWasR = false;
        m_position = 0;
        m_lastLineStart = 0;
        m_alreadyScanned = -1;
        m_eol = TextBuffer::eolUnknown;
        m_text.clear();
        m_converterState = m_codec.isEmpty() ? QStringDecoder() : QStringDecoder(m_codec.toUtf8().constData());
        m_bomFound = false;
        m_firstRead = true;

        // init the hash with the git header
        const QString header = QStringLiteral("blob %1").arg(m_fileSize);
        m_digest.reset();
        m_digest.addData(QByteArray(header.toLatin1() + '\0'));

        // if already opened, close the file...
        if (m_file->isOpen()) {
            m_file->close();
        }

        return m_file->open(QIODevice::ReadOnly);
    }

    /**
     * end of file reached?
     * @return end of file reached
     */
    bool eof() const
    {
        return m_eof && !m_lastWasEndOfLine && (m_lastLineStart == m_text.length());
    }

    /**
     * Detected end of line mode for this file.
     * Detected during reading, is valid after complete file is read.
     * @return eol mode of this file
     */
    TextBuffer::EndOfLineMode eol() const
    {
        return m_eol;
    }

    /**
     * BOM found?
     * @return byte order mark found?
     */
    bool byteOrderMarkFound() const
    {
        return m_bomFound;
    }

    /**
     * mime type used to create filter dev
     * @return mime-type of filter device
     */
    const QString &mimeTypeForFilterDev() const
    {
        return m_mimeType;
    }

    /**
     * internal Unicode data array
     * @return internal Unicode data
     */
    const QChar *unicode() const
    {
        return m_text.unicode();
    }

    /**
     * Get codec for this loader
     * @return currently in use codec of this loader
     */
    QString textCodec() const
    {
        return m_codec;
    }

    /**
     * read a line, return length + offset in Unicode data
     * @param offset offset into internal Unicode data for read line
     * @param length length of read line
     * @param tooLongLinesWrapped was a too long line seen?
     * @param longestLineLoaded length of the longest line that hit the limit
     * @return true if no encoding errors occurred
     */
    bool readLine(int &offset, int &length, bool &tooLongLinesWrapped, int &longestLineLoaded)
    {
        length = 0;
        offset = 0;
        bool encodingError = false;

        static const QLatin1Char cr(QLatin1Char('\r'));
        static const QLatin1Char lf(QLatin1Char('\n'));

        /**
         * did we read two time but got no stuff? encoding error
         * fixes problem with one character latin-1 files, which lead to crash otherwise!
         * bug 272579
         */
        bool failedToConvertOnce = false;

        /**
         * keep track if we have found BOM so that failedToConvertOnce is not erroneously set to true
         * BUG: 440359
         */
        bool bomPreviouslyFound = m_bomFound;

        // honor the line length limit early
        const auto lineLimitHandler = [this, &offset, &length, &tooLongLinesWrapped, &longestLineLoaded](int lineStart, int textLength) {
            if ((m_lineLengthLimit <= 0) || (textLength <= m_lineLengthLimit)) {
                return false;
            }

            // remember stick error
            tooLongLinesWrapped = true;
            longestLineLoaded = std::max(longestLineLoaded, textLength);

            // search for place to wrap
            int spacePosition = m_lineLengthLimit - 1;
            for (int testPosition = m_lineLengthLimit - 1; (testPosition >= 0) && (testPosition >= (m_lineLengthLimit - (m_lineLengthLimit / 10)));
                 --testPosition) {
                // wrap place found?
                if (m_text[lineStart + testPosition].isSpace() || m_text[lineStart + testPosition].isPunct()) {
                    spacePosition = testPosition;
                    break;
                }
            }

            m_lastWasEndOfLine = false;
            m_lastWasR = false;

            // line data
            offset = lineStart;
            length = spacePosition + 1;

            m_lastLineStart = m_position = (lineStart + length);
            return true;
        };

        /**
         * reading loop
         */
        while (m_position <= m_text.length()) {
            // handle too long lines early even if we not yet have seen the end
            if (m_alreadyScanned > m_lastLineStart && lineLimitHandler(m_lastLineStart, m_alreadyScanned - m_lastLineStart)) {
                return !encodingError;
            }

            if (m_position == m_text.length()) {
                // try to load more text if something is around
                if (!m_eof) {
                    // kill the old lines...
                    m_text.remove(0, m_lastLineStart);

                    // try to read new data
                    const int c = m_file->read(m_buffer.data(), m_buffer.size());

                    // if any text is there, append it....
                    if (c > 0) {
                        // update hash sum
                        m_digest.addData(QByteArrayView(m_buffer.data(), c));

                        // detect byte order marks & codec for byte order marks on first read
                        if (m_firstRead) {
                            /**
                             * if no codec given, do autodetection
                             */
                            if (!m_converterState.isValid()) {
                                /**
                                 * first: try to get HTML header encoding, includes BOM handling
                                 */
                                m_converterState = QStringDecoder::decoderForHtml(m_buffer);

                                /**
                                 * else: use KEncodingProber
                                 */
                                if (!m_converterState.isValid()) {
                                    KEncodingProber prober(m_proberType);
                                    prober.feed(m_buffer.constData(), c);

                                    // we found codec with some confidence?
                                    if (prober.confidence() > 0.5) {
                                        m_converterState = QStringDecoder(prober.encoding().constData());
                                    }
                                }

                                // no codec, no chance, encoding error, else remember the codec name
                                if (!m_converterState.isValid()) {
                                    return false;
                                }
                            }

                            // we want to convert the bom for later detection
                            m_converterState = QStringDecoder(m_converterState.name(), QStringConverter::Flag::ConvertInitialBom);

                            // remember name, might have changed
                            m_codec = QString::fromUtf8(m_converterState.name());
                        }

                        // detect broken encoding
                        Q_ASSERT(m_converterState.isValid());
                        const QString unicode = m_converterState.decode(QByteArrayView(m_buffer.data(), c));
                        encodingError = encodingError || m_converterState.hasError();

                        // check and remove bom
                        if (m_firstRead && !unicode.isEmpty() && (unicode.front() == QChar::ByteOrderMark || unicode.front() == QChar::ByteOrderSwapped)) {
                            m_bomFound = true;
                            m_text.append(QStringView(unicode).last(unicode.size() - 1));

                            // swapped BOM is encoding error
                            encodingError = encodingError || unicode.front() == QChar::ByteOrderSwapped;
                        } else {
                            m_text.append(unicode);
                        }
                        m_firstRead = false;
                    }

                    // is file completely read ?
                    m_eof = (c == -1) || (c == 0);

                    // recalc current pos and last pos
                    m_position -= m_lastLineStart;
                    m_alreadyScanned = m_position - 1;
                    m_lastLineStart = 0;
                }

                // oh oh, end of file, escape !
                if (m_eof && (m_position == m_text.length())) {
                    m_lastWasEndOfLine = false;

                    // line data
                    offset = m_lastLineStart;
                    length = m_position - m_lastLineStart;

                    m_lastLineStart = m_position;

                    lineLimitHandler(offset, length);
                    return !encodingError && !failedToConvertOnce;
                }

                // empty? try again
                if (m_position == m_text.length()) {
                    if (!bomPreviouslyFound && m_bomFound) {
                        // BOM was processed above, so we didn't fail to convert
                        bomPreviouslyFound = true;
                    } else {
                        failedToConvertOnce = true;
                    }
                    continue;
                }
            }

            for (; m_position < m_text.length(); m_position++) {
                m_alreadyScanned = m_position;
                QChar current_char = m_text.at(m_position);
                if (current_char == lf) {
                    m_lastWasEndOfLine = true;

                    if (m_lastWasR) {
                        m_lastLineStart++;
                        m_lastWasR = false;
                        m_eol = TextBuffer::eolDos;
                    } else {
                        // line data
                        offset = m_lastLineStart;
                        length = m_position - m_lastLineStart;

                        m_lastLineStart = m_position + 1;
                        m_position++;

                        // only win, if not dos!
                        if (m_eol != TextBuffer::eolDos) {
                            m_eol = TextBuffer::eolUnix;
                        }

                        lineLimitHandler(offset, length);
                        return !encodingError;
                    }
                } else if (current_char == cr) {
                    m_lastWasEndOfLine = true;
                    m_lastWasR = true;

                    // line data
                    offset = m_lastLineStart;
                    length = m_position - m_lastLineStart;

                    m_lastLineStart = m_position + 1;
                    m_position++;

                    // should only win of first time!
                    if (m_eol == TextBuffer::eolUnknown) {
                        m_eol = TextBuffer::eolMac;
                    }

                    lineLimitHandler(offset, length);
                    return !encodingError;
                } else if (current_char == QChar::LineSeparator) {
                    m_lastWasEndOfLine = true;

                    // line data
                    offset = m_lastLineStart;
                    length = m_position - m_lastLineStart;

                    m_lastLineStart = m_position + 1;
                    m_position++;

                    lineLimitHandler(offset, length);
                    return !encodingError;
                } else {
                    m_lastWasEndOfLine = false;
                    m_lastWasR = false;
                }
            }
        }

        return !encodingError;
    }

    QByteArray digest()
    {
        return m_digest.result();
    }

private:
    QString m_codec;
    bool m_eof;
    bool m_lastWasEndOfLine;
    bool m_lastWasR;
    int m_position;
    int m_lastLineStart;
    int m_alreadyScanned = -1;
    TextBuffer::EndOfLineMode m_eol;
    QString m_mimeType;
    QIODevice *m_file;
    QByteArray m_buffer;
    QCryptographicHash m_digest;
    QString m_text;
    QStringDecoder m_converterState;
    bool m_bomFound;
    bool m_firstRead;
    KEncodingProber::ProberType m_proberType;
    quint64 m_fileSize;
    const int m_lineLengthLimit;
};

}

#endif
