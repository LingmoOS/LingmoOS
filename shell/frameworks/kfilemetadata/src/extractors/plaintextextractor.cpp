/*
    SPDX-FileCopyrightText: 2012 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#include "plaintextextractor.h"

#include <QDebug>
#include <QFile>
#include <QStringDecoder>
#include <QtMinMax>

#include <KEncodingProber>

#if defined(Q_OS_LINUX) || defined(__GLIBC__)
    #include <fcntl.h>
#endif

using namespace KFileMetaData;

PlainTextExtractor::PlainTextExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{

}

const QStringList supportedMimeTypes = {
    QStringLiteral("text/plain"),
};

QStringList PlainTextExtractor::mimetypes() const
{
    return supportedMimeTypes;
}

void PlainTextExtractor::extract(ExtractionResult* result)
{
    QFile file(result->inputUrl());
    bool isOpen = false;

#ifdef O_NOATIME
    const QByteArray filePath = QFile::encodeName(result->inputUrl());
    int fd = open(filePath.constData(), O_RDONLY | O_NOATIME);
    if (fd >= 0) {
        isOpen = file.open(fd, QIODevice::ReadOnly | QIODevice::Text, QFileDevice::AutoCloseHandle);
    } else
#endif
    {
        isOpen = file.open(QIODevice::ReadOnly | QIODevice::Text);
    }

    if (!isOpen) {
        return;
    }

    result->addType(Type::Text);
    if (!(result->inputFlags() & ExtractionResult::ExtractPlainText)) {
        return;
    }

    auto autodetectCodec = [](QFile &file) -> QStringDecoder {
        const qint64 BUFFER_SIZE = 256 * 1024;
        const auto buffer = file.read(BUFFER_SIZE);
        file.seek(0);

        // First 16 bytes for detecting by BOM.
        const QByteArrayView bufferForBom(buffer.begin(), qMin(16, buffer.size()));

        // first: try to get encoding by BOM handling
        // If BOM has been found, trust it
        if (auto encoding = QStringConverter::encodingForData(bufferForBom)) {
            return QStringDecoder(encoding.value());
        }

        // second: try to get encoding by KEncodingProber
        KEncodingProber prober(KEncodingProber::Universal);
        prober.feed(buffer.constData());

        // we found codec with some confidence?
        if (prober.confidence() > 0.5) {
            auto proberDecoder = QStringDecoder(prober.encoding().constData());
            // rare case, but if not valid, do not return proberDecoder
            if (proberDecoder.isValid()) {
                return proberDecoder;
            }
        }

        return QStringDecoder{QStringConverter::System};
    };

    QStringDecoder codec = {autodetectCodec(file)};

    int lines = 0;

    while (!file.atEnd()) {
        QString text = codec.decode(file.readLine());

        if (codec.hasError()) {
            qDebug() << "Invalid encoding. Ignoring" << result->inputUrl();
            return;
        }

        // Newline '\n' can be first symbol in line in case UTF-16LE.
        if (!text.isEmpty() && text.front() == QLatin1Char('\n')) {
            text.removeFirst();
        } else if (!text.isEmpty() && text.back() == QLatin1Char('\n')) {
            text.removeLast();
        }

        // This case is possible for Little-Endian encodings
        // when '\00' part of the newline character
        // is mistakenly read here as a separate line.
        if (file.atEnd() && text.isEmpty()) {
            break;
        }

        result->append(text);

        lines += 1;
    }
    if (result->inputFlags() & ExtractionResult::ExtractMetaData) {
        result->add(Property::LineCount, lines);
    }
}

#include "moc_plaintextextractor.cpp"
