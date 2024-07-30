/*
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ani_p.h"

#include <QDebug>
#include <QImage>
#include <QScopeGuard>
#include <QVariant>
#include <QtEndian>

#include <cstring>

namespace
{
struct ChunkHeader {
    char magic[4];
    quint32_le size;
};

struct AniHeader {
    quint32_le cbSize;
    quint32_le nFrames; // number of actual frames in the file
    quint32_le nSteps; // number of logical images
    quint32_le iWidth;
    quint32_le iHeight;
    quint32_le iBitCount;
    quint32_le nPlanes;
    quint32_le iDispRate;
    quint32_le bfAttributes; // attributes (0 = bitmap images, 1 = ico/cur, 3 = "seq" block available)
};

struct CurHeader {
    quint16_le wReserved; // always 0
    quint16_le wResID; // always 2
    quint16_le wNumImages;
};

struct CursorDirEntry {
    quint8 bWidth;
    quint8 bHeight;
    quint8 bColorCount;
    quint8 bReserved; // always 0
    quint16_le wHotspotX;
    quint16_le wHotspotY;
    quint32_le dwBytesInImage;
    quint32_le dwImageOffset;
};

} // namespace

ANIHandler::ANIHandler() = default;

bool ANIHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("ani");
        return true;
    }

    // Check if there's another frame coming
    const QByteArray nextFrame = device()->peek(sizeof(ChunkHeader));
    if (nextFrame.size() == sizeof(ChunkHeader)) {
        const auto *header = reinterpret_cast<const ChunkHeader *>(nextFrame.data());
        if (qstrncmp(header->magic, "icon", sizeof(header->magic)) == 0 && header->size > 0) {
            setFormat("ani");
            return true;
        }
    }

    return false;
}

bool ANIHandler::read(QImage *outImage)
{
    if (!ensureScanned()) {
        return false;
    }

    if (device()->pos() < m_firstFrameOffset) {
        device()->seek(m_firstFrameOffset);
    }

    const QByteArray frameType = device()->read(4);
    if (frameType != "icon") {
        return false;
    }

    const QByteArray frameSizeData = device()->read(sizeof(quint32_le));
    if (frameSizeData.size() != sizeof(quint32_le)) {
        return false;
    }

    const auto frameSize = *(reinterpret_cast<const quint32_le *>(frameSizeData.data()));
    if (!frameSize) {
        return false;
    }

    const QByteArray frameData = device()->read(frameSize);

    const bool ok = outImage->loadFromData(frameData, "cur");

    ++m_currentImageNumber;

    // When we have a custom image sequence, seek to before the frame that would follow
    if (!m_imageSequence.isEmpty()) {
        if (m_currentImageNumber < m_imageSequence.count()) {
            const int nextFrame = m_imageSequence.at(m_currentImageNumber);
            if (nextFrame < 0 || nextFrame >= m_frameOffsets.count()) {
                return false;
            }
            const auto nextOffset = m_frameOffsets.at(nextFrame);
            device()->seek(nextOffset);
        } else if (m_currentImageNumber == m_imageSequence.count()) {
            const auto endOffset = m_frameOffsets.last();
            if (device()->pos() != endOffset) {
                device()->seek(endOffset);
            }
        }
    }

    return ok;
}

int ANIHandler::currentImageNumber() const
{
    if (!ensureScanned()) {
        return 0;
    }
    return m_currentImageNumber;
}

int ANIHandler::imageCount() const
{
    if (!ensureScanned()) {
        return 0;
    }
    return m_imageCount;
}

bool ANIHandler::jumpToImage(int imageNumber)
{
    if (!ensureScanned()) {
        return false;
    }

    if (imageNumber < 0) {
        return false;
    }

    if (imageNumber == m_currentImageNumber) {
        return true;
    }

    // If we have a custom image sequence we have a index of frames we can jump to
    if (!m_imageSequence.isEmpty()) {
        if (imageNumber >= m_imageSequence.count()) {
            return false;
        }

        const int targetFrame = m_imageSequence.at(imageNumber);

        const auto targetOffset = m_frameOffsets.value(targetFrame, -1);

        if (device()->seek(targetOffset)) {
            m_currentImageNumber = imageNumber;
            return true;
        }

        return false;
    }

    if (imageNumber >= m_frameCount) {
        return false;
    }

    // otherwise we need to jump from frame to frame
    const auto oldPos = device()->pos();

    if (imageNumber < m_currentImageNumber) {
        // start from the beginning
        if (!device()->seek(m_firstFrameOffset)) {
            return false;
        }
    }

    while (m_currentImageNumber < imageNumber) {
        if (!jumpToNextImage()) {
            device()->seek(oldPos);
            return false;
        }
    }

    m_currentImageNumber = imageNumber;
    return true;
}

bool ANIHandler::jumpToNextImage()
{
    if (!ensureScanned()) {
        return false;
    }

    // If we have a custom image sequence we have a index of frames we can jump to
    // Delegate to jumpToImage
    if (!m_imageSequence.isEmpty()) {
        return jumpToImage(m_currentImageNumber + 1);
    }

    if (device()->pos() < m_firstFrameOffset) {
        if (!device()->seek(m_firstFrameOffset)) {
            return false;
        }
    }

    const QByteArray nextFrame = device()->peek(sizeof(ChunkHeader));
    if (nextFrame.size() != sizeof(ChunkHeader)) {
        return false;
    }

    const auto *header = reinterpret_cast<const ChunkHeader *>(nextFrame.data());
    if (qstrncmp(header->magic, "icon", sizeof(header->magic)) != 0) {
        return false;
    }

    const qint64 seekBy = sizeof(ChunkHeader) + header->size;

    if (!device()->seek(device()->pos() + seekBy)) {
        return false;
    }

    ++m_currentImageNumber;
    return true;
}

int ANIHandler::loopCount() const
{
    if (!ensureScanned()) {
        return 0;
    }
    return -1;
}

int ANIHandler::nextImageDelay() const
{
    if (!ensureScanned()) {
        return 0;
    }

    int rate = m_displayRate;

    if (!m_displayRates.isEmpty()) {
        int previousImage = m_currentImageNumber - 1;
        if (previousImage < 0) {
            previousImage = m_displayRates.count() - 1;
        }
        rate = m_displayRates.at(previousImage);
    }

    return rate * 1000 / 60;
}

bool ANIHandler::supportsOption(ImageOption option) const
{
    return option == Size || option == Name || option == Description || option == Animation;
}

QVariant ANIHandler::option(ImageOption option) const
{
    if (!supportsOption(option) || !ensureScanned()) {
        return QVariant();
    }

    switch (option) {
    case QImageIOHandler::Size:
        return m_size;
    // TODO QImageIOHandler::Format
    // but both iBitCount in AniHeader and bColorCount are just zero most of the time
    // so one would probably need to traverse even further down into IcoHeader and IconDirEntry...
    // but Qt's ICO/CUR handler always seems to give us a ARB
    case QImageIOHandler::Name:
        return m_name;
    case QImageIOHandler::Description: {
        QString description;
        if (!m_name.isEmpty()) {
            description += QStringLiteral("Title: %1\n\n").arg(m_name);
        }
        if (!m_artist.isEmpty()) {
            description += QStringLiteral("Author: %1\n\n").arg(m_artist);
        }
        return description;
    }

    case QImageIOHandler::Animation:
        return true;
    default:
        break;
    }

    return QVariant();
}

bool ANIHandler::ensureScanned() const
{
    if (m_scanned) {
        return true;
    }

    if (device()->isSequential()) {
        return false;
    }

    auto *mutableThis = const_cast<ANIHandler *>(this);

    const auto oldPos = device()->pos();
    auto cleanup = qScopeGuard([this, oldPos] {
        device()->seek(oldPos);
    });

    device()->seek(0);

    const QByteArray riffIntro = device()->read(4);
    if (riffIntro != "RIFF") {
        return false;
    }

    const auto riffSizeData = device()->read(sizeof(quint32_le));
    if (riffSizeData.size() != sizeof(quint32_le)) {
        return false;
    }
    const auto riffSize = *(reinterpret_cast<const quint32_le *>(riffSizeData.data()));
    // TODO do a basic sanity check if the size is enough to hold some metadata and a frame?
    if (riffSize == 0) {
        return false;
    }

    mutableThis->m_displayRates.clear();
    mutableThis->m_imageSequence.clear();

    while (device()->pos() < riffSize) {
        const QByteArray chunkId = device()->read(4);
        if (chunkId.length() != 4) {
            return false;
        }

        if (chunkId == "ACON") {
            continue;
        }

        const QByteArray chunkSizeData = device()->read(sizeof(quint32_le));
        if (chunkSizeData.length() != sizeof(quint32_le)) {
            return false;
        }
        auto chunkSize = *(reinterpret_cast<const quint32_le *>(chunkSizeData.data()));

        if (chunkId == "anih") {
            if (chunkSize != sizeof(AniHeader)) {
                qWarning() << "anih chunk size does not match ANIHEADER size";
                return false;
            }

            const QByteArray anihData = device()->read(sizeof(AniHeader));
            if (anihData.size() != sizeof(AniHeader)) {
                return false;
            }

            auto *aniHeader = reinterpret_cast<const AniHeader *>(anihData.data());

            // The size in the ani header is usually 0 unfortunately,
            // so we'll also check the first frame for its size further below
            mutableThis->m_size = QSize(aniHeader->iWidth, aniHeader->iHeight);
            mutableThis->m_frameCount = aniHeader->nFrames;
            mutableThis->m_imageCount = aniHeader->nSteps;
            mutableThis->m_displayRate = aniHeader->iDispRate;
        } else if (chunkId == "rate" || chunkId == "seq ") {
            const QByteArray data = device()->read(chunkSize);
            if (static_cast<quint32_le>(data.size()) != chunkSize || data.size() % sizeof(quint32_le) != 0) {
                return false;
            }

            // TODO should we check that the number of rate entries matches nSteps?
            auto *dataPtr = data.data();
            QList<int> list;
            for (int i = 0; i < data.size(); i += sizeof(quint32_le)) {
                const auto entry = *(reinterpret_cast<const quint32_le *>(dataPtr + i));
                list.append(entry);
            }

            if (chunkId == "rate") {
                // should we check that the number of rate entries matches nSteps?
                mutableThis->m_displayRates = list;
            } else if (chunkId == "seq ") {
                // Check if it's just an ascending sequence, don't bother with it then
                bool isAscending = true;
                for (int i = 0; i < list.count(); ++i) {
                    if (list.at(i) != i) {
                        isAscending = false;
                        break;
                    }
                }

                if (!isAscending) {
                    mutableThis->m_imageSequence = list;
                }
            }
            // IART and INAM are technically inside LIST->INFO but "INFO" is supposedly optional
            // so just handle those two attributes wherever we encounter them
        } else if (chunkId == "INAM" || chunkId == "IART") {
            const QByteArray value = device()->read(chunkSize);

            if (static_cast<quint32_le>(value.size()) != chunkSize) {
                return false;
            }

            // DWORDs are aligned to even sizes
            if (chunkSize % 2 != 0) {
                device()->read(1);
            }

            // FIXME encoding
            const QString stringValue = QString::fromLocal8Bit(value.constData(), std::strlen(value.constData()));
            if (chunkId == "INAM") {
                mutableThis->m_name = stringValue;
            } else if (chunkId == "IART") {
                mutableThis->m_artist = stringValue;
            }
        } else if (chunkId == "LIST") {
            const QByteArray listType = device()->read(4);

            if (listType == "INFO") {
                // Technically would contain INAM and IART but we handle them anywhere above
            } else if (listType == "fram") {
                quint64 read = 0;
                while (read < chunkSize) {
                    const QByteArray chunkType = device()->read(4);
                    read += 4;
                    if (chunkType != "icon") {
                        break;
                    }

                    if (!m_firstFrameOffset) {
                        mutableThis->m_firstFrameOffset = device()->pos() - 4;
                        mutableThis->m_currentImageNumber = 0;

                        // If size in header isn't valid, use the first frame's size instead
                        if (!m_size.isValid() || m_size.isEmpty()) {
                            const auto oldPos = device()->pos();

                            device()->read(sizeof(quint32_le));

                            const QByteArray curHeaderData = device()->read(sizeof(CurHeader));
                            const QByteArray cursorDirEntryData = device()->read(sizeof(CursorDirEntry));

                            if (curHeaderData.length() == sizeof(CurHeader) && cursorDirEntryData.length() == sizeof(CursorDirEntry)) {
                                auto *cursorDirEntry = reinterpret_cast<const CursorDirEntry *>(cursorDirEntryData.data());
                                mutableThis->m_size = QSize(cursorDirEntry->bWidth, cursorDirEntry->bHeight);
                            }

                            device()->seek(oldPos);
                        }

                        // If we don't have a custom image sequence we can stop scanning right here
                        if (m_imageSequence.isEmpty()) {
                            break;
                        }
                    }

                    mutableThis->m_frameOffsets.append(device()->pos() - 4);

                    const QByteArray frameSizeData = device()->read(sizeof(quint32_le));
                    if (frameSizeData.size() != sizeof(quint32_le)) {
                        return false;
                    }

                    const auto frameSize = *(reinterpret_cast<const quint32_le *>(frameSizeData.data()));
                    device()->seek(device()->pos() + frameSize);

                    read += frameSize;

                    if (m_frameOffsets.count() == m_frameCount) {
                        // Also record the end of frame data
                        mutableThis->m_frameOffsets.append(device()->pos() - 4);
                        break;
                    }
                }
                break;
            }
        }
    }

    if (m_imageCount != m_frameCount && m_imageSequence.isEmpty()) {
        qWarning("ANIHandler: 'nSteps' is not equal to 'nFrames' but no 'seq' entries were provided");
        return false;
    }

    if (!m_imageSequence.isEmpty() && m_imageSequence.count() != m_imageCount) {
        qWarning("ANIHandler: count of entries in 'seq' does not match 'nSteps' in anih");
        return false;
    }

    if (!m_displayRates.isEmpty() && m_displayRates.count() != m_imageCount) {
        qWarning("ANIHandler: count of entries in 'rate' does not match 'nSteps' in anih");
        return false;
    }

    if (!m_frameOffsets.isEmpty() && m_frameOffsets.count() - 1 != m_frameCount) {
        qWarning("ANIHandler: number of actual frames does not match 'nFrames' in anih");
        return false;
    }

    mutableThis->m_scanned = true;
    return true;
}

bool ANIHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("ANIHandler::canRead() called with no device");
        return false;
    }
    if (device->isSequential()) {
        return false;
    }

    const QByteArray riffIntro = device->peek(12);

    if (riffIntro.length() != 12) {
        return false;
    }

    if (!riffIntro.startsWith("RIFF")) {
        return false;
    }

    // TODO sanity check chunk size?

    if (riffIntro.mid(4 + 4, 4) != "ACON") {
        return false;
    }

    return true;
}

QImageIOPlugin::Capabilities ANIPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "ani") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && ANIHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *ANIPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new ANIHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_ani_p.cpp"
