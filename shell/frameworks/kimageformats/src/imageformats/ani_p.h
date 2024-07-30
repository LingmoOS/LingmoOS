/*
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_ANI_P_H
#define KIMG_ANI_P_H

#include <QImageIOPlugin>
#include <QSize>

class ANIHandler : public QImageIOHandler
{
public:
    ANIHandler();

    bool canRead() const override;
    bool read(QImage *image) override;

    int currentImageNumber() const override;
    int imageCount() const override;
    bool jumpToImage(int imageNumber) override;
    bool jumpToNextImage() override;

    int loopCount() const override;
    int nextImageDelay() const override;

    bool supportsOption(ImageOption option) const override;
    QVariant option(ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    bool ensureScanned() const;

    bool m_scanned = false;

    int m_currentImageNumber = 0;

    int m_frameCount = 0; // "physical" frames
    int m_imageCount = 0; // logical images
    // Stores a custom sequence of images
    QList<int> m_imageSequence;
    // and the corresponding offsets where they are
    // since we can't read the image data sequentally in this case then
    QList<qint64> m_frameOffsets;
    qint64 m_firstFrameOffset = 0;

    int m_displayRate = 0;
    QList<int> m_displayRates;

    QString m_name;
    QString m_artist;
    QSize m_size;
};

class ANIPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "ani.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_ANI_P_H
