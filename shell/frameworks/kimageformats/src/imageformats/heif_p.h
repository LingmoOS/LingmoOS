/*
    High Efficiency Image File Format (HEIF) support for QImage.

    SPDX-FileCopyrightText: 2020 Sirius Bakke <sirius@bakke.co>
    SPDX-FileCopyrightText: 2021 Daniel Novomesky <dnovomesky@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_HEIF_P_H
#define KIMG_HEIF_P_H

#include <QByteArray>
#include <QImage>
#include <QImageIOPlugin>
#include <QMutex>

class HEIFHandler : public QImageIOHandler
{
public:
    HEIFHandler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;

    QVariant option(ImageOption option) const override;
    void setOption(ImageOption option, const QVariant &value) override;
    bool supportsOption(ImageOption option) const override;

    static bool isHeifDecoderAvailable();
    static bool isHeifEncoderAvailable();
    static bool isHej2DecoderAvailable();

    static bool isSupportedBMFFType(const QByteArray &header);
    static bool isSupportedHEJ2(const QByteArray &header);

private:
    bool ensureParsed() const;
    bool ensureDecoder();

    enum ParseHeicState {
        ParseHeicError = -1,
        ParseHeicNotParsed = 0,
        ParseHeicSuccess = 1,
    };

    ParseHeicState m_parseState;
    int m_quality;
    QImage m_current_image;

    bool write_helper(const QImage &image);

    static void startHeifLib();
    static void finishHeifLib();
    static size_t m_initialized_count;

    static bool m_plugins_queried;
    static bool m_heif_decoder_available;
    static bool m_heif_encoder_available;
    static bool m_hej2_decoder_available;

    static QMutex &getHEIFHandlerMutex();
};

class HEIFPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "heif.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_HEIF_P_H
