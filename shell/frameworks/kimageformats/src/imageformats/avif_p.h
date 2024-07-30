/*
    AV1 Image File Format (AVIF) support for QImage.

    SPDX-FileCopyrightText: 2020 Daniel Novomesky <dnovomesky@gmail.com>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef KIMG_AVIF_P_H
#define KIMG_AVIF_P_H

#include <QByteArray>
#include <QImage>
#include <QImageIOPlugin>
#include <QPointF>
#include <QSize>
#include <QVariant>
#include <avif/avif.h>
#include <qimageiohandler.h>

class QAVIFHandler : public QImageIOHandler
{
public:
    QAVIFHandler();
    ~QAVIFHandler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;

    static bool canRead(QIODevice *device);

    QVariant option(ImageOption option) const override;
    void setOption(ImageOption option, const QVariant &value) override;
    bool supportsOption(ImageOption option) const override;

    int imageCount() const override;
    int currentImageNumber() const override;
    bool jumpToNextImage() override;
    bool jumpToImage(int imageNumber) override;

    int nextImageDelay() const override;

    int loopCount() const override;

private:
    static QPointF CompatibleChromacity(qreal chrX, qreal chrY);
    bool ensureParsed() const;
    bool ensureOpened() const;
    bool ensureDecoder();
    bool decode_one_frame();

    enum ParseAvifState {
        ParseAvifError = -1,
        ParseAvifNotParsed = 0,
        ParseAvifSuccess = 1,
        ParseAvifMetadata = 2,
        ParseAvifFinished = 3,
    };

    ParseAvifState m_parseState;
    int m_quality;

    uint32_t m_container_width;
    uint32_t m_container_height;
    QSize m_estimated_dimensions;

    QByteArray m_rawData;
    avifROData m_rawAvifData;

    avifDecoder *m_decoder;
    QImage m_current_image;

    bool m_must_jump_to_next_image;
};

class QAVIFPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "avif.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_AVIF_P_H
